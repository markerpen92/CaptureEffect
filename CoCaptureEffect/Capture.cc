#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/gnuplot.h"
//#include "DeCompile.h"
#include <dirent.h>
#include <iostream>
#include <regex>

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("NCS0912 Topo");

Gnuplot2dDataset Rate5Thr;
Gnuplot2dDataset Rate11Thr;
Gnuplot2dDataset Rate5LossPkt;
Gnuplot2dDataset Rate11LossPkt;

void ServerDection(Ptr<PacketSink> APsink , uint32_t *APLastPktNum)
{
    if(APsink->GetTotalRx() > *APLastPktNum)
    {
        Time now = Simulator::Now();
        cout << "AP revice " << APsink->GetTotalRx()-*APLastPktNum << " Pkts" << "in sec : " << now.GetSeconds() << "\n";
    }
    *APLastPktNum = APsink->GetTotalRx();
    Simulator::Schedule(MilliSeconds(1.0) , &ServerDection , APsink , APLastPktNum);
}

void TCPCalculateThroughput(Ptr<PacketSink> *sink , uint32_t nWifi , uint32_t *AllNodesPastPktNum)
{
    Time now = Simulator::Now();
    cout << "~~~~~~~~~~" << now.GetSeconds() << "~~~~~~~~~~" << "\n";

    for(uint32_t i=0 ; i<nWifi ; i++)
    {
        double nowThr = 0.0;
        nowThr = (sink[i]->GetTotalRx() - AllNodesPastPktNum[i]) * (double) 8 / 1e6;
        cout << "Node[" << i << "]\t" << nowThr << "   pkts  " << sink[i]->GetTotalRx()/1024 << "\n" ; 
        AllNodesPastPktNum[i] = sink[i]->GetTotalRx();
    }
    //return AllNodesPastThr;

    Simulator::Schedule(Seconds(1.0) , &TCPCalculateThroughput , &sink[0] , nWifi , &AllNodesPastPktNum[0]);
}

void UDPCaculateThroughput(Ptr<PacketSink> *APsink , uint32_t nWifi , uint32_t *AllNodesPastPktNum)
{
    Time now = Simulator::Now();
    cout << "~~~~~~~~~~" << now.GetSeconds() << "~~~~~~~~~~" << "\n";

    double EachNodeThr[nWifi] = {0.0};
    for(uint32_t i=0 ; i<nWifi ; i++)
    {
        double nowThr = 0.0;
        nowThr = (APsink[i]->GetTotalRx() - AllNodesPastPktNum[i]) * (double) 8 / 1e6;
        cout << "Node[" << i << "] " << nowThr << "\tpkts " << APsink[i]->GetTotalRx()/1024 << "\n" ; 
        AllNodesPastPktNum[i] = APsink[i]->GetTotalRx();
        EachNodeThr[i] = nowThr;
    }
    //return AllNodesPastThr;
    double rate5_averagethr=0.0;
    double rate11_averagethr=0.0;
    for(uint32_t i=0 ; i<nWifi ; i++)
    {
        if(i<nWifi/2)
        {
            rate5_averagethr += EachNodeThr[i];
        }
        else
        {
            rate11_averagethr += EachNodeThr[i];
        }
    }
    rate5_averagethr /= (nWifi/2);
    rate11_averagethr /= (nWifi/2);
    Rate5Thr.Add(Simulator::Now().GetSeconds() , rate5_averagethr);
    Rate11Thr.Add(Simulator::Now().GetSeconds() , rate11_averagethr);

    Simulator::Schedule(Seconds(1.0) , &UDPCaculateThroughput , &APsink[0] , nWifi , &AllNodesPastPktNum[0]);
}

void UDP_LossRateMeasure(FlowMonitorHelper* flowmon , Ptr<FlowMonitor> monitor , uint32_t nWifi , Ipv4InterfaceContainer MobileInterface)
{
    double EachNodeLossRate[nWifi] = {0.0};
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmon->GetClassifier());
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();
    for(std::map<FlowId, FlowMonitor::FlowStats>::const_iterator set = stats.begin(); set != stats.end(); set++)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(set->first);
      for(uint32_t i=0 ; i<nWifi ; i++)
      {
          if(t.sourceAddress == MobileInterface.GetAddress(i))
          {
            EachNodeLossRate[i] = (double)(set->second.txPackets-set->second.rxPackets)/set->second.txPackets;
          }
      }
    }
    double rate5_averageloss=0.0;
    double rate11_averageloss=0.0;
    for(uint32_t i=0 ; i<nWifi ; i++)
    {
        cout << "Node[" << i << "]\tLossRate : " << EachNodeLossRate[i] << "\n";
        if(i<nWifi/2)
        {
            rate5_averageloss += EachNodeLossRate[i];
        }
        else
        {
            rate11_averageloss += EachNodeLossRate[i];
        }
    }
    rate5_averageloss /= (nWifi/2);
    rate11_averageloss /= (nWifi/2);
    Rate5LossPkt.Add(Simulator::Now ().GetSeconds (), rate5_averageloss);
    Rate11LossPkt.Add(Simulator::Now ().GetSeconds (), rate11_averageloss);

    Simulator::Schedule(Seconds(1.0) , &UDP_LossRateMeasure , flowmon , monitor , nWifi , MobileInterface);
}

uint32_t GetRecordFileAmount()
{
    const char* path = "./ApplicationRecord/BinaryRecord/";
    DIR* dir;
    struct dirent* entry;
    uint32_t count = 0;

    if ((dir = opendir(path)) != NULL) {
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_REG) {
                count++;
            }
        }
        closedir(dir);
    } else {
        std::cerr << "Error opening directory: " << path << std::endl;
    }

    return count;
}

void SetNodeInfo(uint32_t nSta, double* NodeRate, double (*NodesLocatioon)[3], double (*NodesAppWorkTime)[2])
{
    for(uint32_t i=0 ; i<nSta ; i++)
    {
      std::string filePath = "./CoCaptureEffect/TextRecord/Node[" + std::to_string(i+1) + "]AppRecord.txt";

      std::ifstream ReadFile(filePath);
      if (!ReadFile.is_open())
      {
          std::cerr << "Error opening file: " << filePath << std::endl;
      }
      std::string line;
      while (std::getline(ReadFile, line)) 
      {
          std::regex buildChannelRegex("BuildCahnnel\\(([^,]+) , ([^,]+) , ([^,]+) , ([^,]+) , ([^\\)]+)\\)");
          std::regex scheduleRegex("Schedule\\(([^,]+) , ([^,]+)\\)");
          std::smatch match;
          if(std::regex_search(line , match , buildChannelRegex))
          {
            NodeRate[i] = std::stod(match[2]);
            NodesLocatioon[i][0] = std::stod(match[3]);
            NodesLocatioon[i][1] = std::stod(match[4]);
            NodesLocatioon[i][2] = std::stod(match[5]);
          }
          if(std::regex_search(line , match , scheduleRegex))
          {
              NodesAppWorkTime[i][0] = std::stod(match[1]);
              NodesAppWorkTime[i][1] = std::stod(match[2]);
          }
          cout << "The Line : " << line << std::endl;
      }
      ReadFile.close();
    }
}


int main(int argc , char *argv[])
{
    cout << "Begin" << endl;
    bool tracing = false;
    CommandLine cmd;
    //cmd.AddValue("nWifi" , "wifi STA num" , nWifi);
    cmd.AddValue("tracing" , "add pcap file?" , tracing);
    cout << "Start" << endl;
    double NodesRate[200] = {0.0};
    double NodesLocatioon[200][3];
    double NodesAppWorkTime[200][2];
    cout << "Init Succ1" << endl ;
    for(int i=0 ; i<200 ; i++)
    {
      for(int j=0 ; j<3 ; j++)
      {
        NodesLocatioon[i][j] = 0.0;
      }
      for(int k=0 ; k<2 ; k++)
      {
        NodesAppWorkTime[i][k] = 0.0;
      }
    }
    cout << "Init Succ2" << endl ;
    uint32_t nWifi = GetRecordFileAmount();
    cout << "Get NodeNum : " << nWifi << endl ;
    SetNodeInfo(nWifi , &NodesRate[0] , &NodesLocatioon[0] , &NodesAppWorkTime[0]);
    cout << "SetNodeInfo succ" << endl ;

    cmd.Parse(argc , argv);

    uint32_t AllNodesPastPktNum[nWifi] = {0}; //measurement

    Ptr<PacketSink> sink[nWifi];

    NodeContainer AccessPoint;
    AccessPoint.Create(1);

    NodeContainer MobileSTAs;
    MobileSTAs.Create(nWifi);

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper     phy     = YansWifiPhyHelper::Default();
    phy.SetChannel(channel.Create());

    WifiHelper wifi , WifiRate5 , WifiRate11;
    wifi.SetRemoteStationManager("ns3::AarfWifiManager");
    //wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager" , "DataMode" , StringValue("DsssRate11Mbps"));
    WifiRate5.SetRemoteStationManager("ns3::ConstantRateWifiManager" , "DataMode" , StringValue("DsssRate5_5Mbps"));
    WifiRate11.SetRemoteStationManager("ns3::ConstantRateWifiManager" , "DataMode" , StringValue("DsssRate11Mbps"));
    wifi.SetStandard(ns3::WIFI_PHY_STANDARD_80211b);
    WifiRate5.SetStandard(ns3::WIFI_PHY_STANDARD_80211b);
    WifiRate11.SetStandard(ns3::WIFI_PHY_STANDARD_80211b);

    WifiMacHelper mac;
    Ssid ssid = Ssid("NCS0912-Env");
    mac.SetType("ns3::StaWifiMac",
                "Ssid" , SsidValue(ssid),
                "ActiveProbing" , BooleanValue(false));

    NodeContainer STARate5 , STARate11;

    for(uint32_t i=0 ; i<nWifi ; i++)
    {
      if(NodesRate[i] == 5.0)
      {
        STARate5.Add(MobileSTAs.Get(i));
      }
      else if(NodesRate[i] == 11.0)
      {
        STARate11.Add(MobileSTAs.Get(i));
      }
    }

    //int HalfNodes = int(nWifi/2);
    NetDeviceContainer staDevicesRate5;
    staDevicesRate5 = WifiRate5.Install(phy , mac , STARate5);
    NetDeviceContainer staDevicesRate11;
    staDevicesRate11 = WifiRate11.Install(phy , mac , STARate11);

    mac.SetType("ns3::ApWifiMac",
                "Ssid" , SsidValue(ssid));

    NetDeviceContainer apDevices;
    apDevices = wifi.Install(phy , mac , AccessPoint);

    MobilityHelper mobility;

  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

  positionAlloc->Add (Vector (0.0, 0.0, 0.0));

  for (uint32_t i = 0; i < nWifi; i++)
  {
    positionAlloc->Add(Vector(NodesLocatioon[i][0] , NodesLocatioon[i][1] , NodesLocatioon[i][2]));
  }

  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (AccessPoint);
  mobility.Install (MobileSTAs);

  InternetStackHelper stack;
  stack.Install(AccessPoint);
  stack.Install(MobileSTAs);

  Ipv4AddressHelper address;

  address.SetBase("10.1.1.0" , "255.255.255.0");
  //address.Assign(staDevicesRate5);
  //address.Assign(staDevicesRate11);
  Ipv4InterfaceContainer MobileInterface;
  MobileInterface.Add(address.Assign(staDevicesRate5));
  MobileInterface.Add(address.Assign(staDevicesRate11));

  Ipv4InterfaceContainer apInterface;

  apInterface = address.Assign(apDevices);

    /*for(uint32_t i=0 ; i<nWifi ; i++)  //measurement
    {
      PacketSinkHelper sinkHelper("ns3::UdpSocketFactory" , InetSocketAddress(Ipv4Address::GetAny (), 9));
      ApplicationContainer sinkApp = sinkHelper.Install(MobileSTAs.Get(i));
      sinkApp.Start(Seconds(0.0));
      sink[i] = StaticCast<PacketSink> (sinkApp.Get(0));
    }*/

  for(uint32_t i=0 ; i<nWifi ; i++)  //measurement
  {
      PacketSinkHelper sinkHelper("ns3::UdpSocketFactory" , InetSocketAddress(apInterface.GetAddress(0) , 9+i));
      ApplicationContainer APsinkApp = sinkHelper.Install(AccessPoint.Get(0));
      APsinkApp.Start(Seconds(0.0));
      sink[i] = StaticCast<PacketSink> (APsinkApp.Get(0));
  }

    /*PacketSinkHelper APsinkHelper("ns3::UdpSocketFactory" , InetSocketAddress(apInterface.GetAddress(0) , 9));
    ApplicationContainer APsinkApp = APsinkHelper.Install(AccessPoint.Get(0));
    APsinkApp.Start(Seconds(0.0));
    Ptr<PacketSink> APsink = StaticCast<PacketSink>(APsinkApp.Get(0));
    uint32_t APLastPktNum = 0;*/

    //for testing topo

  for(uint32_t i=0 ; i<nWifi ; i++)
  {
    OnOffHelper server ("ns3::UdpSocketFactory", (InetSocketAddress (apInterface.GetAddress(0), 9+i)));
    server.SetAttribute ("PacketSize", UintegerValue (1024)); // G.711 encoding, 20ms delay, 160bytes +12bytes(rtp)
    server.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
    server.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
    //server.SetAttribute ("DataRate", DataRateValue (DataRate ("1Mbps")));
    ApplicationContainer serverApp = server.Install(MobileSTAs.Get(i));
    serverApp.Start(Seconds (NodesAppWorkTime[i][0]));
  }

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  // to measure packet loss by using flow monitor
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll ();


  Simulator::Schedule(Seconds(1.0) , &UDPCaculateThroughput , &sink[0] , nWifi , &AllNodesPastPktNum[0]);
  //Simulator::Schedule(MilliSeconds(1.0) , &ServerDection , APsink , &APLastPktNum);
  Simulator::Schedule(Seconds(1.0) , &UDP_LossRateMeasure , &flowmon , monitor , nWifi , MobileInterface);
  
  double endtime=0.0;
  for(uint32_t i=0 ; i<nWifi ; i++)
  {
    if(NodesAppWorkTime[i][1] > endtime)
    {
      endtime = NodesAppWorkTime[i][1];
    }
  }
  cout << "End Time : " << endtime << endl;
  Simulator::Stop(Seconds(endtime));

  if(tracing)
  {
      phy.EnablePcap("Ncs0912" , apDevices.Get(0));
  }

  Simulator::Run();

  Gnuplot plot ("CpatureThr.png");
  plot.SetTerminal ("png");
  plot.SetLegend ("Time (sec)" , "Throughput (Mbps)");
  plot.SetExtra("set yrange [0:1]; set ytics 0,0.5,1");
  Rate5Thr.SetTitle("Throughput : rate5.5");
  Rate5Thr.SetStyle(Gnuplot2dDataset::LINES_POINTS);
  plot.AddDataset(Rate5Thr);
  Rate11Thr.SetTitle("Throughput : rate11");
  Rate11Thr.SetStyle(Gnuplot2dDataset::LINES_POINTS);
  plot.AddDataset(Rate11Thr);
  std::ofstream plotFile ("CaptureThr.plt");
  plot.GenerateOutput (plotFile);
  plotFile.close ();

  Gnuplot plot2 ("CpatureLoss.png");
  plot2.SetTerminal ("png");
  plot2.SetLegend ("Time (sec)" , "LossRate (Mbps)");
  plot2.SetExtra("set yrange [0:1]; set ytics 0,0.5,1");
  Rate5LossPkt.SetTitle("LossRate : rate5.5");
  Rate5LossPkt.SetStyle(Gnuplot2dDataset::LINES_POINTS);
  plot2.AddDataset(Rate5LossPkt);
  Rate11LossPkt.SetTitle("LossRate : rate11");
  Rate11LossPkt.SetStyle(Gnuplot2dDataset::LINES_POINTS);
  plot2.AddDataset(Rate11LossPkt);
  std::ofstream plotFile2 ("CaptureLossPkt.plt");
  plot2.GenerateOutput (plotFile2);
  plotFile2.close ();

  Simulator::Destroy();
  return 0;
}

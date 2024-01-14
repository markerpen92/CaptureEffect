# NS3-Capture Effect

## 目的
模擬節點與基地台(Access Point)之間的通訊，利用通訊建立彼此之間傳輸訊息的通道。
並基於此背景之上讓節點進行UDP傳輸，最後測量整體網路的吞吐量與碰撞率並證明Capture Effect的存在。

## 與計算機結構相關之處
基地台透過節點廣播的2進位編碼，建立雙方的傳輸通道，這樣做與作業系統嘗試透過組合語言分配任務或是記憶體有相似之處。

## 方法
+ 程式碼架構
![image](https://github.com/markerpen92/CaptureEffect/assets/99935143/c665607f-9c0a-4a8e-a251-d686a6bd519d)
  - 客戶端相關
![image](https://github.com/markerpen92/CaptureEffect/assets/99935143/4e13939b-cbdc-40f0-affa-1ec76a7e7c88)

  - 基地台相關
![image](https://github.com/markerpen92/CaptureEffect/assets/99935143/3de13f86-ba11-41bd-831b-d85fea8b0112)

+ 吞吐量與碰撞率結果
  - Throughput
![image](https://github.com/markerpen92/CaptureEffect/assets/99935143/5b28897e-e1a3-45ef-9637-b8cf7ff2f853)

  - Collision Rate
![image](https://github.com/markerpen92/CaptureEffect/assets/99935143/b21446e0-e949-4b17-83c5-259bd51560fa)

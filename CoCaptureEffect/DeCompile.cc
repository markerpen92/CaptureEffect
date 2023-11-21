// DeCompiler.cpp

#include "DeCompile.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include <bitset>

void DeCompiler::Decode(int nodeId) {
    std::string binaryFilePath = "/home/user/Desktop/NS3_TMP/CPUOS_Capture/ApplicationRecord/BinaryRecord/Node[" + std::to_string(nodeId) + "]BinaryRecord.txt";
    std::string utf8FilePath = "./TextRecord/Node[" + std::to_string(nodeId) + "]AppRecord.txt";

    std::ifstream binaryFile(binaryFilePath, std::ios::binary);
    if (!binaryFile) 
    {
        std::cerr << "Error opening binary file: " << binaryFilePath << std::endl;
        return;
    }

    std::ofstream utf8File(utf8FilePath);
    if (!utf8File) 
    {
        std::cerr << "Error opening UTF-8 file: " << utf8FilePath << std::endl;
        return;
    }

    std::string line;
    std::string utf8str;
    while (std::getline(binaryFile, line)) 
    {
        std::vector<char> binaryData(line.begin(), line.end());
        utf8str = BinaryToUtf8(binaryData);
        utf8File << utf8str << std::endl;
    }

    binaryFile.close();
    utf8File.close();

    std::cout << "Conversion complete: " << utf8FilePath << std::endl;
}

uint32_t DeCompiler::RequestAmount() {
    const char* folderPath = "/home/user/Desktop/NS3_TMP/CPUOS_Capture/ApplicationRecord/BinaryRecord/";
    uint32_t fileCount = 0;

    DIR* dir;
    struct dirent* ent;

    if ((dir = opendir(folderPath)) != nullptr) {
        while ((ent = readdir(dir)) != nullptr) {
            struct stat st;
            std::string filePath = std::string(folderPath) + std::string(ent->d_name);

            if (stat(filePath.c_str(), &st) == 0 && S_ISREG(st.st_mode)) {
                fileCount++;
            }
        }

        closedir(dir);
        std::cout << "Number of files in the folder: " << fileCount << std::endl;
    } else {
        perror("Error opening folder");
    }

    return fileCount;
}


//the function is refer by ChatGPT
std::string DeCompiler::BinaryToUtf8(const std::vector<char>& binaryData) {
    std::string utf8String;
    for (size_t i = 0; i < binaryData.size(); i += 8) {
        std::bitset<8> byte(&binaryData[i]);
        utf8String.push_back(static_cast<char>(byte.to_ulong()));
    }
    return utf8String;
}
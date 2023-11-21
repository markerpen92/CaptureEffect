// DeCompiler.h

#ifndef DECOMPILER_H
#define DECOMPILER_H

#include <string>
#include <vector>

class DeCompiler {
public:
    void Decode(int nodeId);
    uint32_t RequestAmount();

private:
    std::string BinaryToUtf8(const std::vector<char>& binaryData);
};

#endif  // DECOMPILER_H

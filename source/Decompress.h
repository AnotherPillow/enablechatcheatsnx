#include <iostream>
#include <vector>
#include <stdexcept>
#include <zlib.h>
// #include "zlib/zlib.h"


std::vector<char> decompressZlib(const std::vector<char>& compressedData) {
    std::vector<char> empty_vector(1);
    uLongf decompressedSize = compressedData.size() * 4; 
    std::vector<char> decompressedData(decompressedSize);

    int result;
    while ((result = uncompress(reinterpret_cast<Bytef*>(decompressedData.data()), &decompressedSize,
                                reinterpret_cast<const Bytef*>(compressedData.data()), compressedData.size())) == Z_BUF_ERROR) {
        decompressedSize *= 2;
        decompressedData.resize(decompressedSize);
    }

    if (result != Z_OK) {
        printf("Failed to decompress save\n");
        return empty_vector;
        
    }

    decompressedData.resize(decompressedSize);
    return decompressedData;
}
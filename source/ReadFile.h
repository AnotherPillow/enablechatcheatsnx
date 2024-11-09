#include <iostream>
#include <fstream>
#include <vector>

inline std::vector<char> readFileToBuffer(const std::string& filePath) {
    std::vector<char> empty_vector(1);
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        printf("Failed to open the file: %s", filePath.c_str());
        return empty_vector;
    }

    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);  // Rewind the file

    std::vector<char> buffer(fileSize);

    if (!file.read(buffer.data(), fileSize)) {
        printf("Failed to read the file: %s", filePath.c_str());
        return empty_vector;
    }

    return buffer;
}
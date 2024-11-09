#include <iostream>
#include <fstream>
#include <string>

void writeStringToFile(const std::string& str, const std::string& filename) {
    std::ofstream outfile(filename, std::ios::binary);
    
    if (!outfile) {
        printf("Failed to open %s\n", filename.c_str());
        return;
    }

    outfile.write(str.c_str(), str.size());

    outfile.close();

    printf("%s written successfully.\n", filename.c_str());
}
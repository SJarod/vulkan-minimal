#pragma once

#include <fstream>
#include <string>
#include <vector>

/**
 * return success
 */
static inline bool read_binary_file(const std::string &filename, std::vector<char> &out)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file : " << filename << std::endl;
        return false;
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    out.resize(fileSize);

    file.seekg(0);
    file.read(out.data(), fileSize);

    file.close();
    return true;
}

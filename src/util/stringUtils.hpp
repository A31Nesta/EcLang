#pragma once

// STD
#include <string>
#include <vector>
#include <cstdint>

namespace eclang::string {
    // trim from start (in place)
    void ltrim(std::string &s);

    // trim from end (in place)
    void rtrim(std::string &s);
    std::string trim(std::string s);

    // Separates a string of comma separated values into a string vector.
    std::vector<std::string> separateString(std::string s);

    // Modified, base function taken from: https://stackoverflow.com/a/24315631
    std::string tabsToSpaces(std::string str);

    // Utilities
    uint8_t getIndentationLevel(std::string s);
    std::string setIndentationLv(std::string s, uint8_t targetIndentation);

    // Is the string part of the vector?
    bool contains(std::vector<std::string>& v,std::string s);
    bool contains(const std::vector<std::string>& v,std::string s);
    uint64_t indexOf(std::vector<std::string>& v,std::string s);
    uint64_t indexOf(const std::vector<std::string>& v,std::string s);
}
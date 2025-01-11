#include "stringUtils.hpp"

#include <sstream>
#include <algorithm>

namespace eclang::string {
    // String Trim
    // Taken from: https://stackoverflow.com/a/217605
    // trim from start (in place)
    void ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
    }
    // trim from end (in place)
    void rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), s.end());
    }
    std::string trim(std::string s) {
        rtrim(s);
        ltrim(s);
        return s;
    }

    // Separates a string of comma separated values into a string vector.
    std::vector<std::string> separateString(std::string s) {
        std::vector<std::string> out;

        std::stringstream stream(s);
        std::string line; // where we store each value

        // getline
        while (std::getline(stream, line, ',')) {
            // We also trim the line to eliminate any white space before or after the comma
            out.push_back(trim(line));
        }
        return out;
    }

    // Modified, base function taken from: https://stackoverflow.com/a/24315631
    std::string tabsToSpaces(std::string str) {
        size_t start_pos = 0;
        while((start_pos = str.find("\t", start_pos)) != std::string::npos) {
            str.replace(start_pos, 1, "    ");
            start_pos += 4;
        }
        return str;
    }

    // Utilities
    uint8_t getIndentationLevel(std::string s) {
        uint64_t strSizeOrig = s.length();
        ltrim(s);
        uint64_t strSizeNew = s.length();
        return strSizeOrig - strSizeNew;
    }
    std::string setIndentationLv(std::string s, uint8_t targetIndentation) {
        uint8_t currentIndentation = getIndentationLevel(s);
        if (currentIndentation < targetIndentation) {
            ltrim(s);
            return s;
        }
        else if (targetIndentation < s.length()) {
            return s.substr(targetIndentation);
        }
        return s;
    }
    // Is the string part of the vector?
    bool contains(std::vector<std::string>& v,std::string s) {
        return std::find(v.begin(), v.end(), s) != v.end();
    }
    bool contains(const std::vector<std::string>& v,std::string s) {
        return std::find(v.begin(), v.end(), s) != v.end();
    }
    uint64_t indexOf(std::vector<std::string>& v,std::string s) {
        return std::distance(v.begin(), std::find(v.begin(), v.end(), s));
    }
    uint64_t indexOf(const std::vector<std::string>& v,std::string s) {
        return std::distance(v.begin(), std::find(v.begin(), v.end(), s));
    }
}
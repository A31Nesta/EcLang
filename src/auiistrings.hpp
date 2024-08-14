#pragma once

#include <cstdint>
#include <string>
#include <vector>

enum StringTypes {
    NONE,
    NORMAL,
    MARKDOWN
};

uint8_t wordIsString(std::string s, bool& inString, StringTypes& strType);
bool endsInString(std::string s, bool inString, StringTypes& strType);

// Get flags and string data
bool isNormalString(std::string s);
bool isMarkdownString(std::string s);
std::vector<uint8_t> getStringData(std::string s);
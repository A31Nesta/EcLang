#include <auiistrings.hpp>

uint8_t wordIsString(std::string s, bool& inString, StringTypes& strType) {
    std::string lineCheckStr = s;
    bool checking = true;
    bool found = false;
    
    while (checking) {
        uint64_t normalIndex = lineCheckStr.find_first_of("\"");
        uint64_t mdIndex     = lineCheckStr.find_first_of("```");
        bool validNormal = normalIndex != std::string::npos;
        bool validMarkdown = mdIndex != std::string::npos;

        // No quotes? Then exit
        if (!(validNormal || validMarkdown)) {
            checking = false;
            continue;
        }

        // If we weren't in a string we now are :)
        // We make sure to register the type of String we are in
        if (!inString) {
            inString = true;
            found = true;

            uint64_t index = std::string::npos;
            StringTypes strTypeSet = NONE;
            if (validMarkdown && validNormal) {
                // If both are valid choose the one with the lowest value (found first)
                index = (normalIndex<mdIndex)?normalIndex:mdIndex;
                strTypeSet = (normalIndex<mdIndex)?NORMAL:MARKDOWN;    
            }
            else if (validMarkdown && !validNormal) {
                strTypeSet = MARKDOWN;
                index = mdIndex;
            }
            else if (!validMarkdown && validNormal) {
                strTypeSet = NORMAL;
                index = normalIndex;
            }

            // Are quotes escaped?
            if (index > 0) {
                if (lineCheckStr.at(index-1) == '\\') {
                    lineCheckStr = lineCheckStr.substr(index+1);
                    inString = false;
                    continue;
                }
            }

            strType = strTypeSet;
            lineCheckStr = lineCheckStr.substr(index+1);
        }
        // If we ARE on a string we will only accept the symbol of the same type as this string
        // Example: On a Markdown String we will ignore these (") and only detect these (```)
        else {
            if (strType == NORMAL && validNormal) {
                if (normalIndex > 0) {
                    if (lineCheckStr.at(normalIndex-1) == '\\') {
                        lineCheckStr = lineCheckStr.substr(normalIndex+1);
                        continue;
                    }
                }

                inString = false;
                found = true;
                lineCheckStr = lineCheckStr.substr(normalIndex+1);
            }
            else if (strType == MARKDOWN && validMarkdown) {
                if (mdIndex > 0) {
                    if (lineCheckStr.at(mdIndex-1) == '\\') {
                        lineCheckStr = lineCheckStr.substr(mdIndex+1);
                        continue;
                    }
                }

                inString = false;
                found = true;
                lineCheckStr = lineCheckStr.substr(mdIndex+1);
            }
            else {
                // Exit, no quote found
                checking = false;
            }
        }
    }

    if (found) {
        return 1;
    }
    return inString?2:0;
}
bool endsInString(std::string s, bool inString, StringTypes& strType) {
    wordIsString(s, inString, strType);
    return inString;
}

// Get flags and string data
bool isNormalString(std::string s) {
    uint64_t indexOfNormal = s.find_first_of("\"");
    uint64_t indexOfMarkdown = s.find_first_of("```");
    uint64_t indexOfQuotes = (indexOfNormal < indexOfMarkdown)?indexOfNormal:indexOfMarkdown;
    
    if (indexOfQuotes == std::string::npos) {
        return false;
    }
    if (indexOfQuotes == indexOfMarkdown) {
        return false;
    }
    return true;
}
bool isMarkdownString(std::string s) {
    uint64_t indexOfNormal = s.find_first_of("\"");
    uint64_t indexOfMarkdown = s.find_first_of("```");
    uint64_t indexOfQuotes = (indexOfNormal < indexOfMarkdown)?indexOfNormal:indexOfMarkdown;
    
    if (indexOfQuotes == std::string::npos) {
        return false;
    }
    if (indexOfQuotes == indexOfNormal) {
        return false;
    }
    return true;
}
std::vector<uint8_t> getStringData(std::string s) {
    std::vector<uint8_t> data;

    uint64_t indexOfNormal = s.find_first_of("\"");
    uint64_t indexOfMarkdown = s.find_first_of("```");
    uint64_t indexOfQuotes = (indexOfNormal < indexOfMarkdown)?indexOfNormal:indexOfMarkdown;

    // Fuck!
    if (indexOfQuotes == std::string::npos) {
        throw "Data was not a String";
    }

    if (indexOfQuotes == indexOfNormal) {
        uint8_t flags = 0;
        if (indexOfQuotes != 0) {
            for (uint8_t i = 0; i < indexOfQuotes; i++) {
                char c = s.at(i);
                switch (c) {
                    case 'f': // Files
                        flags = flags | 1;
                        break;
                }
            }
        }

        std::string cleanString = s.substr(indexOfQuotes+1);
        cleanString = cleanString.substr(0, cleanString.find_last_of("\""));

        data.push_back(0); // poot string type here
        data.push_back(flags); // pootis flags here
        for (char c : cleanString) {
            data.push_back(c); // poot data here
        }
        data.push_back(0); // poot null terminator here
    }
    else {
        std::string cleanString = s.substr(indexOfQuotes+3);
        cleanString = cleanString.substr(0, cleanString.find_last_of("```")-2);

        data.push_back(1); // poot string type here
        for (char c : cleanString) {
            data.push_back(c); // poot data here
        }
        data.push_back(0); // poot null terminator here
    }

    return data; // thanks engie :D
}
#include <stringutils.hpp>
#include <instructionstrings.hpp>
#include <constants.hpp>

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

std::vector<std::string> auiiStringFromBEEF(const std::vector<uint8_t> beef, uint64_t& beefIndex) {
    uint8_t strType = beef.at(beefIndex);

    std::string flags;
    if (strType == 0) {
        flags += '0';

        uint8_t strFlag = beef.at(beefIndex+1);
        if (strFlag & 1) { // String is File Path
            flags += 'f';
        }
        if (strFlag & 2) { // Wide String (unused probably)
            flags += 'w';
        }

        beefIndex += 2;
    }
    else {
        flags += '1';
        beefIndex += 1;
    }

    std::string strData;

    uint64_t j = 0;
    bool searching = true;
    while (searching) {
        if (beef.at(beefIndex+j) == 0) {
            beefIndex += j+1;
            searching = false;
        }
        else {
            strData += (char)beef.at(beefIndex+j);
        }
        j++;
    }

    return {flags, strData};
}

std::vector<std::string> meatInstructionToString(NodeTypes nodeTypes, PropertiesMap properties, const std::vector<uint8_t> beef, uint64_t& beefIndex, uint8_t instID) {
    // Prepare vector with string data
    std::vector<std::string> data;

    uint8_t propertyID = instID - 10;
    
    std::string propertyName = "";
    std::string propertyTypes = "";

    // Search for property name knowing only the ID
    uint8_t i = 0;
    for (std::pair<std::string, std::string> prop : properties) {
        // std::cout << std::to_string(i) << ". " << prop.first << ", " << prop.second << " | " << std::to_string(propertyID) << "\n";
        if (propertyID == i) {
            propertyName = prop.first;
            propertyTypes = prop.second;
            // std::cout << propertyName << ", " << propertyTypes << " <=====\n";
        }
        i++;
    }

    // Check: It has been found
    if (propertyName.empty()) {
        beefIndex++;
        return {};
    }

    // Put name in array
    data.push_back(propertyName);

    // Loop to extract data from instruction.
    std::vector<std::string> argumentTypes = separateString(propertyTypes);

    beefIndex++;
    for (std::string argType : argumentTypes) {
        if (argType == "String" || argType == "MDString" || argType == "StringMD" || argType == "StringAny") {
            // std::cout << beefIndex << " A\n";
            std::vector<std::string> auiiString = auiiStringFromBEEF(beef, beefIndex);
            // std::cout << beefIndex << " B\n";

            // Put flags and string content in array
            data.push_back(auiiString.at(0));
            data.push_back(auiiString.at(1));
        }

        // Bytes
        else if (argType == "uint8_t") {
            uint8_t number = beef.at(beefIndex);
            beefIndex++;

            data.push_back(std::to_string(number));
        }
        else if (argType == "byte") {
            int8_t number = *reinterpret_cast<int8_t*>(beef.at(beefIndex));
            beefIndex++;

            data.push_back(std::to_string(number));
        }

        // Shorts
        else if (argType == "uint16_t") {
            uint8_t num[2] = {beef.at(beefIndex), beef.at(beefIndex+1)};
            uint16_t number = *reinterpret_cast<uint16_t*>(num);
            beefIndex += 2;

            data.push_back(std::to_string(number));
        }
        else if (argType == "short") {
            uint8_t num[2] = {beef.at(beefIndex), beef.at(beefIndex+1)};
            int16_t number = *reinterpret_cast<int16_t*>(num);
            beefIndex += 2;

            data.push_back(std::to_string(number));
        }

        // Ints
        else if (argType == "uint32_t") {
            uint8_t num[4] = {beef.at(beefIndex), beef.at(beefIndex+1), beef.at(beefIndex+2), beef.at(beefIndex+3)};
            uint32_t number = *reinterpret_cast<uint32_t*>(num);
            beefIndex += 4;

            data.push_back(std::to_string(number));
        }
        else if (argType == "int") {
            uint8_t num[4] = {beef.at(beefIndex), beef.at(beefIndex+1), beef.at(beefIndex+2), beef.at(beefIndex+3)};
            int32_t number = *reinterpret_cast<int32_t*>(num);
            beefIndex += 4;

            data.push_back(std::to_string(number));
        }

        // Longs
        else if (argType == "uint64_t") {
            uint8_t num[8] = {
                beef.at(beefIndex),
                beef.at(beefIndex+1),
                beef.at(beefIndex+2),
                beef.at(beefIndex+3),
                beef.at(beefIndex+4),
                beef.at(beefIndex+5),
                beef.at(beefIndex+6),
                beef.at(beefIndex+7)
            };
            uint64_t number = *reinterpret_cast<uint64_t*>(num);
            beefIndex += 8;

            data.push_back(std::to_string(number));
        }
        else if (argType == "long") {
            uint8_t num[8] = {
                beef.at(beefIndex),
                beef.at(beefIndex+1),
                beef.at(beefIndex+2),
                beef.at(beefIndex+3),
                beef.at(beefIndex+4),
                beef.at(beefIndex+5),
                beef.at(beefIndex+6),
                beef.at(beefIndex+7)
            };
            int64_t number = *reinterpret_cast<int64_t*>(num);
            beefIndex += 8;

            data.push_back(std::to_string(number));
        }

        // Floats
        else if (argType == "float") {
            uint8_t num[4] = {beef.at(beefIndex), beef.at(beefIndex+1), beef.at(beefIndex+2), beef.at(beefIndex+3)};
            float number = *reinterpret_cast<float*>(num);
            beefIndex += 4;

            data.push_back(std::to_string(number));
        }
        else if (argType == "double") {
            uint8_t num[8] = {
                beef.at(beefIndex),
                beef.at(beefIndex+1),
                beef.at(beefIndex+2),
                beef.at(beefIndex+3),
                beef.at(beefIndex+4),
                beef.at(beefIndex+5),
                beef.at(beefIndex+6),
                beef.at(beefIndex+7)
            };
            double number = *reinterpret_cast<double*>(num);
            beefIndex += 8;

            data.push_back(std::to_string(number));
        }
    }

    return data;
}

std::vector<std::vector<std::string>> meatToStrings(NodeTypes nodeTypes, PropertiesMap properties, const std::vector<uint8_t> beef) {
    // std::cout << "Called BEEFtoStrings()\n";
    std::vector<std::vector<std::string>> data;

    uint64_t beefIndex = 0;
    bool readingBEEF = true;

    while (readingBEEF) {
        uint8_t instID = beef.at(beefIndex);

        switch (instID) {
        case 0: // Change in Scope
            {
                std::vector<uint8_t> intData;
                for (uint8_t i = 1; i <= 4; i++) {
                    intData.push_back(beef.at(beefIndex+i));
                }
                uint32_t object = *reinterpret_cast<uint32_t*>(intData.data());
                
                beefIndex += 5;
                data.push_back({"scope", std::to_string(object)}); // Set array
            }
            break;
        case 1: // Object Creation (create, Type, Name)
            {
                uint8_t auiiDataType = beef.at(beefIndex+1);

                std::string auiiDataTypeStr;
                auiiDataTypeStr = nodeTypes.at(auiiDataType);

                beefIndex += 2;
                std::vector<std::string> auiiString = auiiStringFromBEEF(beef, beefIndex);

                data.push_back({"create", auiiDataTypeStr, auiiString.at(1)}); // Set array
            }
            break;
        case 2: // Custom property (custom, Name, Value)
            {
                beefIndex += 1;
                std::vector<std::string> name = auiiStringFromBEEF(beef, beefIndex);
                std::vector<std::string> value = auiiStringFromBEEF(beef, beefIndex);

                data.push_back({"custom", name.at(1), value.at(1)}); // Set array
            }
            break;
        case 3: // Inclusion (include, file to include)
            {
                beefIndex += 1;
                std::vector<std::string> value = auiiStringFromBEEF(beef, beefIndex);

                data.push_back({"include", value.at(1)}); // Set array
            }
            break;
        case 4: // Registration (register, name, file to register)
            {
                beefIndex += 1;
                std::vector<std::string> name = auiiStringFromBEEF(beef, beefIndex);
                beefIndex -= 1; // TODO: Investigate why this works lmao.
                std::vector<std::string> value = auiiStringFromBEEF(beef, beefIndex);

                // std::cout << "Name: " + name.at(1) + " | Value: " + value.at(1) + "\n";
                data.push_back({"register", name.at(1), value.at(1)}); // Set array
            }
            break;
        
        default:
            {
                std::vector<std::string> vec = meatInstructionToString(nodeTypes, properties, beef, beefIndex, instID);
                data.push_back(vec);
            }
        }

        // std::cout << beefIndex << " / " << beef.size() << "\n";

        if (beefIndex >= beef.size()) {
            readingBEEF = false;
        }
    }

    // return {};
    return data;
}
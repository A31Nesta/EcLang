/**
    Meatgrinder: The Meat compiler
    ------------------------------
    By A31Nesta

    C++ Functions that allow the
    creation of compilers of Node-
    based languages used by SELibs

    The languages that this
    library has been designed for
    are AmbigUII (using WASM) and
    SelAmb (for Selengine)
*/

#include <iterator>
#include <meatgrinder.hpp>
#include <auiistrings.hpp>
#include <stringutils.hpp>
#include <constants.hpp>

// STD
#include <sstream>
#include <string>
#include <vector>
#include <cstdint>
#include <fstream>
#include <iostream>

// TODO: Refactor code, remove repeating structures
std::vector<uint8_t> setProperty(PropertiesMap properties, std::string word, std::string argument) {
    std::vector<uint8_t> data;

    // Check: Is the name of the property valid? (registered)
    if (properties.find(word) == properties.end()) {
        throw "No property with name " + word + " registered";
    }

    // Get argument types needed for property (`word`)
    std::string argTypesString = properties.at(word);

    // Get vectors with argument values found in the `arguments` variable
    // and with argument types.
    std::vector<std::string> arguments = separateString(argument);
    std::vector<std::string> argumentTypes = separateString(argTypesString);

    // If length doesn't match, we have a problem
    if (arguments.size() != argumentTypes.size()) {
        throw "Not enough arguments introduced for property \"" + word + "\". Expected " + std::to_string(argumentTypes.size()) + ", got " + std::to_string(arguments.size());
    }

    // Enter the instruction ID into the binary file.
    // The instruction ID will always be whatever the index in the map is + 10.
    // The first 10 spaces are reserved for Meatgrinder stuff like changing scope or declaring nodes
    uint8_t instructionID = std::distance(properties.begin(), properties.find(word)) + 10; // Possibly cursed but if it works it works.
    data.push_back(instructionID);

    // Now, in a loop, place the binary data of all the parameters
    for (uint8_t i = 0; i < argumentTypes.size(); i++) {
        std::string arg = arguments.at(i);
        std::string argType = argumentTypes.at(i);

        // We do different stuff for each data type
        if (argType == "String") {
            // Check: Is this argument actually an AUII String?
            if (!isNormalString(arg)) {
                throw "Argument was not a Normal String";
            }

            // Get binary data from the string formatted for AUII
            std::vector<uint8_t> stringData = getStringData(argument);
            // Insert the binary data to the data array
            data.insert(data.end(), stringData.begin(), stringData.end());
        }
        else if (argType == "MDString" || argType == "StringMD") {
            // Check: Is this argument actually a Markdown AUII String?
            if (!isMarkdownString(arg)) {
                throw "Argument was not a Markdown String";
            }

            // Get binary data from the string formatted for AUII
            std::vector<uint8_t> stringData = getStringData(argument);
            // Insert the binary data to the data array
            data.insert(data.end(), stringData.begin(), stringData.end());
        }
        else if (argType == "StringAny") {
            // Check: Is this argument actually an AUII String of any kind?
            if (!isNormalString(argument) && !isMarkdownString(argument)) {
                throw "Argument was not a String";
            }

            // Get binary data from the string formatted for AUII
            std::vector<uint8_t> stringData = getStringData(argument);
            // Insert the binary data to the data array
            data.insert(data.end(), stringData.begin(), stringData.end());
        }

        // BYTES
        else if (argType == "uint8_t") {
            // Check: Is this argument really a number?
            int convertedInt = 0;
            try {
                convertedInt = std::stoi(arg);
            } catch (...) {
                throw "Expected numeric (uint8_t) argument for property \"" + word + "\", got \"" + arg + "\" instead";
            }

            // Check: Is this number really a byte?
            if (convertedInt < 0 || convertedInt > UINT8_MAX) {
                throw "Expected uint8_t (0-255) argument for property \"" + word + "\", got \"" + arg + "\" instead";
            }

            // All checks passed, put this byte into the data array
            uint8_t convertedByte = convertedInt;
            data.push_back(convertedByte);
        }
        else if (argType == "byte") {
            // Check: Is this argument really a number?
            int convertedInt = 0;
            try {
                convertedInt = std::stoi(arg);
            } catch (...) {
                throw "Expected numeric (byte) argument for property \"" + word + "\", got \"" + arg + "\" instead";
            }

            // Check: Is this number really a byte?
            if (convertedInt < INT8_MIN || convertedInt > INT8_MAX) {
                throw "Expected byte (-128 - 127) argument for property \"" + word + "\", got \"" + arg + "\" instead";
            }

            // All checks passed, put this byte into the data array
            int8_t convertedByte = convertedInt;
            data.push_back(convertedByte);
        }

        // SHORTS
        else if (argType == "uint16_t") {
             // Check: Is this argument really a number?
            int convertedInt = 0;
            try {
                convertedInt = std::stoi(arg);
            } catch (...) {
                throw "Expected numeric (uint16_t) argument for property \"" + word + "\", got \"" + arg + "\" instead";
            }

            // Check: Is this number really a short?
            if (convertedInt < 0 || convertedInt > UINT16_MAX) {
                throw "Expected uint16_t (0-65535) argument for property \"" + word + "\", got \"" + arg + "\" instead";
            }

            // All checks passed, put this short into the data array
            uint16_t convertedShort = convertedInt;
            uint8_t* convertedBytes = reinterpret_cast<uint8_t*>(&convertedShort);
            data.push_back(convertedBytes[0]);
            data.push_back(convertedBytes[1]);
        }
        else if (argType == "short") {
             // Check: Is this argument really a number?
            int convertedInt = 0;
            try {
                convertedInt = std::stoi(arg);
            } catch (...) {
                throw "Expected numeric (short) argument for property \"" + word + "\", got \"" + arg + "\" instead";
            }

            // Check: Is this number really a short?
            if (convertedInt < INT16_MIN || convertedInt > INT16_MAX) {
                throw "Expected short (-32768 - 32767) argument for property \"" + word + "\", got \"" + arg + "\" instead";
            }

            // All checks passed, put this short into the data array
            int16_t convertedShort = convertedInt;
            uint8_t* convertedBytes = reinterpret_cast<uint8_t*>(&convertedShort);
            data.push_back(convertedBytes[0]);
            data.push_back(convertedBytes[1]);
        }

        // INTS
        else if (argType == "uint32_t") {
            // Check: Is this argument really a number?
            uint64_t convertedLong = 0;
            try {
                convertedLong = std::stoul(arg);
            } catch (...) {
                throw "Expected numeric (uint32_t) argument for property \"" + word + "\", got \"" + arg + "\" instead";
            }

            // Check: Is this number really an int?
            if (convertedLong < 0 || convertedLong > UINT32_MAX) {
                throw "Expected uint32_t argument for property \"" + word + "\", got \"" + arg + "\" instead";
            }

            // All checks passed, put this int into the data array
            uint32_t convertedInt = convertedLong;
            uint8_t* convertedBytes = reinterpret_cast<uint8_t*>(&convertedInt);
            data.push_back(convertedBytes[0]);
            data.push_back(convertedBytes[1]);
            data.push_back(convertedBytes[2]);
            data.push_back(convertedBytes[3]);
        }
        else if (argType == "int") {
            // Check: Is this argument really a number?
            int32_t convertedInt = 0;
            try {
                convertedInt = std::stoi(arg);
            } catch (...) {
                throw "Expected numeric (int) argument for property \"" + word + "\", got \"" + arg + "\" instead";
            }

            // All checks passed, put this int into the data array
            uint8_t* convertedBytes = reinterpret_cast<uint8_t*>(&convertedInt);
            data.push_back(convertedBytes[0]);
            data.push_back(convertedBytes[1]);
            data.push_back(convertedBytes[2]);
            data.push_back(convertedBytes[3]);
        }

        // LONGS
        else if (argType == "uint64_t") {
            // Check: Is this argument really a number?
            uint64_t convertedLong = 0;
            try {
                convertedLong = std::stoul(arg);
            } catch (...) {
                throw "Expected numeric (uint64_t) argument for property \"" + word + "\", got \"" + arg + "\" instead";
            }

            // All checks passed, put this int into the data array
            uint8_t* convertedBytes = reinterpret_cast<uint8_t*>(&convertedLong);
            data.push_back(convertedBytes[0]);
            data.push_back(convertedBytes[1]);
            data.push_back(convertedBytes[2]);
            data.push_back(convertedBytes[3]);
            data.push_back(convertedBytes[4]);
            data.push_back(convertedBytes[5]);
            data.push_back(convertedBytes[6]);
            data.push_back(convertedBytes[7]);
        }
        else if (argType == "long") {
            // Check: Is this argument really a number?
            int64_t convertedLong = 0;
            try {
                convertedLong = std::stol(arg);
            } catch (...) {
                throw "Expected numeric (long) argument for property \"" + word + "\", got \"" + arg + "\" instead";
            }

            // All checks passed, put this int into the data array
            uint8_t* convertedBytes = reinterpret_cast<uint8_t*>(&convertedLong);
            data.push_back(convertedBytes[0]);
            data.push_back(convertedBytes[1]);
            data.push_back(convertedBytes[2]);
            data.push_back(convertedBytes[3]);
            data.push_back(convertedBytes[4]);
            data.push_back(convertedBytes[5]);
            data.push_back(convertedBytes[6]);
            data.push_back(convertedBytes[7]);
        }

        // FLOAT AND DOUBLE
        else if (argType == "float") {
            // Check: Is this argument really a number?
            float convertedFloat = 0;
            try {
                convertedFloat = std::stof(arg);
            } catch (...) {
                throw "Expected numeric (float) argument for property \"" + word + "\", got \"" + arg + "\" instead";
            }

            // All checks passed, put this int into the data array
            uint8_t* convertedBytes = reinterpret_cast<uint8_t*>(&convertedFloat);
            data.push_back(convertedBytes[0]);
            data.push_back(convertedBytes[1]);
            data.push_back(convertedBytes[2]);
            data.push_back(convertedBytes[3]);
        }
        else if (argType == "double") {
            // Check: Is this argument really a number?
            double convertedFloat = 0;
            try {
                convertedFloat = std::stod(arg);
            } catch (...) {
                throw "Expected numeric (double) argument for property \"" + word + "\", got \"" + arg + "\" instead";
            }

            // All checks passed, put this int into the data array
            uint8_t* convertedBytes = reinterpret_cast<uint8_t*>(&convertedFloat);
            data.push_back(convertedBytes[0]);
            data.push_back(convertedBytes[1]);
            data.push_back(convertedBytes[2]);
            data.push_back(convertedBytes[3]);
            data.push_back(convertedBytes[4]);
            data.push_back(convertedBytes[5]);
            data.push_back(convertedBytes[6]);
            data.push_back(convertedBytes[7]);
        }
    }

    return data;
}

// Functions
std::vector<uint8_t> fileToMeat(NodeTypes nodeTypes, PropertiesMap properties, std::string path) {
    std::ifstream file(path);

    if (!file.is_open()) {
        // std::cout << "ERROR: File with path \"" << path << "\" does not exist.\n";
        return {};
    }

    std::string stringFile;
    std::string line;
    while (getline(file, line)) {
        stringFile += line + "\n";
    }

    file.close();

    return stringToMeat(nodeTypes, properties, stringFile);
}
std::vector<uint8_t> stringToMeat(NodeTypes nodeTypes, PropertiesMap properties, std::string file) {
    // Strings, one for the entire file, other for just one line
    std::stringstream filestream(file);
    std::string fileStr;
    std::string line;

    // Are we in a String?
    uint8_t inStringIndentationLv = 0;
    bool inStringFirstLine = false;
    bool queueEndString = false;
    bool inString = false;
    StringTypes strType = NONE;

    while (getline(filestream, line, '\n')) {
        // Replace tabs with spaces
        line = tabsToSpaces(line);

        if (inStringFirstLine) {
            inStringFirstLine = false;
        }
        if (queueEndString) {
            queueEndString = false;
            inString = false;
        }

        // Are we inside a String?
        bool newInString = endsInString(line, inString, strType);
        if (!inString && newInString) { 
            inString = true;
            inStringFirstLine = true;
            inStringIndentationLv = getIndentationLevel(line);

        } else if (!newInString) {
            queueEndString = true;
        }
        
        // Remove comment and trim string
        std::string cleanLine;

        if (inString && !inStringFirstLine && !queueEndString) {
            cleanLine = setIndentationLv(line, inStringIndentationLv+INDENTATION);
        }
        else if (inString && inStringFirstLine && !queueEndString) {
            cleanLine = line;
            ltrim(cleanLine);
        }
        else {
            // Find comment
            uint64_t comment = line.find("//");
            cleanLine = line.substr(0, comment);
            cleanLine = trim(cleanLine);
        }

        // Ignore line if it's empty
        if (!inString && cleanLine.empty()) {
            continue;
        }

        // append to fileStr
        // std::cout << cleanLine << "\n";
        fileStr += cleanLine + ((inString&&!queueEndString)?"\n":" ");
    }

    // Split and interpret
    // Binary data for the entire file
    std::vector<uint8_t> data;
    // History for the scope, by default it has 0 (the root)
    std::vector<uint32_t> scopeHistory = {0};
    // Object ID array
    std::vector<std::string> objects = {};

    // String stuff
    std::stringstream stream(fileStr);
    std::string instruction;

    inString = false;
    strType = NONE;
    std::string fullInstruction = "";

    while (getline(stream, instruction, ';')) {
        inString = endsInString(instruction, inString, strType);

        fullInstruction += instruction;

        // Holy moly! A string got cut off! that means that the string had a semicolon inside!
        if (inString) {
            // add the semicolon back to the string
            fullInstruction += ";";
            continue;
        }

        // Now finally read the full instruction(s)
        try {
            std::vector v = instructionToMeat(nodeTypes, properties, fullInstruction, scopeHistory, objects);
            data.insert(data.end(), v.begin(), v.end());
        } catch (std::string e) {
            return {};
        }
        fullInstruction = ""; // reset the fullInstruction
    }
    // What the fuck? A random string without semicolon?
    if (inString) {
        try {
            std::vector v = instructionToMeat(nodeTypes, properties, fullInstruction, scopeHistory, objects);
            data.insert(data.end(), v.begin(), v.end());
        } catch (std::string e) {
            return {};
        }
        fullInstruction = ""; // reset the fullInstruction
    }

    return data;
}

std::vector<uint8_t> instructionToMeat(NodeTypes nodeTypes, PropertiesMap properties, std::string line, std::vector<uint32_t>& scopeHierarchy, std::vector<std::string>& objects) {
    std::vector<uint8_t> data;

    std::stringstream stream(line);
    std::string word;

    // Are we inside a String?
    bool inString = false;
    bool inStringFirstWord = false;
    StringTypes strType = NONE;
    
    // Separate lines into words in order to get small instructions
    while (getline(stream, word, ' ')) {
        uint8_t wordStr = wordIsString(word, inString, strType);
        // std::cout << newInString << " ";

        if (word.empty()) {
            continue;
        }

        // Object creation
        if (contains(nodeTypes, word)) {
            std::string argument;
            getline(stream, argument, ' ');

            uint64_t typeID = indexOf(nodeTypes, word);

            data.push_back(1); // Creation of object
            data.push_back(typeID); // ID of the Node Type
            data.push_back(0); // String type (normal)
            data.push_back(0); // String flags (none)
            for (char c : argument) {
                data.push_back(c); // String data (name of object)
            }
            data.push_back(0); // NULL terminator

            // Register object
            objects.push_back(argument);
        }
        
        // Setting a property
        // FIXME: Possibly problematic code: No support for multiple values.
        // TODO: Implement fix: Make loop read until end of instruction
        else if (properties.find(word) != properties.end()) {
            std::string equals;
            getline(stream, equals, ' ');

            // TODO: Test, new check to see if "equals sign" is actually there
            if (equals != "=") {
                throw "Property with name \"" + word + "\" was mentioned but nothing is being assigned to it.";
            }

            std::string argument = "";
            bool continueReading = true;
            while (continueReading) {
                std::string l;
                if (!getline(stream, l, ' ')) {
                    continueReading = false;
                }

                argument += l + " ";

                uint8_t wordStr = wordIsString(l, inString, strType);
                if (!wordStr) {
                    continueReading = false;
                }
            }

            argument = trim(argument);

            std::vector<uint8_t> propertyBin = setProperty(properties, word, argument);
            data.insert(data.end(), propertyBin.begin(), propertyBin.end());
        }
        
        // Inclusion
        else if (word == "#include") {
            std::string argument;
            getline(stream, argument, ' ');
            data.push_back(3); // Inclusion
            data.push_back(0); // Normal String
            data.push_back(0); // No flags
            for (char c : argument) {
                data.push_back(c); // Name of file to include
            }
            data.push_back(0); // NULL terminator
        }

        // Register
        else if (word == "#register") {
            std::string argument;
            getline(stream, argument, ' ');
            data.push_back(4); // Registration
            data.push_back(0); // Normal String
            data.push_back(0); // No flags
            for (char c : argument) {
                data.push_back(c); // Name of file to register
            }

            getline(stream, argument, ' ');
            data.push_back(0); // Normal String
            data.push_back(0); // No flags
            for (char c : argument) {
                data.push_back(c); // Path of file to register
            }

            data.push_back(0); // NULL terminator
        }

        // TODO: Template

        // Scope Change
        else if (word == "{") {
            uint32_t newScope = objects.size();
            uint8_t* newScopeBytes = reinterpret_cast<uint8_t*>(&newScope);
            scopeHierarchy.push_back(newScope);

            data.push_back(0);
            for (uint8_t i = 0; i < 4; i++) {
                data.push_back(newScopeBytes[i]);
            }
        }
        else if (word == "}") {
            if (scopeHierarchy.size() < 2) {
                throw "Tried to close root scope";
            }

            scopeHierarchy.pop_back();
            uint32_t newScope = scopeHierarchy.at(scopeHierarchy.size()-1);
            uint8_t* newScopeBytes = reinterpret_cast<uint8_t*>(&newScope);

            data.push_back(0);
            for (uint8_t i = 0; i < 4; i++) {
                data.push_back(newScopeBytes[i]);
            }
        }

        // Other (custom, should be of String Type)
        else {
            std::string secondWord;
            getline(stream, secondWord, ' ');

            // Are we setting a property?
            if (secondWord == "=") {
                std::string argument = "";
                bool continueReading = true;
                while (continueReading) {
                    std::string l;
                    if (!getline(stream, l, ' ')) {
                        continueReading = false;
                    }

                    argument += l + " ";

                    uint8_t wordStr = wordIsString(l, inString, strType);
                    if (!wordStr) {
                        continueReading = false;
                    }
                }

                argument = trim(argument);

                if (!isNormalString(argument)) {
                    throw argument + " should be a String, as it is a custom property";
                }

                std::vector<uint8_t> stringData = getStringData(argument);

                data.push_back(2); // Custom property
                data.push_back(0); // Normal String
                data.push_back(0); // No flags
                for (char c : word) {
                    data.push_back(c); // Name of custom property
                }
                data.push_back(0); // NULL terminator
                data.insert(data.end(), stringData.begin(), stringData.end());
            }
            else {
                throw word + " is unknown";
            }
        }
    }

    return data;
}
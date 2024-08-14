#pragma once
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

#include <cstdint>
#include <vector>
#include <string>
#include <map>

typedef std::vector<std::string> NodeTypes;
typedef std::map<std::string, std::string> PropertiesMap;

std::vector<uint8_t> stringToMeat(NodeTypes nodeTypes, PropertiesMap properties, std::string file);
std::vector<uint8_t> fileToMeat(NodeTypes nodeTypes, PropertiesMap properties, std::string path);
std::vector<uint8_t> instructionToMeat(NodeTypes nodeTypes, PropertiesMap properties, std::string line, std::vector<uint32_t>& scopeHierarchy, std::vector<std::string>& objects);
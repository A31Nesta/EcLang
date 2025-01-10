#include "classes/class.hpp"
// std
#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace eclang {
    Class::Class(std::string name) {
        this->name = name;
    }
        
    void Class::registerAttribute(Attribute attribute) {
        attributes.push_back(attribute);
        attributesStr.push_back(attribute.getName());
    }
    void Class::registerAttribute(std::string name, type::Type type) {
        attributes.push_back(Attribute(name, type));
        attributesStr.push_back(name);
    }

    std::string Class::getName() {
        return name;
    }
    std::vector<std::string> Class::getAttributes() {
        return attributesStr;
    }
    bool Class::attributeExists(uint32_t id) {
        return id < attributesStr.size();
    }
    bool Class::attributeExists(std::string attribute) {
        return std::find(attributesStr.begin(), attributesStr.end(), attribute) != attributesStr.end();
    }
    std::string Class::getAttributeName(uint32_t id) {
        if (!attributeExists(id)) {
            throw std::runtime_error(
                "ECLANG_ERROR: Couldn't get attribute name from ID. ID ("+std::to_string(id)+") was greater than "
                "the total amount of attributes ("+std::to_string(attributesStr.size())+")"
            );
        }
        return attributesStr.at(id);
    }
    uint32_t Class::getAttributeID(std::string attribute) {
        auto iterator = std::find(attributesStr.begin(), attributesStr.end(), attribute);
        // We don't use attributeExists here to avoid using std::find() again
        if (iterator == attributesStr.end()) {
            throw std::runtime_error(
                "ECLANG_ERROR: Couldn't get attribute ID from name. Attribute with name \""+attribute+"\" is not registered"
            );
        }
        return iterator - attributesStr.begin();
    }
    type::Type Class::getAttributeType(std::string attribute) {
        uint32_t attributeID;
        try {
            attributeID = getAttributeID(attribute);
        } catch (...) {
            throw std::runtime_error("ECLANG_ERROR: Couldn't get type of attribute \""+attribute+"\". Attribute is not registered");
        }
        return attributes.at(attributeID).getType();
    }
    type::Type Class::getAttributeType(uint32_t id) {
        if (!attributeExists(id)) {
            throw std::runtime_error("ECLANG_ERROR: Couldn't get type of attribute with ID \""+std::to_string(id)+"\". Invalid ID");
        }
        return attributes.at(id).getType();
    }
}
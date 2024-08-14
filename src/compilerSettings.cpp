#include <compilerSettings.hpp>

// EXAMPLE CONFIGURATION FOR MEATGRINDER
// This configuration is the one used for AUII.
// Do not compile this file, the program that uses
// this library should implement its own vector and map
namespace compilerSettings {
    std::vector<std::string> nodeTypes = {
        // AUII Types
        "Panel",
        "Text",
        "Image",
        "Button",
        "TextInput",
        "Password",
        "Checkbox",
        "Radio",
        "Color",
        "Select",
        "Sprite",
        "Framebuffer",
        "Navbar",
        "Footer",

        // HTML Types
        "Input",
        "Form"
    };

    // Custom properties, scope changes, creation among others are handled by Meatgrinder.
    // Here is where you add the possible properties of a node and the type of data needed to set them.
    std::map<std::string, std::string> properties = {
        {"style", "String"},
        {"text-style", "String"},
        {"css", "String,String"},
        {"text", "StringAny"},
        {"image", "String"},
        {"description", "String"},
        {"aspect-ratio", "uint8_t,uint8_t"},
        {"grid-resolution", "uint16_t,uint16_t"}
    };
}
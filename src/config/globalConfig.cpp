#include "globalConfig.hpp"
#include <vector>

namespace eclang::config {
    // OBJECTS
    // -------
    std::vector<Language> languages {
        // EcLanguage Descriptor
        // ---------------------
        {
            "eclang-descriptor", "eld", "eldc", {'E', 'C', 'L', 'D', 0x31}, {
                // CLASSES
                {"Language", {
                    // 

                }}
            }
        },
        // AUII
        // ----
        {
            "AUII", "auii", "auic", {'A', 'U', 'I', 'I', 0x31}
        }
    };

    // "PRIVATE" FUNCTIONS
    // -------------------

    // HEADER DEFINITIONS
    // ------------------

    /**
        Returns all registered languages.
        By default, the configuration comes with 3
        languages: AUII, NEA and LangDescriptor
    */
    std::vector<Language> getLanguages() {
        return languages;
    }
}
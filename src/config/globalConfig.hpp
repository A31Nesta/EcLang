#pragma once

#include "classes/language.hpp"
namespace eclang::config {
    /**
        Returns all registered languages.
        By default, the configuration comes with 3
        languages: AUII, NEA and LangDescriptor
    */
    extern std::vector<Language> getLanguages();
}
#pragma once

#include "classes/language.hpp"
namespace eclang::config {
    /**
        Returns all registered languages.
    */
    extern std::vector<Language> getLanguages();
    /**
        Registers a language.
        Allows you to use this language for
        compilation/decompilation/interpretation
    */
    extern void registerLanguage(Language lang);
}
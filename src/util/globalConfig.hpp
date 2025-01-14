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

    /**
        Adds an alias to a file path.
        Whether or not the path is valid is not checked.
    */
    extern void filepathRegister(std::string alias, std::string path);
    /**
        Is this string an alias?
    */
    extern bool filepathIsAlias(std::string alias);
    /**
        Returns the file path for the given alias
    */
    extern std::string filepathGetFor(std::string alias);
}
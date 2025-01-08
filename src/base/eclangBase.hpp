#pragma once

// eclang
#include "languageDescriptor.hpp"
// std
#include <cstdint>
#include <string>
#include <vector>

namespace eclang {
    /**
        A single EcLang file. It contains a compiled file. If a source
        file is passed to the constructor, it will get automatically compiled.

        All the magic happens here but the actual interface is not here.
        For the EcLang class available to the user of this library check
        the EcLang class.
    */
    class EcLangBase {
    public:
        /**
            Constructs an EcLang object from the path to the file.
            The Language (AUII, NEA, Other...) will be automatically
            detected.

            For languages other than AUII and NEA, a Language Descriptor
            for that language must be created and registered before loading the
            file
        */
        EcLangBase(std::string filepath);
        /**
            Constructs an EcLang object from raw data that corresponds to the
            contents of a source or compiled EcLang file.
            The Language (AUII, NEA, Other...) will be automatically
            detected.

            For languages other than AUII and NEA, a Language Descriptor
            for that language must be created and registered before loading the
            file
        */
        EcLangBase(void* data, size_t size);
        
        /**
            Saves the compiled/decompiled file.
            If a source file was loaded in the constructor we save a compiled file,
            if a compiled file was loaded instead we save a decompiled source file.

            Not specifying a filename or leaving it empty will save a file with the
            same name as the loaded file but with the extension changed.

            e.g. `myFile.src` --> `myFile.bin` (compilation) `myFile.bin` --> `myFile.src` (decompilation)
        */
        void saveToFile(std::string fileWithoutExtension = "");
        /**
            Saves the compiled EcLang file (compiled at initialization or simply loaded)

            Not specifying a filename or leaving it empty will save a file with the
            same name as the loaded file but with the extension changed.

            e.g. `myFile.src` --> `myFile.bin` (compilation) `myFile.bin` --> `myFile.src` (decompilation)
        */
        void saveToFileCompiled(std::string fileWithoutExtension = "");
        /**
            Saves the source EcLang file.
            If the file specified in the initialization was a compiled file,
            this method will decompile the file. This decompiled file will not have
            any comments, as they're lost during compilation.

            Not specifying a filename or leaving it empty will save a file with the
            same name as the loaded file but with the extension changed.

            e.g. `myFile.src` --> `myFile.bin` (compilation) `myFile.bin` --> `myFile.src` (decompilation)
        */
        void saveToFileSource(std::string fileWithoutExtension = "");

        /**
            TODO: Get data from the tree
        */

    private:
        /**
            Takes the source file (string) as input and returns a vector of uint8_t
            containing the compiled file.
            A language descriptor is required in order to interpret the file and compile it.

            This method is called during initialization if the file is a source file.
        */
        std::vector<uint8_t> compile(std::string source, language::LanguageDescriptor* language);
        /**
            Takes the compiled file (binary) as input and returns a string containing the
            decompiled source code.
            A language descriptor is required in order to interpret the compiled file.

            This method is called during initialization if the file is a compiled file.
            If this method fails to execute, the program can still continue to run. Errors
            can happen if the language descriptor used for compilation is not the same as the
            one used for decompilation (for example due to updates to the language)
        */
        std::string decompile(std::vector<uint8_t> compiled, language::LanguageDescriptor* language);

        // The name of this file without the extension
        std::string name;

        // Language Descriptor currently being used.
        // Obtained by the configuration once the EcLang object
        // figures out the language being used in the loaded file
        language::LanguageDescriptor* languageDescriptor;

        // Source code for the current file.
        // This value can be obtained from the user or from decompilation
        std::string source;
        // Compiled code for the current file.
        // This value can be obtained from the user or from compilation
        std::vector<uint8_t> compiled;
    };
}
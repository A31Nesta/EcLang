#pragma once
/**
    EcLang: A compiled C-like Markup Language
    -----------------------------------------
    By A31Nesta

    A simple library to use the EcLang
    markup language in C++.

    EcLang is part of the EcLibs and Occlusion
    projects: it natively supports compiling and
    reading AUII and NEA files (2D and 3D scenes)
*/

// INCLUDES
// --------
#include <cstdint>
#include <string>
#include <vector>

// CLASSES
// -------

namespace eclang {
    namespace language {
        /**
            Information that allows an EcLang object to understand
            how a custom EcLang file (compiled or not) should be interpreted
            and/or compiled.

            Manual creation of Language Descriptors is only necessary for
            files that are not AUII or NEA as these types have predefined
            Language Descriptors
        */
        // TODO: Implement Language Descriptors
        class EcLangDescriptor {
        public:

        private:

        };
    }

    /**
        A single EcLang file. It contains a compiled file. If a source
        file is passed to the constructor, it will get automatically compiled.
    */
    class EcLang {
    public:
        /**
            Constructs an EcLang object from the path to the file.
            The Language (AUII, NEA, Other...) will be automatically
            detected.

            For languages other than AUII and NEA, a Language Descriptor
            for that language must be created and registered before loading the
            file
        */
        EcLang(std::string filepath);
        /**
            Constructs an EcLang object from raw data that corresponds to the
            contents of a source or compiled EcLang file.
            The Language (AUII, NEA, Other...) will be automatically
            detected.

            For languages other than AUII and NEA, a Language Descriptor
            for that language must be created and registered before loading the
            file
        */
        EcLang(void* data, size_t size);
        
        /**
            Saves the compiled EcLang file (compiled at initialization or simply loaded)
        */
        void saveToFileCompiled(std::string fileWithoutExtension);
        /**
            Saves the source EcLang file.
            If the file specified in the initialization was a compiled file,
            this method will decompile the file. This decompiled file will not have
            any comments, as they're lost during compilation.
        */
        void saveToFileSource(std::string fileWithoutExtension);

        /**
            TODO: Get data from the tree
        */

    private:
        /**
            Takes as input the source file (string) and returns a vector of uint8_t
            containing the compiled file.

            A language descriptor is required in order to interpret the file and compile it.

            This method is called during initialization if the file is a source file
        */
        std::vector<uint8_t> compile(std::string source, language::EcLangDescriptor* language);


        // Language Descriptor currently being used.
        // Obtained by the configuration once the EcLang object
        // figures out the language being used in the loaded file
        language::EcLangDescriptor* languageDescriptor;
    };
}
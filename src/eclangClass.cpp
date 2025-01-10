#include "classes/eclangClass.hpp"
#include "classes/language.hpp"
#include "config/globalConfig.hpp"

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <ios>
#include <stdexcept>
#include <vector>

namespace eclang {
    // PUBLIC
    // ------

    /**
        Constructs an EcLang object from the path to the file.
        The Language (AUII, NEA, Other...) will be automatically
        detected.
    */
    EcLang::EcLang(std::string filepath) {
        // Read file as binary
        std::ifstream file(filepath, std::ios::binary);
        if (!file) {
            throw std::runtime_error("ECLANG_ERROR: Couldn't load file \""+filepath+"\". No such fie or directory");
        }

        // Find file size
        file.seekg(0, std::ios::end);
        size_t length = file.tellg();
        file.seekg(0, std::ios::beg);

        // Put contents into vector
        char* buffer = new char[length];
        if (!file.read(buffer, length)) {
            delete[] buffer;
            throw std::runtime_error("ECLANG_ERROR: Couldn't copy contents of file into buffer");
        }

        // Close file
        file.close();

        // Initialize EcLang with the raw file data
        initializeEcLang(buffer, length);
    }
    /**
        Constructs an EcLang object from raw data that corresponds to the
        contents of a source or compiled EcLang file.
        The Language (AUII, NEA, Other...) will be automatically
        detected.
    */
    EcLang::EcLang(void* data, size_t size) {
        initializeEcLang(data, size);
    }
    
    /**
        Saves the compiled/decompiled file.
        If a source file was loaded in the constructor we save a compiled file,
        if a compiled file was loaded instead we save a decompiled source file.
    */
    void EcLang::saveToFile(std::string fileWithoutExtension) {
        // If if's a source file we save the compiled file.
        // If not, we save a decompiled source file
        if (fileWasSource) {
            saveToFileCompiled(fileWithoutExtension);
        } else {
            saveToFileSource(fileWithoutExtension);
        }
    }
    /**
        Saves the compiled EcLang file (compiled at initialization or simply loaded)
    */
    void EcLang::saveToFileCompiled(std::string fileWithoutExtension) {
        if (fileWithoutExtension == "") fileWithoutExtension = name;
        // TODO: Implement save to file compiled
    }
    /**
        Saves the source EcLang file.
        If the file specified in the initialization was a compiled file,
        this method will decompile the file. This decompiled file will not have
        any comments, as they're lost during compilation.
    */
    void EcLang::saveToFileSource(std::string fileWithoutExtension) {
        if (fileWithoutExtension == "") fileWithoutExtension = name;
        // TODO: Implement save to file source
    }

    // PRIVATE
    // -------

    /**
        For both constructors, a void* of data and a size are obtained
        and passed to this function. This function figures out the type of
        the file (language, binary/source...) and compiles the file if it's
        a source file.
    */
    void initializeEcLang(void* dataRaw, size_t size) {
        // Pass void* to uint8_t to make working with raw data easier
        uint8_t* data = (uint8_t*)dataRaw;

        // Get all languages
        std::vector<Language> languages = config::getLanguages();

        // Identify first bytes
        // --------------------
        // If they're equal to any Language's identifier bytes, the file
        // is identified as Compiled and a reference to the Language object will be saved.
        // If not, we check if the first bytes of the file are `#language`; in that case
        // the file will be treated as text (Source) and we will try to find the language
        // specified in our configuration
        
    }
}
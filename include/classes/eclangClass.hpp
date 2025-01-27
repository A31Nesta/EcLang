#pragma once

// eclang
#include "classes/object.hpp"
#include "language.hpp"
// std
#include <cstddef>
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
    class EcLang {
    public:
        /**
            Constructs an EcLang object from the path to the file.
            The Language (AUII, NEA, Other...) will be automatically
            detected.
        */
        EcLang(std::string filepath, uint8_t fileID = 0);
        /**
            Constructs an EcLang object from raw data that corresponds to the
            contents of a source or compiled EcLang file.
            The Language (AUII, NEA, Other...) will be automatically
            detected.

            The name must not contain an extension and will be used during
            file export if no name is specified
        */
        EcLang(std::string name, void* data, size_t size, uint8_t fileID = 0);
        /**
            Guess what it does
        */
        ~EcLang();
        
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
            Returns the Object objects from the current file as a vector.
            The Objects allow us to access all the data with a simple interface
        */
        std::vector<Object*> getAllObjects();
        /**
            Returns the Object objects with the class name specified
            from the current file as a vector.
        */
        std::vector<Object*> getObjectsByClass(std::string className);
        /**
            Returns the Object object with the name specified.
            A specific object in the hierarchy can be specified with paths (node1/node2/node3).
            The pointer returned may be nullptr.
        */
        Object* getObject(std::string name);

    private:
        /**
            For both constructors, a void* of data and a size are obtained
            and passed to this function. This function figures out the type of
            the file (language, binary/source...) and compiles the file if it's
            a source file.

            Throws a runtime error if the language can't be determined.
        */
        void initializeEcLang(void* dataRaw, size_t size);
        /**
            Constructs all the Object objects by parsing a source file.
        */
        void constructFromSource(std::string source);
        /**
            Constructs all the Object objects by reading a binary file.
        */
        void constructFromBinary(std::vector<uint8_t> compiled);

        /**
            returns a vector of uint8_t containing the compiled file.

            This method is called when calling saveToFileCompiled() if the file is a source file.
        */
        std::vector<uint8_t> compile();
        /**
            returns a string containing the decompiled source code.

            This method is called when calling saveToFileSource().
            If this method fails to execute, an exception will be thrown.
            Errors can happen if the Language used for compilation is not the same as the
            one used for decompilation (for example due to updates to the language)
        */
        std::string decompile();

        // FOR USE IN ANOTHER ECLANG
        /**
            Returns the Object objects from the current file as a vector.
            This function marks this object as "included", which effectively
            transfers the ownership of the objects to the object calling this
            function (the parent node during inclusion)
        */
        std::vector<Object*> _getAllObjectsAsInclude();
        /**
            Returns the Template Node Path.
            The template node path will be empty if the file doesn't contain
            a #template tag.
        */
        std::vector<Object*> _getTemplateNodePath();

        // COMPILATION / DECOMPILATION HELPER FUNCTIONS

        /**
            Takes a pointer to an instance of Object and returns its compiled code.
            This function is recursive and will produce the compiled code for setting
            the attributes, setting the template tag and creating children objects
        */
        std::vector<uint8_t> compileObjects(std::vector<Object*> objects);
        /**
            Takes a pointer to an instance of Object and returns the CREATE instruction
            for it.
        */
        std::vector<uint8_t> compileObjectCreation(Object* object);

        /**
            Takes a pointer to an instance of Object and returns its decompiled code.
            This function is recursive and will produce the decompiled code for setting
            the attributes, setting the template tag and creating children objects
        */
        std::string decompileObjects(std::vector<Object*> objects, uint8_t tabs = 0);

        /**
            Converts a number to an array of bytes
        */
        std::vector<uint8_t> numberToBytes(int8_t number);
        std::vector<uint8_t> numberToBytes(int16_t number);
        std::vector<uint8_t> numberToBytes(int32_t number);
        std::vector<uint8_t> numberToBytes(int64_t number);
        std::vector<uint8_t> numberToBytes(uint8_t number);
        std::vector<uint8_t> numberToBytes(uint16_t number);
        std::vector<uint8_t> numberToBytes(uint32_t number);
        std::vector<uint8_t> numberToBytes(uint64_t number);
        std::vector<uint8_t> numberToBytes(float number);
        std::vector<uint8_t> numberToBytes(double number);

        // Names of files relevant to this file
        // When a file is included or used as template dynamically,
        // its name gets registered here.
        // Every object has an associated file in the form
        // of a numeric ID.
        // 
        // This array is only used when the file is opened directly
        // by the user.
        // 
        // The index 0 contains the name of this file
        std::vector<std::string> includedFilenames;
        // Determined during initialization. It indicates whether or not the file
        // was a source file or a compiled file.
        bool fileWasSource;

        // Language currently being used.
        // Obtained by the configuration once the EcLang object
        // figures out the language being used in the loaded file
        Language* language;

        // Source code for the current file.
        // This value can be obtained from the user or from decompilation
        std::string source;
        // Compiled code for the current file.
        // This value can be obtained from the user or from compilation
        std::vector<uint8_t> compiled;

        // All the objects obtained from parsing/reading source/binary file that
        // are in the root (and not inside another node (object))
        // contain actual data that we can read with `object.get<DataType>Of(attribute);`
        std::vector<Object*> objects;
        // While reading, objects are created and stored in the `objects` array.
        // When we enter scope, we add the last object in the `objects` array here.
        // When we exit scope, we remove the last object in this array.
        // WE DO NOT DELETE ANYTHING IN THIS ARRAY. THESE ARE REFERENCES
        std::vector<Object*> scope;
        // Has a (possible) parent EcLang obtained this object's Objects?
        // If this is the case, even if this EcLang instance is deleted we
        // don't want to delete our Objects
        bool isIncluded = false;

        // Inclusion / Templating
        // ----------------------

        // Template Node Path.
        // The stack (scope) that corresponds to the template node.
        // Empty if there are no template nodes
        std::vector<Object*> templateNode;
        // The template node of the template imported
        // Empty if no template was included
        std::vector<Object*> externalTemplateNode;

        // The current file. When a file is dynamically included,
        // the current file index goes up and the name is registered
        // into the array. This value is copied into every loaded object.
        // 
        // IMPORTANT! If this value is set to non-zero in the constructor, all inclusions
        // will be done statically. This is because we only care about compiling the current
        // file; dynamically included files are compiled into an "include" instruction so we
        // shouldn't need to care about files that may change.
        const uint8_t currentFile;
    };
}
#include "eclang.hpp"
#include <iostream>

int main() {
    std::cout << "Registering files...\n";
    eclang::EcLang registration("register.elt");
    std::cout << "Done!\n\n";

    // Read Source File
    std::cout << "Reading Source File:\n";
    eclang::EcLang testEcLang1("testFile.elt");
    std::cout << "Done!\n\n";

    // Compile
    std::cout << "Compiling Source File:\n";
    testEcLang1.saveToFileCompiled("compiled");
    std::cout << "Compiled!!!\n\n";

    // Read Compiled File
    std::cout << "Reading Compiled File:\n";
    eclang::EcLang testEcLang2("compiled.eltc");
    std::cout << "Done!\n\n";

    // Decompile
    std::cout << "Decompiling Compiled File:\n";
    testEcLang1.saveToFileSource("decompiled");
    std::cout << "Decompiled!!!\n\n";

    // // Try to get allTestData/correctData/stringTest1/string
    // eclang::Object* stringTest1 = testEcLang1.getObject("allTestData/correctData/stringTest1");
    // if (stringTest1 == nullptr) { return 0; }
    // uint8_t stringAttribute = stringTest1->getIDOf("string");
    // std::cout << "Value of allTestData/correctData/stringTest1/string: " << stringTest1->getStringOf(stringAttribute) << "\n";
}
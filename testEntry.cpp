#include "eclang.hpp"
#include <iostream>

int main() {
    std::cout << "Registering files...\n";
    eclang::EcLang registration("register.elt");
    std::cout << "Done!\n\n";

    std::cout << "Testing Source File:\n";
    eclang::EcLang testEcLang1("testFile.elt");
    // std::cout << "Testing Binary File:\n";
    // eclang::EcLang testEcLang2("testFile.eltc");

    // Compile
    std::cout << "Compiling Source File:\n";
    testEcLang1.saveToFileCompiled();
    std::cout << "Compiled!!!\n\n";

    // Try to get allTestData/correctData/stringTest1/string
    eclang::Object* stringTest1 = testEcLang1.getObject("allTestData/correctData/stringTest1");
    if (stringTest1 == nullptr) { return 0; }
    uint8_t stringAttribute = stringTest1->getIDOf("string");
    std::cout << "Value of allTestData/correctData/stringTest1/string: " << stringTest1->getStringOf(stringAttribute) << "\n";
}
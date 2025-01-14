#include "eclang.hpp"
#include <iostream>

int main() {
    std::cout << "Registering files...\n";
    eclang::EcLang registration("register.elt");
    std::cout << "Done!\n\n";

    std::cout << "Testing Source File:\n";
    eclang::EcLang testEcLang1("testFile.elt");
    std::cout << "Testing Binary File:\n";
    eclang::EcLang testEcLang2("testFile.eltc");
}
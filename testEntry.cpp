#include "eclang.hpp"
#include <iostream>

int main() {
    std::cout << "Testing Source File:\n";
    eclang::EcLang testEcLang1("testFile.eld");
    std::cout << "Testing Binary File:\n";
    eclang::EcLang testEcLang2("testFile.eldc");
}
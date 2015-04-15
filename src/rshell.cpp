// RSHELL.CPP
// Main source code for rshell

#include <iostream>
#include <string>

#include "rshell.h"


void init() {}

int run() {
    prompt();

    return 0;
}

std::string prompt() {
    std::cout << "$ ";
    std::string input;
    std::cin >> input;
    return input;
}

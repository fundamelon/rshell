#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <iostream>

#include "rshell.h"

//#define DEBUG


int main(int argc, char** argv) {

    // debug
#ifdef DEBUG
    for(int i = 0; i < argc; i++)
        std::cout << argv[i] << std::endl;
#endif

    prompt();

    if(argc > 1) {
        execvp(argv[1], &argv[1]);
        perror(argv[1]);
    }

    return 0;
}

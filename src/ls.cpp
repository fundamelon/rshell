// LS.CPP
// Main source code for rshell ls

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

#include <iostream>
#include <vector>

#include "ls.h"


int LS_MODE = 0;


int main(int argc, char** argv) {

    std::string cmdflags = "";
    std::vector<const char*> fileflags;

    // parse input flags
    for(int i = 1; i < argc; i++)
        // is a flag
        if(argv[i][0] == '-')
            cmdflags.append(&argv[i][1]);
        // is a file/directory
        else
            fileflags.push_back(argv[i]);

    // modify program behavior from flags
    for(char c : cmdflags)
        switch(c) {
            case 'a':
            LS_MODE |= LS_MODE_SHOWALL;
            break;
            
            case 'R':
            LS_MODE |= LS_MODE_RECURSIVE;
            break;

            case 'l':
            LS_MODE |= LS_MODE_LIST;
            break;

            default:
            std::cout << "Invalid option: \'" << c 
                      << "\'\nSupported flags: -a, -R, -l" << std::endl;
            exit(1);
            break;
        }


    auto files = scandir(".");

    for(auto filename : files)
        std::cout << filename << " ";
        
    std::cout << std::endl;

    return 0;
}


std::vector<const char*> scandir(const char* path) {
   
	DIR* dirp;
    if( (dirp = opendir(path)) == NULL) {
        perror("opendir");
        exit(1);
    }

    std::vector<const char*> files;

    struct dirent *entry;
    errno = 0;
    while( (entry = readdir(dirp)) != NULL)
        files.push_back(entry->d_name);
    
    if(errno != 0) {
        perror("readdir");
        exit(1);
    }

    if(closedir(dirp) == -1) {
        perror("closedir");
        exit(1);
    }

    return files;
}

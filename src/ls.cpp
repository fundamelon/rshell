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


int main(int argc, char** argv) {
    
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

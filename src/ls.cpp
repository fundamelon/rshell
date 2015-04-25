// LS.CPP
// Main source code for rshell ls

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>

#include <iostream>
#include <vector>
#include <algorithm>

#include "ls.h"

// quick filename comparison function (case and . insensitive)
bool namecmp(std::string i, std::string j) {
    std::transform(i.begin(), i.end(), i.begin(), ::tolower);
    std::transform(j.begin(), j.end(), j.begin(), ::tolower);
    if(i.size() > 0 && i[0] == '.') i.erase(0, 1);
    if(j.size() > 0 && j[0] == '.') j.erase(0, 1);
    return (i < j);
}

int main(int argc, char** argv) {
    
    auto files = scandir(".");
    std::sort(files.begin(), files.end(), namecmp);

    for(auto filename : files)
        std::cout << filename << "  ";
        
    std::cout << std::endl;

    return 0;
}


std::vector<std::string> scandir(const char* path) {
   
	DIR* dirp;
    if( (dirp = opendir(path)) == NULL) {
        perror("opendir");
        exit(1);
    }

    std::vector<std::string> files;

    struct dirent* entry;
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

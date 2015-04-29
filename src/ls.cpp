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


int LS_MODE = 0;


int main(int argc, char** argv) {

    std::string cmdflags = "";
    std::vector<const char*> cmdfiles;

    // parse input flags
    for(int i = 1; i < argc; i++)
        if(argv[i][0] == '-')
            cmdflags.append(&argv[i][1]);  // is a flag
        else
            cmdfiles.push_back(argv[i]);  // is a file

    // flag if multiple files are being requested
    if(cmdfiles.size() > 1) 
        LS_MODE |= LS_MODE_MANYFILES;

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

    if(cmdfiles.empty())
        readloc(".");
    else for(auto path : cmdfiles)
        readloc(path);

    return 0;
}


// quick filename comparison function (. insensitive)
bool namecmp(std::string i, std::string j) {
    std::transform(i.begin(), i.end(), i.begin(), ::tolower);
    std::transform(j.begin(), j.end(), j.begin(), ::tolower);
    if(i.size() > 1 && i[0] == '.') i.erase(0, 1);
    if(j.size() > 1 && j[0] == '.') j.erase(0, 1);
    return (i < j);
}


// take a filepath and print out its contents
void readloc(const char* path) {

    struct stat stat_buf;
    if(stat(path, &stat_buf) == -1) { perror("stat"); exit(1); }

    if(S_ISDIR(stat_buf.st_mode)) {
        if(LS_MODE & LS_MODE_MANYFILES) std::cout << path << ":" << std::endl;
    } else {
        std::cout << path << std::endl;
        return;
    }

    // get and sort files within directory
    auto files = scandir(path);
    std::sort(files.begin(), files.end(), namecmp);

    for(auto f : files)
        if(LS_MODE & LS_MODE_SHOWALL || f[0] != '.')
            std::cout << f << "  ";
        else continue;

    std::cout << std::endl;
}


// scan a directory for all contained files
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

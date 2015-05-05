// MV.CPP
//

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>

#include <iostream>
#include <vector>
#include <string>
#include <cstring>


#include "rm.h"


bool RM_DIRS = false;



int main(int argc, char** argv) {
    
    if(argc != 3) {
        std::cout << "Wrong number of inputs\nSyntax is: mv <source> <dest>\n";
        return 1;
    }

    const char* src = argv[1];
    const char* dst = argv[2];

    if(strcmp(src, dst) == 0) {
        std::cout << "error: source and destination " << src << " are the same" << std::endl;
        return 1;
    }

    struct stat src_stat;
    struct stat dst_stat;
   
    // check source path (must exist)
    if(stat(src, &src_stat) == -1) { perror("stat: source path"); return 1; }
    if(!S_ISREG(src_stat.st_mode)) {
        std::cout << "error: source path must point to a file" << std::endl;
        return 1;
    }

    // check dest path (must not be a file)
    int stat_err = stat(dst, &dst_stat);
    if(stat_err == 0 && !S_ISDIR(dst_stat.st_mode)) {
        std::cout << "error: destination file already exists" << std::endl;
        return 1;
    }


    // create hard link
    if(link(src, dst) != 0) { perror("link: error moving to destination"); return 1; }

    // delete old link
    if(unlink(src) != 0) { perror("unlink: error removing old link"); return 1; }
    
    return 0;
}

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
#include <string.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>

#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <utility>

#include "ls.h"



int LS_MODE = 0;



int main(int argc, char** argv) {

    std::string cmdflags = "";
    std::vector<const char*> cmdfiles;


    // parse input strings 
    for(int i = 1; i < argc; i++)
        if(argv[i][0] == '-')
            cmdflags.append(&argv[i][1]);  // is a flag
        else
            cmdfiles.push_back(argv[i]);  // is a file

    // flag if multiple files are being requested
    if(cmdfiles.size() > 1) 
        LS_MODE |= LS_MODE_MANYFILES;

    // flag that first entry will reset
    LS_MODE |= LS_MODE_FIRSTENTRY;

    // sort cmdfiles alphabetically
    std::sort(cmdfiles.begin(), cmdfiles.end(), namecmp);


    // set flags accordingly
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
        }


    // begin program on requested files
    if(cmdfiles.empty())
        readloc(".");
    else for(auto path : cmdfiles)
        readloc(path);

    return 0;
}



struct classnamecmp {
    bool operator() (const std::string &lhs, const std::string &rhs) const
        { return namecmp(lhs, rhs); }
};


// quick filename comparison function (leading '.' and case insensitive)
bool namecmp(std::string i, std::string j) {
    for(char &c : i) c = tolower(c);
    for(char &c : j) c = tolower(c);
    if(i.size() > 0 && i[0] == '.') i.erase(0, 1);
    if(j.size() > 0 && j[0] == '.') j.erase(0, 1);
    return (i < j);
}



// take a filepath and print out its contents
void readloc(const char* path) {

    // first-run formatting
    if(!(LS_MODE & LS_MODE_FIRSTENTRY))
        std::cout << std::endl;
    else
        LS_MODE &= ~LS_MODE_FIRSTENTRY;

    // output nice directory title 
    if(     LS_MODE & LS_MODE_MANYFILES ||
            LS_MODE & LS_MODE_RECURSIVE     ) 
        std::cout << path << ":" << std::endl;


    // get and sort files within directory
    std::vector<std::string> files;
    int scan_status = scandir(path, files);
    if(scan_status == -1) return;
    else if(scan_status == 1) {
        //std::cout << path << std::endl;
        printinfo(path);
        return;
    }
    std::sort(files.begin(), files.end(), namecmp);


    // create container of same type for dirs
    decltype(files) dirs;

    // first iteration through filename list
    for(auto &&f : files) {
        
        std::string fullpath = std::string(path) + "/" + f;

        //stat the file
        struct stat stat_buf;
        if(stat(fullpath.c_str(), &stat_buf) == -1) { perror("stat"); exit(1); }
        
        // skip hidden files if not in showall mode
        if(f == "." || f == ".." || (!(LS_MODE & LS_MODE_SHOWALL) && f[0] == '.'))
            continue; 
        
        // handle as dir or print filename
        if(S_ISDIR(stat_buf.st_mode) && (LS_MODE & LS_MODE_RECURSIVE)) {
            dirs.push_back(f);
        }

        // replace with full path for printing
        f = fullpath;
    }

    // send set of filepaths to print
    printinfo(files);

    // non-list formatting
    if(!(LS_MODE & LS_MODE_LIST)) std::cout << std::endl;


    // recursive step
    if(LS_MODE & LS_MODE_RECURSIVE) 
        for(auto d : dirs)
            readloc((std::string(path) + "/" + d).c_str());
}



int printinfo(const char* path) {
    return printinfo({ std::string(path) });
}


            
int printinfo(std::vector<std::string> paths) {

    std::map<std::string, struct stat, classnamecmp> filestats;

    nlink_t max_nlink = 0;
    off_t max_size = 0;
    blkcnt_t total_blocks = 0;
   
    // first iteration - collect info, construct map
    for(auto p : paths) {

        struct stat stat_buf;
        if(stat(p.c_str(), &stat_buf) == -1) { perror("stat"); return -1; }

        // find maximum values of numeric fields (for column formatting)
        if(stat_buf.st_nlink > max_nlink) max_nlink = stat_buf.st_nlink;
        if(stat_buf.st_size > max_size) max_size = stat_buf.st_size;
        
        // extract filename from full path
        std::string filename(p);
        filename = filename.substr(filename.find_last_of("/\\") + 1);

        if(LS_MODE & LS_MODE_SHOWALL || filename[0] != '.')
            total_blocks += stat_buf.st_blocks;

        filestats.insert(std::make_pair(filename, stat_buf));
    }


    // if listing, output used block total (1024 to 512 byte)
    if(LS_MODE & LS_MODE_LIST)
        std::cout << "total " << total_blocks/2 << std::endl;


    for(auto &fs : filestats) {

        std::string filename = fs.first;
        struct stat filestat = fs.second;

        if(LS_MODE & LS_MODE_SHOWALL || filename[0] != '.') {
            
            mode_t m = filestat.st_mode;
            char typechar;
            
            // filetype testing
            if(S_ISCHR(m))
                typechar = 'c';
            else if(S_ISBLK(m))
                typechar = 'b';
            else if(S_ISFIFO(m))
                typechar = 'p';
            else if(S_ISLNK(m))
                typechar = 'l';
            else if(S_ISSOCK(m))
                typechar = 's';
            else if(S_ISREG(m))
                typechar = '-';
            else if(S_ISDIR(m))
                typechar = 'd';
            

            // list mode formatting
            if(LS_MODE & LS_MODE_LIST) {
                std::string rights = "rwxrwxrwx";
                int mask = m & (S_IRWXU | S_IRWXG | S_IRWXO);
                for(int i = 0; i < 8; i++)
                    rights[8-i] = (mask & (1 << i)) ? rights[8-i] : '-';

                std::cout << typechar << rights << " ";
   
                // print link number
                nlink_t nlink = filestat.st_nlink;
                for(unsigned int i = 0; i < std::to_string(max_nlink).length() - std::to_string(nlink).length(); i++)
                    std::cout << " ";
                std::cout << nlink << " ";

                // print owner and group 
                uid_t uid = filestat.st_uid;
                gid_t gid = filestat.st_gid;

                struct passwd* pwd;
                errno = 0;
                pwd = getpwuid(uid);
                if(errno != 0) perror("getpwuid"); 

                struct group* grp;
                errno = 0;
                grp = getgrgid(gid);
                if(errno != 0) perror("getpwuid");

                std::cout << pwd->pw_name << " " << grp->gr_name << " ";

                // print size of file
                off_t size = filestat.st_size;
                for(unsigned int i = 0; i < std::to_string(max_size).length() - std::to_string(size).length(); i++)
                    std::cout << " ";
                std::cout << size << " ";

                // print time last modified
                // if modified before current year print year instead of time
                time_t* mtime = &filestat.st_mtime;
                time_t curtime = time(NULL);
                char* time_str = ctime(mtime);
                if(localtime(mtime)->tm_year == localtime(&curtime)->tm_year)
                    for(int i = 4; i < 16; i++)
                        std::cout << time_str[i];
                else {
                    for(int i = 4; i < 11; i++)
                        std::cout << time_str[i];
                    std::cout << " " << localtime(mtime)->tm_year + 1900;
                }

                std::cout << " ";
            }
    
            // filetype coloring
            switch(typechar) {
                case '-':
                std::cout << LS_COL_FILE;
                break;

                case 'd':
                std::cout << LS_COL_DIR;
                break;

                default:
                break;
            }

            if(filename[0] == '.') std::cout << LS_COL_HIDDEN;

            std::cout << filename << LS_COL_DEFAULT << "  ";
 
            if(LS_MODE & LS_MODE_LIST)
                std::cout << std::endl;
        }
    }

    return 0;
}



// scan a directory for all contained files
// return values: -1 - syscall error; 1 - is a file
int scandir(const char* path, std::vector<std::string> &files) {
   
    struct stat stat_buf;
    if(stat(path, &stat_buf) == -1) { perror("stat"); return 1; }

    if(!S_ISDIR(stat_buf.st_mode))
        return 1;

	DIR* dirp;
    if( (dirp = opendir(path)) == NULL) {
        perror("opendir");
        return -1; 
    }


    struct dirent* entry;
    errno = 0;
    while( (entry = readdir(dirp)) != NULL)
        files.push_back(entry->d_name);
    
    if(errno != 0) {
        perror("readdir");
        return -1;
    }

   if(closedir(dirp) != 0) {
        perror("closedir");
        return -1;
    }
    
    return 0;
}

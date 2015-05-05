#ifndef SCANDIR_H
#define SCANDIR_H
// ichor001: yanked from previous project // scan a directory for all contained files
// return values: -1 - syscall error; 1 - is a file
int scandir(const char* path, std::vector<std::string> &files) {
   
    struct stat stat_buf;
    if(stat(path, &stat_buf) == -1) { perror("stat: looking up file"); return 1; }

    if(!S_ISDIR(stat_buf.st_mode))
        return 1;

	DIR* dirp;
    if( (dirp = opendir(path)) == NULL) {
        perror("opendir: attempting to open directory");
        return -1; 
    }


    struct dirent* entry;
    errno = 0;
    while( (entry = readdir(dirp)) != NULL)
        files.push_back(entry->d_name);
    
    if(errno != 0) {
        perror("readdir: attempting to read directory contents");
        return -1;
    }

   if(closedir(dirp) != 0) {
        perror("closedir: attempting to close directory");
        return -1;
    }

    return 0;
}
#endif

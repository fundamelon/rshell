// LS.H
// Header file for rshell ls

enum LS_MODEFLAGS {
    LS_MODE_SHOWALL     = 0001,
    LS_MODE_RECURSIVE   = 0002,
    LS_MODE_LIST        = 0004
};

extern int LS_MODE;

std::vector<const char*> scandir(const char* path); 

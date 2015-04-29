// LS.H
// Header file for rshell ls

enum LS_MODEFLAGS {
    LS_MODE_SHOWALL     = 0001,
    LS_MODE_RECURSIVE   = 0002,
    LS_MODE_LIST        = 0004,
    LS_MODE_MANYFILES   = 0010
};

extern int LS_MODE;

void readloc(const char* path);

std::vector<const char*> scandir(const char* path); 

// LS.H
// Header file for rshell ls

enum LS_MODEFLAGS {
    LS_MODE_SHOWALL     = 00001,
    LS_MODE_RECURSIVE   = 00002,
    LS_MODE_LIST        = 00004,
    LS_MODE_MANYFILES   = 00010,
    LS_MODE_FIRSTENTRY  = 00100
};

extern int LS_MODE;

bool namecmp(std::string, std::string);
void readloc(const char* path);
int scandir(const char* path, std::vector<std::string> &); 

// LS.H
// Header file for rshell ls

enum LS_MODEFLAGS {
    LS_MODE_SHOWALL     = 0001,
    LS_MODE_RECURSIVE   = 0002,
    LS_MODE_LIST        = 0004,
    LS_MODE_MANYFILES   = 0010
};

extern int LS_MODE;

bool namecmp(std::string, std::string);
void readloc(const char* path);
std::vector<std::string> scandir(const char* path); 

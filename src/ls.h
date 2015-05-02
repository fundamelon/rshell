// LS.H
// Header file for rshell ls

enum LS_MODEFLAGS {
    LS_MODE_SHOWALL     = 00001,
    LS_MODE_RECURSIVE   = 00002,
    LS_MODE_LIST        = 00004,
    LS_MODE_MANYFILES   = 00010,
    LS_MODE_FIRSTENTRY  = 00100
};

const char* LS_COL_DEFAULT =    "\033[0;39m";
const char* LS_COL_FILE =       "\033[1;92m";
const char* LS_COL_DIR =        "\033[1;34m";
const char* LS_COL_HIDDEN =     "\033[47m";

extern int LS_MODE;

struct classnamecmp;
bool namecmp(std::string, std::string);
void readloc(const char* path);
int printinfo(const char* path);
int printinfo(std::vector<std::string>);
int scandir(const char* path, std::vector<std::string> &); 

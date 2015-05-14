// RSHELL.CPP
// Main source code for rshell

// enable debug messages
//#define RSHELL_DEBUG
// prepend "[RSHELL]" to prompt, helps to differ from bash
#define RSHELL_PREPEND

#define COL_DEFAULT "\033[39m"
#define COL_PREPEND "\033[32m"

#include "unistd.h"
#include "sys/wait.h"
#include "stdio.h"
#include "errno.h"

#include <iostream>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/regex.hpp>

#include "rshell.h"


const char* CONN_AMP = "&&";
const char* CONN_PIPE = "||";
const char* CONN_SEMIC = ";";
const char* TOKN_COMMENT = "#";

const char* RDIR_PIPE_SYM = "|";
const char* RDIR_INPUT_SYM = "<";
const char* RDIR_OUTPUT_SYM = ">";

enum RDIR_TYPE {
    RDIR_PIPE = 0x001,
    RDIR_INPUT = 0x002,
    RDIR_OUTPUT = 0x004,
    RDIR_OUTPUT_APP = 0x008
};


struct pipe {
    pipe(int t) : type(t) {}
    pipe(int r, int l, int t) : type(t) { files[0] = r; files[1] = t; }
    int type;
    int pipefd[2] = { 0, 1 };
    int files[2];
};
 

/* Initialize environment */
void init() {}


/* Main loop - controls command line, parsing, and execution logic */
int run() {
   
    std::vector<std::string> tokens_spc;
    std::vector<std::string> tokens_pipe;
    std::vector<std::string> tokens_word;
    std::string usr_input;
    int prev_exit_code = 0;

    while(true) {
        
        bool skip_cmd = false;
        std::string prev_spc = "";
        usr_input = prompt();
 
        // regex '||', '&&', ';', or '#'
        tokens_spc = tokenize(usr_input, "(\\|\\||\\&\\&|;|#)");

        for(unsigned int i = 0; i < tokens_spc.size(); i++) {

            std::string spc = tokens_spc.at(i);

            boost::trim(spc);

#ifdef RSHELL_DEBUG
            std::cout << "<" << spc << ">" << std::endl;
#endif

            if(spc == "") continue;

            // assumption: a connector token has no whitespace
            if(         spc == std::string(CONN_AMP)) {
                if(i == 0 || prev_spc != "") {  
                    std::cout << "syntax error: bad token \"" << CONN_AMP << "\"\n";
                    break;
                } else if(prev_exit_code != 0  && i != 0) {
                    skip_cmd = true; 
                    continue;
                }
               
                prev_spc = spc;
                continue;

            } else if(  spc == std::string(CONN_PIPE)) {
                if(i == 0 || prev_spc != "") { 
                    std::cout << "syntax error: bad token \"" << CONN_PIPE << "\"\n";
                    break;
                } else if(prev_exit_code == 0 && i != 0) {
                    skip_cmd = true;
                    continue;
                } 
       
                prev_spc = spc;
                continue;

            } else if(  spc == std::string(CONN_SEMIC)) {
                if(i == 0)  {
                    std::cout << "syntax error: bad token \"" << CONN_SEMIC << "\"\n";
                    break;
                } else {
                    prev_exit_code = 0;
                    skip_cmd = false;
                    prev_spc = spc;
                    continue;
                }
            } else if(  spc == std::string(TOKN_COMMENT)) {
                break;
            }

            prev_spc = "";
            if(skip_cmd) continue;

            tokens_pipe = tokenize(spc, "\\||<|>"); // regex '|', '<', or '>'
            int syntax_err = 0;

            std::vector<std::string> cmd_set;
            std::vector<struct pipe> pipe_set;

            std::string cur_rdir = "";

            // syntax pass
            for(unsigned int pipe_i = 0; pipe_i < tokens_pipe.size(); pipe_i++) {

                std::string cmd = tokens_pipe.at(pipe_i);

                if(cmd == "") { 
                    tokens_pipe.erase(tokens_pipe.begin() + pipe_i);
                    break;
                }
                boost::trim(cmd);

                if(cmd == RDIR_PIPE_SYM) {
                    if(pipe_i == 0) {
                        syntax_err = 1;
                        std::cout << "syntax error: token \"|\" at start of command\n";
                        break;
                    } else if(cur_rdir != "") {
                        syntax_err = 1;
                        std::cout << "syntax error: bad token near \"|\"\n";
                        break;
                    }

                    cur_rdir.append(RDIR_PIPE_SYM);
                    
                    continue;

                } else if(cmd == RDIR_INPUT_SYM) {
                    if(pipe_i == tokens_pipe.size() - 1) {
                        syntax_err = 1;
                        std::cout << "syntax error: expected file for input \"<\"\n";
                        break;
                    }

                    continue;

                } else if(cmd == RDIR_OUTPUT_SYM) {
                    if(pipe_i == tokens_pipe.size() - 1) {
                        syntax_err = 1;
                        std::cout << "syntax error: expected file for output \">\"\n";
                        break;
                    }

                    // skip to next consecutive output redir symbol
                    if(tokens_pipe.at(pipe_i + 1) == RDIR_OUTPUT_SYM) {
                        pipe_i++;
                        continue;
                    } 

                } else {
                    cmd_set.push_back(cmd);
                }
            }

            if(syntax_err == 1) break;

            // command running pass
            for(unsigned int cmd_i = 0; cmd_i < cmd_set.size(); cmd_i++) {

                auto cmd = cmd_set.at(cmd_i);

                tokens_word = toksplit(cmd, " ");
                for(unsigned int j = 0; j < tokens_word.size(); j++) {

                    std::string word = tokens_word.at(j);

                    // using boost for convenience - this can be implemented manually
                    boost::trim(word);

                    // kill empty words
                    if(word == "") tokens_word.erase(tokens_word.begin() + j);
                }

                std::vector<char*> cmd_argv(tokens_word.size() + 1);
                for(unsigned int k = 0; k < tokens_word.size(); k++) { 
                    cmd_argv[k] = &tokens_word[k][0]; 
#ifdef RSHELL_DEBUG 
                    std::cout << "\t" << "<" << tokens_word.at(k) << ">" << std::endl;
#endif
                }            

                // exit only if first word is "exit", after formatting
                if(tokens_word.at(0) == "exit") return 0;

                prev_exit_code = execute(cmd_argv[0], cmd_argv.data());

                // if execvp returned and had error, stop the process
                //  if(err_num == 1) return 1;
            }
        }
    }
    
    return 0;
}


/* Prints prompt text and takes in raw command line input */
std::string prompt() {
    
    char hostname[HOST_NAME_MAX];
    if(gethostname(hostname, HOST_NAME_MAX) == -1)
        perror("gethostname");
    
#ifdef RSHELL_PREPEND
    std::cout << COL_PREPEND << "[RSHELL] ";
#endif

    std::cout << COL_DEFAULT;

    std::cout << getlogin() << "@" << hostname;
    std::cout << "$ " << std::flush;

    std::string input_raw;
    std::getline(std::cin, input_raw);

    return input_raw;
}


/* fork and exec a program, complete error checking */
int execute(const char* path, char* const argv[]) {

#ifdef RSHELL_DEBUG
    std::cout << "executing " << path << std::endl;
#endif
    
    int pid = fork();

#ifdef RSHELL_DEBUG
    std::cout << "created process with id " << pid << std::endl;
#endif

    if(pid == -1) {
        perror("fork");
        return -1;
    } else if(pid == 0) {
        execvp(path, argv);
        perror(path);
        exit(1);
    } else {

        int exit_code; 
        
        // pass exit code along
        if(waitpid(pid, &exit_code, 0) == -1) perror("waitpid");
        return exit_code;    
    }

    return 0;
}


/* Overload to tokenize by whitespace */
std::vector<std::string> tokenize(std::string s) {
    return tokenize(s, "\\s");
}


/* Tokenize a string using boost regex */
std::vector<std::string> tokenize(std::string s, std::string r) {

    std::vector<std::string> token_vec;
 
//    boost::algorithm::split_regex(token_vec, s, boost::regex(r));
   
    std::string::const_iterator s_start, s_end;
    s_start = s.begin();
    s_end = s.end();

    boost::match_results<std::string::const_iterator> results;
    boost::match_flag_type flags = boost::match_default;
    while(boost::regex_search(s_start, s_end, results, boost::regex(r), flags)) {

        token_vec.push_back(std::string(s_start, results[0].first));
        token_vec.push_back(results[0]);
        
        s_start = results[0].second;
        flags |= boost::match_prev_avail;
        flags |= boost::match_not_bob;
    }

    token_vec.push_back(std::string(s_start, s_end));

    // scrub vector of empty fields
        

    for(unsigned int i = 0; i < token_vec.size(); i++) {
#ifdef RSHELL_DEBUG
//        std::cout << "[" << token_vec.at(i) << "]" << std::endl; 
#endif
        if(token_vec.at(i) == "") token_vec.erase(token_vec.begin() + i);
    } 
        
    return token_vec;
}


/* Tokenize a string using boost split */
std::vector<std::string> toksplit(std::string s, std::string toks) {

    std::vector<std::string> token_vec;
    boost::split(token_vec, s, boost::is_any_of(toks), boost::token_compress_on);
    return token_vec;
}

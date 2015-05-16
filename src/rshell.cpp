// RSHELL.CPP
// Main source code for rshell

#include "unistd.h"
#include "stdio.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "sys/wait.h"
#include "fcntl.h"
#include "errno.h"
#include "signal.h"

#include <iostream>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/regex.hpp>


// enable debug messages and macros
#define RSHELL_DEBUG
// prepend "[RSHELL]" to prompt, helps to differ from bash
#define RSHELL_PREPEND

#define COL_DEFAULT "\033[39m"
#define COL_PREPEND "\033[32m"
#define COL_DEBUG   "\033[33m"

// debug print macro
#ifdef RSHELL_DEBUG
#define _PRINT(stream) std::cout << COL_DEBUG << "[DEBUG] " \
                                 << COL_DEFAULT << stream \
                                 << std::endl << std::flush;
#else
#define _PRINT(stream)
#endif


#include "rshell.h"



const char* CONN_AMP = "&&";
const char* CONN_PIPE = "||";
const char* CONN_SEMIC = ";";
const char* TOKN_COMMENT = "#";

const char* REDIR_SYM_PIPE = "|";
const char* REDIR_SYM_INPUT = "<";
const char* REDIR_SYM_INPUT_STR = "<<<";
const char* REDIR_SYM_OUTPUT = ">";
const char* REDIR_SYM_OUTPUT_APP = ">>";

enum REDIR_TYPE {
    REDIR_TYPE_CMD        = 0x001,
    REDIR_TYPE_PIPE       = 0x002,
    REDIR_TYPE_INPUT      = 0x004,
    REDIR_TYPE_INPUT_STR  = 0x008,
    REDIR_TYPE_OUTPUT     = 0x010,
    REDIR_TYPE_OUTPUT_APP = 0x020
};



volatile sig_atomic_t sigint_flag = 0;
void catch_interrupt(int sig_num) {
    sigint_flag = 1;
}
 


/* Initialize environment */
void init() {

    //signal(SIGINT, catch_interrupt);
}



/* Main loop - controls command line, parsing, and execution logic */
int run() {
   
    std::vector<std::string> tokens_spc;
    std::vector<std::string> tokens_redir;
    std::vector<std::string> tokens_word;
    std::string usr_input;
    int prev_exit_code = 0;

    _PRINT("starting in debug mode");

    while(true) {
       
        if(sigint_flag) {  
            _PRINT("interrupt signal recieved, ignoring")
            sigint_flag = 0;
        }

        bool skip_cmd = false;
        std::string prev_spc = "";
        usr_input = prompt();

        // split input line into connector-delimited complex commands
        // regex '||', '&&', ';', or '#'
        tokens_spc = tokenize(usr_input, "(\\|\\||\\&\\&|;|#)");

        for(unsigned int i = 0; i < tokens_spc.size(); i++) {

            std::string spc = tokens_spc.at(i);
            boost::trim(spc);

            _PRINT("initial parser: <" << spc << ">")
            
            if(spc == "") continue;

            // assumption: a connector token has no whitespace
            if(         spc == std::string(CONN_AMP)) {
                
                if(i == 0 || prev_spc != "") {  
                    std::cout << "syntax error: bad token \"" << CONN_AMP << "\"\n";
                    break;
                } else if(prev_exit_code != 0  && i != 0) {
                    skip_cmd = true; 
                }
               
                prev_spc = spc;
                continue;

            } else if(  spc == std::string(CONN_PIPE)) {
               
                if(i == 0 || prev_spc != "") { 
                    std::cout << "syntax error: bad token \"" << CONN_PIPE << "\"\n";
                    break;
                } else if(prev_exit_code == 0 && i != 0) {
                    skip_cmd = true;
                } 
                
                prev_spc = spc;
                continue;

            } else if(  spc == std::string(CONN_SEMIC)) {

                prev_spc = spc;
                if(i == 0)  {
                    std::cout << "syntax error: bad token \"" << CONN_SEMIC << "\"\n";
                    break;
                } else {
                    prev_exit_code = 0;
                    skip_cmd = false;
                }

                continue;

            } else if(  spc == std::string(TOKN_COMMENT)) {
                break;
            }

            prev_spc = "";
            if(skip_cmd) continue;


            // begin parsing complex command
            tokens_redir = tokenize(spc, "\\|+|<+|>+"); // regex '|', '<', or '>'
            int syntax_err = 0;

            std::vector<std::string> cmd_set;
            std::vector<struct redir> redir_set;


            // redirection syntax pass
            for(unsigned int redir_i = 0; redir_i < tokens_redir.size(); redir_i++) {

                std::string cmd = tokens_redir.at(redir_i);

                boost::trim(cmd);

                if(cmd == "") { 
                //    tokens_redir.erase(tokens_redir.begin() + redir_i);
                    std::cout << "syntax error: unexpected null command\n";
                    syntax_err = 2;
                    break;
                }

                cmd_set.push_back(cmd);

                if(cmd == REDIR_SYM_PIPE) { // '|' piping operator
                    if(redir_i == 0) {
                        syntax_err = 1;
                        std::cout << "syntax error: token \"|\" at start of command\n";
                        break;
                    } else if(redir_set.back().type != REDIR_TYPE_CMD) {
                        syntax_err = 1;
                        std::cout << "syntax error: bad token near \"|\"\n";
                        break;
                    }

                    redir_set.push_back(redir(REDIR_TYPE_PIPE));
                    continue; // default action

                } else if(cmd == REDIR_SYM_INPUT) { // '<' input redirection operator
                    if(redir_i == tokens_redir.size() - 1) {
                        syntax_err = 1;
                        std::cout << "syntax error: expected file for input \"<\"\n";
                        break;
                    }

                    redir_set.push_back(redir(REDIR_TYPE_INPUT));
                    continue; // default action

                } else if(cmd == REDIR_SYM_OUTPUT) { // '>' output redirection operator
                    if(redir_i == tokens_redir.size() - 1) {
                        syntax_err = 1;
                        std::cout << "syntax error: expected file for output \">\"\n";
                        break;
                    }

                    redir_set.push_back(REDIR_TYPE_OUTPUT);
                    continue; // default action

                } else if(cmd == REDIR_SYM_OUTPUT_APP) { // '>>' operator

                    if(redir_i == tokens_redir.size() - 1) {
                        syntax_err = 1;
                        std::cout << "syntax error: expected file for output \">\"\n";
                        break;
                    }
                        
                    redir_set.push_back(REDIR_TYPE_OUTPUT | REDIR_TYPE_OUTPUT_APP);
                    continue;

                } else if(cmd[0] == REDIR_SYM_PIPE[0] || 
                        cmd[0] == REDIR_SYM_INPUT[0] ||
                        cmd[0] == REDIR_SYM_OUTPUT[0]) { // invalid operator

                    syntax_err = 1;
                    std::cout << "syntax error: bad operator \"" << cmd << "\"\n";
                    break;

                } else // normal command
                    redir_set.push_back(redir(REDIR_TYPE_CMD));
            } // end redirection syntax pass

            if(syntax_err != 0) break;


            bool single_cmd = cmd_set.size() == 1;
            std::vector<int> open_fds;

            if(!single_cmd) {
                for(unsigned int test_i = 0; test_i < cmd_set.size(); test_i++)
                    _PRINT("redir parser: \"" << cmd_set.at(test_i) << "\" : " << redir_set.at(test_i).type)
            }


            // file descriptor forwarded between, default: std input
            int fd_fwd = STDIN_FILENO;

            // command execution pass
            for(unsigned int cmd_i = 0; cmd_i < cmd_set.size(); cmd_i++) {

                auto cmd = cmd_set.at(cmd_i);
                auto redir = redir_set.at(cmd_i);

                // no action on redir operators
                if(redir.type != REDIR_TYPE_CMD) continue;

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
                    
                    _PRINT("\t" << "<" << tokens_word.at(k) << ">");
                }            

                // exit only if first word is "exit", after formatting
                if(tokens_word.at(0) == "exit") return 0;


                // execute command
                if(single_cmd)
                    prev_exit_code = execute(cmd_argv[0], cmd_argv.data());
                else {

                    // piping logic
                    
                    if(cmd_i + 1 < cmd_set.size()) {
                       
                        // '|' piping
                        if(redir_set.at(cmd_i+1).type == REDIR_TYPE_PIPE) {
                            redir.type = REDIR_TYPE_PIPE;
                        }

                        // '<', '>', '>>' file redirection
                        else if(redir_set.at(cmd_i+1).type & 
                                (REDIR_TYPE_INPUT | REDIR_TYPE_OUTPUT)) {
                            const char* filepath = cmd_set.at(cmd_i + 2).c_str();
                            redir.file = filepath;
                            redir.type = redir_set.at(cmd_i+1).type;

                            // set to pipe if there exists a connector afterward
                            if(cmd_i + 3 < cmd_set.size() && !(redir_set.at(cmd_i+3).type == REDIR_TYPE_CMD) ) {
                                redir.type |= REDIR_TYPE_PIPE;
                                cmd_i ++;
                        }

                            cmd_i += 2;
                        } else {
                            std::cout << "error: unknown redirection type\n";
                        }

                    } else if(cmd_i > 1 && redir_set.at(cmd_i - 1).type == REDIR_TYPE_PIPE) {
                        // end of command after pipe: use output redirection
                        redir.file = NULL;
                        redir.type = REDIR_TYPE_OUTPUT;
                        redir.file_fd = STDOUT_FILENO; // override write file
                    }

                    prev_exit_code = execute(&redir, &fd_fwd, cmd_argv[0], cmd_argv.data());

                    // collect open fds
                    if(redir.type & REDIR_TYPE_PIPE) {
                        if(redir.pipefd[0] != STDIN_FILENO) 
                            open_fds.push_back(redir.pipefd[0]);
                        if(redir.pipefd[1] != STDOUT_FILENO)
                            open_fds.push_back(redir.pipefd[1]);
                    }
                    if(redir.type & (REDIR_TYPE_INPUT | REDIR_TYPE_OUTPUT)) {
                        open_fds.push_back(redir.file_fd);
                    }
                }
            } // end command execution pass


            // close all latent fds
            for(auto fd : open_fds) {
                _PRINT("closing fd " << fd)
                if(close(fd) == -1 && fd > 2) { perror("close"); break; }
            }

            //if(prev_exit_code != 0) break;

            // wait for all child processes to end and save exit code
            int pid_child;
            while((pid_child = waitpid(-1, &prev_exit_code, 0))) {
                if(pid_child == -1) break;
                else {

                    _PRINT("child process terminated, pid: " << pid_child)
                    if(!WIFEXITED(prev_exit_code) || WEXITSTATUS(prev_exit_code) != 0) {
                        if(prev_exit_code != errno) // prevent repetition of errmsgs
                            perror("child process: waitpid");
                        break;
                    }
                }
            }
            
            if(prev_exit_code != 0) break;
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
    return execute(NULL, NULL, path, argv);
}



/* fork and exec a program, complete error checking
 *      redir_info: struct containing all redirection data
 *      fd_fwd: forwarded fd for chaining pipes
 */
int execute(struct redir* redir_info, int* fd_fwd, const char* path, char* const argv[]) {

    _PRINT("*** execute " << path)

    bool use_redir = (redir_info != NULL);

    int fd_in_old = STDIN_FILENO, fd_in_new = STDIN_FILENO; // stdin
    int fd_out_old = STDOUT_FILENO, fd_out_new = STDOUT_FILENO; // stdout


    if(use_redir) {
        _PRINT("execute: using redirection " << redir_info->type)
        _PRINT("redir: forwarded fd: " << *fd_fwd)

        if(redir_info->type & REDIR_TYPE_PIPE) {
            // expects forwarded fd from chain
            fd_in_new = *fd_fwd;

            //if(pipe(redir_info->pipefd) != 0) {
            if(pipe2(redir_info->pipefd, O_CLOEXEC) != 0) {
                perror("pipe");
                return errno;
            }

            _PRINT("redir: piping [" << redir_info->pipefd[0] << ", " << redir_info->pipefd[1] << "]")

            // set to attach pipe to output, forward read fd, close old fd_fwd
            fd_out_new = redir_info->pipefd[1];
            *fd_fwd = redir_info->pipefd[0];
        }

        // file redirection
        if(redir_info->type & (REDIR_TYPE_INPUT | REDIR_TYPE_OUTPUT)) {
            // expects forwarded fd from chain
            // expects output file in redir_info->file (or NULL for stdout override)
            if(redir_info->file != NULL) {

                // input redir setup
                if(redir_info->type & REDIR_TYPE_INPUT) {
                    _PRINT("redir: input from file " << redir_info->file)

                    fd_in_new = open(redir_info->file, O_RDONLY | O_CLOEXEC);
                    if(fd_in_new == -1) { 
                        perror("input redirect: open"); 
                        return errno;
                    }

                    redir_info->file_fd = fd_in_new;
                    // rest already taken care of in piping block above
                }

                // output redir setup
                else if(redir_info->type & REDIR_TYPE_OUTPUT) {
                    _PRINT("redir: output to file " << redir_info->file)
                    int app_flag = (redir_info->type & REDIR_TYPE_OUTPUT_APP) ? O_APPEND : O_TRUNC;
                    fd_out_new = open(redir_info->file, O_CREAT | O_WRONLY | O_CLOEXEC | app_flag);
                    if(fd_out_new == -1) {
                        perror("output redirect: open");
                        return errno;
                    } 

                    redir_info->file_fd = fd_out_new;
                    // does not modify forwarded fd (convention)
                    fd_in_new = *fd_fwd;                            

                } else {
                    std::cout << "error: something went crazy wrong with redir i/o\n";
                    return -1;
                }
            } else {
                // end of chain override
                _PRINT("redir: end of pipe chain")
 
                redir_info->file_fd = fd_out_new;
                // does not modify forwarded fd (convention)
                fd_in_new = *fd_fwd;           
            }


        }
    }

    int pid = fork();

    if(pid != 0) _PRINT("execute: created process with id " << pid)

    if(pid == -1) {
        perror("error: fork");
        return -1;
    } else if(pid == 0) { // child
        // fd redirection swapping
        if(use_redir) {

            // redirect stdin
            if(fd_in_old != fd_in_new) {
                if(close(fd_in_old) == -1) {
                    perror("input redirect: close");
                    exit(errno);
                }

                // input from forwarded fd
                if(dup(fd_in_new) == -1) {
                    perror("input redirect: dup");
                    exit(errno);
                }
            }

            // redirect stdout
            if(fd_out_old != fd_out_new) {
                if(close(fd_out_old) == -1) {
                    perror("output redirect: close");
                    exit(errno);
                }

                // output to redir file
                if(dup(fd_out_new) == -1) {
                    perror("output redirect: dup");
                    exit(errno);
                }
            }
 
            if(redir_info->type & REDIR_TYPE_PIPE) {
                // close read end of pipe
                if(close(redir_info->pipefd[0]) == -1) {
                    perror("pipe read end: close");
                    exit(errno);
                }
            }
        }

        execvp(path, argv);
        perror(path);
        exit(1);
    } else if(use_redir) { // parent
    }

    if(use_redir) _PRINT("redir: forwarding fd " << *fd_fwd)
    _PRINT("*** execute " << path << ": success")
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

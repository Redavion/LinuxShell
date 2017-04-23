#ifndef APP_H
#define APP_H

#include <iostream>
#include <sched.h>
#include <vector>
#include <string>
#include <map>

class app
{
public:
    app();
    ~app();
    void start();
    static const std::vector< std::string > builtincmds;

private:
    // private methods
    int execute( std::vector< std::string >& str, int outfd, int infd, int endtoclose );
    int parallel_execution( std::string command_string );
    int executebuiltin( std::vector< std::string >& command );
    void process_parallel_command_entry( std::string& command_str, std::map< int, std::string >& children,
    int outfd = STDOUT_FILENO, int infd = STDIN_FILENO, int endtoclose = -1 );

    // private fields
    int virtual_memory_limit;
    int policy;
    int priority;
};
//this structure will be used to store a set of pipes in a vector
struct pipelink{
    int readfd;
    int writefd;
};

#endif // APP_H

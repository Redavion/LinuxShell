#include <algorithm>
#include <iostream>
#include <math.h>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <system_error>
#include <time.h>
#include <unistd.h>
#include <vector>

#include "app.h"
#include "utilities.h"

using namespace std;
std::map< int, std::string > background_children;

// initialize static members
const vector< string > app::builtincmds( {"set_memlimit", "cd"} );

void writehere( const char* msg )
{
    write( STDOUT_FILENO, msg, strlen( msg ) );
}

// async-safe implementation
void positive_integer_to_string( int number, char* buffer, int length )
{
    number = abs( number ); // just in case

    int numdigits = ( number ) ? log10( number ) : 1;

    if ( length < ( numdigits + 1 ) )
    {
        return;
    }

    for ( int i = 1; i < numdigits; i++ )
    {
        buffer[ numdigits - i ] = number % 10 + '0';
        number /= 10;
    }

    buffer[ numdigits ] = '\0';
}

std::map< int, std::string > siCodeDesc;

void signal_handler( int signum, siginfo_t* siginfo, void* ucontext )
{
    if ( signum == SIGCHLD )
    {
        LOG << "signal handler - child pid " << siginfo->si_pid
            << " had signal code " << siCodeDesc[ siginfo->si_code ]
            << "and exit signal " << siginfo->si_status << "\n";
        // check if the process we are interested in is in the background tasks
        if ( background_children.find( siginfo->si_pid ) !=
             background_children.end() )
        {
            if ( siginfo->si_code == CLD_EXITED )
            {

                int exitcode = siginfo->si_status;
                std::string msg =
                    ": Child " +
                    ( ( exitcode )
                            ? "Self Terminated With Exit Code " + std::to_string( exitcode )
                            : "Exited Successfully" ) +
                    "\n";
                LOG << background_children[ siginfo->si_pid ] << msg;
            }
            else if ( siginfo->si_code == CLD_DUMPED ||
                      siginfo->si_code == CLD_KILLED )
            {
                std::string msg = ": Child Terminated Due to Signal \n";
                LOG << background_children[ siginfo->si_pid ] << msg;
            }
        }
    }
}

// Constructor
app::app()
: virtual_memory_limit( -1 )
{
    struct sigaction new_action;
    new_action.sa_sigaction = signal_handler;
    new_action.sa_flags = SA_RESTART | SA_SIGINFO;
    sigemptyset( &new_action.sa_mask );

    if ( sigaction( SIGCHLD, &new_action, 0 ) == -1 )
    {
        printf( "process %d: error while installing handler for SIGINT\n", getpid() );
    }

    siCodeDesc[ CLD_EXITED ] = "CLD_EXITED (child has exited) ";
    siCodeDesc[ CLD_KILLED ] = "CLD_KILLED (child was killed) ";
    siCodeDesc[ CLD_DUMPED ] = "CLD_DUMPED (child terminated abnormally) ";
    siCodeDesc[ CLD_TRAPPED ] = "CLD_TRAPPED (traced child has trapped) ";
    siCodeDesc[ CLD_STOPPED ] = "CLD_STOPPED (child has stopped) ";
    siCodeDesc[ CLD_CONTINUED ] = "CLD_CONTINUED (stopped child has continued) ";

    cerr << "===============================================" << endl;
    cerr << "Welcome to the CS3281 Shell Assignment " << endl;
    cerr << "===============================================" << endl;
}

// Destructor
app::~app()
{
    cerr << "\n";
    cerr << "===============================================" << endl;
    cerr << "Closing CS3281 Shell Assignment " << endl;
    cerr << "===============================================" << endl;
}

int app::parallel_execution( std::string command_string )
{
    std::vector< string > parallel_commands;
    std::map< int, string > children;
    tokenize_string( command_string, parallel_commands );
    std::vector< string > command;
    bool isForeground;

    // @Task 1: Launch the parallel commands - note that if command1 ||
    // command2
    // is given, you must launch both of them and then wait for both of them.
    // make sure to check if they are built in and if they should not be
    // waited upon i.e. if they are not foreground tasks. Use checkforeground
    // function for that.
    // child is a pair of <int,string>
    sigset_t signal_set;
    sigemptyset( &signal_set );
    sigaddset( &signal_set, SIGCHLD );
    sigprocmask( SIG_BLOCK, &signal_set, NULL );

    for ( auto command_str : parallel_commands )
    {
        command.clear();
        tokenize_string( command_str, command, " " );
        isForeground = checkforeground( command );

        // if the command is built in, execute it (if is in background, remove
        // leading &)
        if ( checkbuiltin( command ) )
        {
            if ( !isForeground )
            {
                command.pop_back(); // remove & from command
            }
            executebuiltin( command );
        }
        // the command isn't built in, execute and add to wait list if it's
        // foreground
        else
        {
            if ( isForeground )
            {
                int pid = execute( command ); // execute process
                children[ pid ] = command[ 0 ]; // add pid for process to be waited on
            }
            else
            {
                // This is a background task.
                command.pop_back(); // remove & from command
                int pid = execute( command ); // execute process
                LOG << "added " << command[ 0 ] << " to log\n";
                background_children[ pid ] = command[ 0 ];
            }
        }
    }

    // Now wait for all the foreground jobs to complete
    for ( auto child : children )
    {
        int status = 0;

        if ( waitpid( child.first, &status, 0 ) < 0 )
        {
            LOG << "error occurred " << strerror( errno ) << " (" << status << ") \n";
            continue;
        }
        else
        {
            if ( WIFEXITED( status ) )
            {
                int exitcode = WEXITSTATUS( status );
                std::string msg =
                    ": Child " +
                    ( ( exitcode )
                            ? "Self Terminated With Exit Code " + std::to_string( exitcode )
                            : "Exited Successfully" ) +
                    "\n";
                LOG << child.second << msg;
            }
            else if ( WIFSIGNALED( status ) )
            {
                LOG << child.second << ": Child Terminated Due to Signal "
                    << WTERMSIG( status ) << "\n";
            }
        }
    }

    sigprocmask( SIG_UNBLOCK, &signal_set, NULL );
    return 0;
}

bool checkbuiltin( std::vector< std::string >& command )
{
    //@ Task2: complete this check to include other built in commands - see
    // readme.
    auto end = app::builtincmds.end();
    return end != std::find( app::builtincmds.begin(), end, command[ 0 ] );
}

int app::executebuiltin( std::vector< string >& command )
{
    // all built in command have 1 command and 1 argument. The last entry is
    // return.

    if ( command.size() != 2 )
    {
        LOG << "Built-in commands require two arguments\n";
        return -1;
    }

    if ( command[ 0 ] == "cd" && command.size() >= 2 )
    {
        // @Task 3: Implement the command to change directory. Search for chdir
        LOG << "Got a command to change directory to " << command[ 1 ] << std::endl;
        if ( chdir( command[ 1 ].c_str() ) < 0 )
        {
            LOG << "Error occured while changing directory to " << command[ 1 ]
                << " error was " << strerror( errno ) << std::endl;
        }
    }

    else if ( command[ 0 ] == "set_memlimit" )
    {
        std::cout << "setting memlimit to " << command[ 1 ] << " bytes\n";
        try
        {
            this->virtual_memory_limit = std::stoi( command[ 1 ] ); // in bytes
        }
        catch ( ... )
        {
            LOG << "Exception occured while converting " << command[ 1 ] << " to int\n";
        }
    }
}

// This function is going to execute the shell command and going to execute
// wait, if the second parameter is true;
int app::execute( std::vector< string >& command )
{
    int status;

    // Command string can contain the main command and a number of command line
    // arguments. We should allocate one extra element to have space for null.
    int commandLen = command.size();
    // If executing in background, remove "&" from command list passed to execvp
    if ( command[ commandLen - 1 ] == "&" )
    {
        commandLen--;
    }
    char** args = ( char** )malloc( ( commandLen + 1 ) * sizeof( char* ) );
    for ( int i = 0; i < commandLen; i++ )
    {
        args[ i ] = strdup( command[ i ].c_str() );
    }
    args[ commandLen ] = 0;
    // create a new process
    pid_t w = fork();
    if ( w < 0 )
    {
        LOG << "\nFork Failed " << errno << "\n";
    }
    else if ( w == 0 )
    {
        // @Task 5: Use the API to implement the memory limits
        if ( this->virtual_memory_limit > 0 )
        {
            struct rlimit rl;
            rl.rlim_cur = this->virtual_memory_limit;
            rl.rlim_max = this->virtual_memory_limit;
            if ( setrlimit( RLIMIT_AS, &rl ) == -1 )
            {
                LOG << "Setting rlimit failed" << strerror( errno ) << endl;
            }
        }

        LOG << "Going to exec " << args[ 0 ] << "\n";
        execvp( args[ 0 ], args );
        LOG << "\nExec Failed " << errno << "\n";
        exit( 2 );
    }
    else if ( w > 0 )
        return w;
}

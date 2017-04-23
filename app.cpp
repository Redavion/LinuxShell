#include <algorithm>
#include <iostream>
#include <math.h>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <system_error>
#include <time.h>
#include <unistd.h>
#include <vector>

#include "app.h"
#include "utilities.h"
//max number of pipes
const int COMMAND_MAX = 10;
int totalNumberOfPipes;
using namespace std;
std::map< int, std::string > background_children;
struct sched_param p;
int pipes[(COMMAND_MAX * 2) - 1];


// initialize static members
const vector< string > app::builtincmds( {"set_memlimit", "cd", "set_policy", "set_prio", "get_policy",
                                          "get_prio"} );

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
: virtual_memory_limit( -1 ), policy( -1 ), priority ( -1 )
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

void app::process_parallel_command_entry(std::string &command_str, std::map<int, std::string> &children, int outfd,
                                         int infd, int endtoclose) {
    std::vector< string > command;
    command.clear();
    tokenize_string (command_str, command, " " );
    bool isForeground= checkforeground( command );
    //check for foreground
    //check for background and then call execute function

    //if command is built in
    if (checkbuiltin(command)){
        if (!isForeground){
            //strip the &
            command.pop_back();
        }
        //then execute it
       executebuiltin(command);
    } else { //if command isn't built in
        if (!isForeground){ //background task
            //strip the &
            command.pop_back();
            int pid = execute(command, outfd, infd, endtoclose); //execute
            background_children[ pid ] = command [ 0 ];
        } else {
            int pid = execute(command, outfd, infd, endtoclose); //execute
            children[ pid ] = command[0];
        }

    }
    return;


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

    std::vector< string > tokenizePipeCommands;

    for ( auto command_str : parallel_commands ) {
        //if there is a | in the command
        if (pipeFinder(command_str)) {
            tokenizePipeCommands.clear();
            tokenize_string(command_str, tokenizePipeCommands, "|");
            // if the number of pipe commands falls between 2 and 10
            if (tokenizePipeCommands.size() >= 2 && tokenizePipeCommands.size() < COMMAND_MAX) {
                totalNumberOfPipes=tokenizePipeCommands.size()-1;
                for (auto pipe_command_str : tokenizePipeCommands) {
                    command.clear();
                    tokenize_string(pipe_command_str, command, " ");
                    isForeground = checkforeground(command);
                    //check to make sure that the command is a foreground job and isn't built in
                    if (!isForeground || checkbuiltin(command)) {
                        LOG << command[0] << "can't be piped" << endl;
                        return -1;
                    }
                }

                //Create required number of pipes
                for (int i = 0; i < tokenizePipeCommands.size() - 1; i++) {
                    if (pipe(pipes + i * 2) < 0) {
                        printf("Pipe Failed\n");
                        return -1;
                    }
                }

                for (int i = 0; i < tokenizePipeCommands.size(); i++) {
                    auto command_s = tokenizePipeCommands[i];
                    if (i == 0) {
                        //first close the read end of the first pipe. Send STDIN_FILENO as the default read end.
                        process_parallel_command_entry(command_s, children, pipes[1], STDIN_FILENO, pipes[0]);
                    } else if (i == tokenizePipeCommands.size() - 1) {
                        process_parallel_command_entry(command_s, children, STDOUT_FILENO, pipes[(i - 1) * 2],
                                                       pipes[(i - 1) * 2 + 1]);
                    } else {
                        process_parallel_command_entry(command_s, children, pipes[(i * 2) + 1], pipes[(i - 1) * 2]);
                    }
                }

                for (int i = 0; i < (tokenizePipeCommands.size() - 1) * 2; i++) {
                    close(pipes[i]);
                }

            } else {
                printf("invalid number of pipes");
                return -1;
            }
        } else {
            process_parallel_command_entry(command_str, children);
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
    else if(command [0]=="set_prio"){

        int sched = sched_getscheduler((pid_t) 0);
        int prio = std::stoi(command[1]);
        //Make sure priority is within bounds of min and max
        if (prio >= sched_get_priority_min(sched) || prio <= sched_get_priority_max(sched)){
            priority = prio;
        } else {
            printf("Priority is invalid");
        }

    }
    else if (command [0] =="set_policy") {
        std::cout << "Setting policy to " << command[1] << "\n";
        // p.sched_priority=1;
        if (command[1] == "FIFO") {
            //so instead of all this we just do a direct store?
           /* p.sched_priority = 1;
            if (sched_setscheduler((pid_t) 0, SCHED_FIFO, &p) != -1) {
                printf("couldn't set scheduler to fifo");
            }*/
            policy = SCHED_FIFO;
        } else if (command[1] == "RR") {

            policy = SCHED_RR;

        } else if (command[1] == "OTHER") {

            policy = SCHED_OTHER;
        }
        //set the default priority
        priority = sched_get_priority_min(policy);
    } else if (command[0] =="get_policy"){
        //printing out policy
        int schedpolicy= sched_getscheduler((pid_t) 0);
        printf("The scheduling policy is: ");
        switch(schedpolicy){
            case SCHED_FIFO:
                printf("FIFO\n");
                break;
            case SCHED_RR:
                printf("ROUNDROBIN\n");
                break;
            case SCHED_OTHER:
                printf("OTHER\n");
                break;
            default:
                printf("unknown\n");
        }

    } else if (command[0]=="get_prio"){
        //can this just be cout prio?

         cout << "Current priority: " << priority << "\n";

    }

}


// This function is going to execute the shell command and going to execute
// wait, if the second parameter is true;
int app::execute( std::vector< string >& command, int outfd, int infd, int endtoclose ) {
    int status;
    // Command string can contain the main command and a number of command line
    // arguments. We should allocate one extra element to have space for null.
    int commandLen = command.size();
    // If executing in background, remove "&" from command list passed to execvp
    if (command[commandLen - 1] == "&") {
        commandLen--;
    }
    char **args = (char **) malloc((commandLen + 1) * sizeof(char *));
    for (int i = 0; i < commandLen; i++) {
        args[i] = strdup(command[i].c_str());
    }
    args[commandLen] = 0;
    // create a new process
    pid_t w = fork();
    if (w < 0) {
        LOG << "\nFork Failed " << errno << "\n";
    } else if (w > 0) {
        //parent process
        return w;
    } else if (w == 0) {
        //we are the child process
        // @Task 5: Use the API to implement the memory limits
        if (this->virtual_memory_limit > 0) {
            struct rlimit rl;
            rl.rlim_cur = this->virtual_memory_limit;
            rl.rlim_max = this->virtual_memory_limit;
            if (setrlimit(RLIMIT_AS, &rl) == -1) {
                LOG << "Setting rlimit failed" << strerror(errno) << endl;
            }
        }
        //set priority and policy after fork
        if (priority != -1) {
            if (policy != SCHED_OTHER) {
                struct sched_param p;
                p.sched_priority = priority;
                if (sched_setscheduler((pid_t) 0, SCHED_FIFO, &p) == -1) {
                    printf("Couldn't set scheduler to SCHED_FIFO\n");
                }
            }
            if (setpriority(PRIO_PROCESS, 0, priority) == -1) {
                    printf("Setting priority failed\n");
            }

         }
        //if the outfd is not STDOUT_FILENO, then you should duplicate stdout to outfd
        if (outfd != STDOUT_FILENO) {
            dup2(outfd, STDOUT_FILENO);
        }

        //if the infd is not STDIN_FILENO, then you should duplicate stdin to infd
        if (infd != STDIN_FILENO) {
            dup2(infd, STDIN_FILENO);
        }


        //close endtoclose if it is not -1, which is the default -
        // check the declaration of process_parallel_command_entry
        if (endtoclose != -1) {
            close(endtoclose);
        }
        for ( int i = 0; i < totalNumberOfPipes; i++ )
        {
            if(pipes[i*2]!=endtoclose && pipes[i*2+1]!=outfd && pipes[i*2]!=infd){
                close(pipes[i*2]);
            }
            if(pipes[i*2+1]!=endtoclose &&pipes[i*2+1]!=outfd && pipes[i*2+1]!=infd){
                close(pipes[i*2+1]);
            }
        }
        execvp(args[0], args);
        printf("Exec Failed\n");
        exit(2);

    }


}



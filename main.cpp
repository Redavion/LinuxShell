#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <unistd.h>
#include "app.h"
#include "utilities.h"

int main( int argc, char* argv[] )
{
    std::auto_ptr< app > a( new app );
    a->start();
}

void app::start()
{
    std::string command_str;
    std::cerr << "\nvbash>>";
    std::getline( std::cin, command_str );

    // strip the command line of white space
    // tokenize the commands into chunks of two
    // With the "exit" command it exits the program
    while ( command_str != "exit" )
    {
        command_str = trim( command_str );
        if ( command_str.size() != 0 )
        {
            this->parallel_execution( command_str );
        }
        std::cerr << "\nvbash>>";
        std::getline( std::cin, command_str );
    }
}

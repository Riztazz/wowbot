#include "client/GameClient.hpp"

#include <boost/program_options.hpp>
#include <iostream>

int main( int argc, char * argv[] )
{
    boost::program_options::options_description desc( "Usage" );
    desc.add_options()( "realmlist,r", boost::program_options::value<std::string>()->required(), "Realmlist ( 127.0.0.1:3724 )" );
    desc.add_options()( "username,u", boost::program_options::value<std::string>()->required(), "Username" );
    desc.add_options()( "password,p", boost::program_options::value<std::string>()->required(), "Password" );

    auto options = boost::program_options::parse_command_line( argc, argv, desc );

    boost::program_options::variables_map vm;
    boost::program_options::store( options, vm );

    try
    {
        vm.notify();
    }
    catch ( ... )
    {
        desc.print( std::cout );
        return -1;
    }

    const std::string realmlist = vm[ "realmlist" ].as<std::string>();
    const std::string username = vm[ "username" ].as<std::string>();
    const std::string password = vm[ "password" ].as<std::string>();

    Wow::GameClient client;
    return client.RunService( realmlist, username, password );
}

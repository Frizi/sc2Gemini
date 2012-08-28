#include <string>
#include <iostream>
#include <algorithm>
#include <signal.h>

#include <boost/program_options.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>

#include <csipc/Server.h>

#include "injections.h"

// messages

#define MSG_PREFIX "gemini."
#define MESSAGE_LOADLIB MSG_PREFIX "loadLib"
#define MESSAGE_INJECT MSG_PREFIX "inject"
#define MESSAGE_EXIT MSG_PREFIX "exit"

namespace po = boost::program_options;
namespace fs = boost::filesystem;

std::vector<fs::path> libraries;

void addLibraries(const std::vector<std::string> &libs);
void searchForLibraries(const std::vector<std::string> &directories);

bool exitSignalState = false;

void sigInt(int sig)
{
    exitSignalState = true;
}

int main(int argc, const char **argv)
{
    std::string application;

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h"   , "produce help message")
        ("run,r"    , po::value<std::string>(&application)->required(), "specifies a path to executable that will be runned and exploited\n"
                                                                        "   use '-' as arg for stdin")
        ("library,l", po::value<std::vector<std::string> >()->notifier(&addLibraries), "load given .dll into executable on startup")
        ("libdir,L" , po::value<std::vector<std::string> >()->notifier(&searchForLibraries), "load every found .dll inside given directory on startup")
    ;

    po::variables_map vm;
    try
    {
        po::store(po::parse_command_line(argc, argv, desc), vm);

        if ( vm.count("help") )
        {
            std::cout << "Usage: " << argv[0] << " [options]" << std::endl
            << desc << std::endl;
            return 0;
        }
        po::notify(vm);

        if(application == "-")
            std::getline(std::cin, application);
    }
    catch(po::error &e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
        std::cerr << desc << std::endl;
        return 1;
    }

    fs::path appPath = fs::path(application);

    signal(SIGINT, &sigInt);
    std::cout << "starting server" << std::endl;
    CsIpc::Server server("gemini");

    std::cout << "running " << appPath << std::endl;

    PROCESS_INFORMATION* pi = startexe(appPath.wstring().c_str());

    BOOST_FOREACH(const fs::path & lib, libraries)
    {
        std::cout << "loading " << lib << std::endl;
        injectDll(pi, lib.wstring().c_str());
        //Sleep(2);
    }

    // server loop
    CsIpc::EventMessage msg;
    bool done = false;
    bool exitState = false;
    while(!done)
    {
        // event loop
        while(server.Peek(msg))
        {
            std::string eventType = msg.getEventType();
            if( eventType == MESSAGE_LOADLIB )
            {
                fs::path pathToLib(msg.getParamString(0));
            }
            else if( eventType == MESSAGE_EXIT )
            {
                // prepare for exit
                exitState = true;
                server.Broadcast(msg);
            }
            else
                server.Broadcast(msg);
        }

        // exit condition
        if(exitState)
        {
            if(server.GetNumOfClients() == 0)
                done = true;
        }

        // check for signal
        if(exitSignalState && !exitState)
        {
            CsIpc::EventMessage exitMsg;

            exitMsg.setEventType( MESSAGE_EXIT );
            server.Broadcast(exitMsg);
            exitState = true;
        }
        else
            // do not use too many resources
            Sleep(1);
    }

    return 0;
}

void addLibraries(const std::vector<std::string> &libs)
{
    BOOST_FOREACH(const std::string & lib, libs)
    {
        libraries.push_back(absolute(fs::path(lib)));
    }
}

void searchForLibraries(const std::vector<std::string> &directories)
{
    BOOST_FOREACH(const std::string & strPath, directories)
    {
        fs::path dir_path(strPath);
        if(fs::is_directory(dir_path))
        {
            fs::directory_iterator end_itr;

            for ( fs::directory_iterator itr( dir_path ); itr != end_itr;  ++itr )
            {
                if( !fs::is_directory(itr->path()) )
                {
                    std::string ext = itr->path().extension().string();
                    // to lowercase
                    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                    if(ext == ".dll")
                        libraries.push_back(absolute(itr->path()));
                }
            }
        }
        else
        {
            throw po::error("specified libdir is not a directory");
        }
    }
}

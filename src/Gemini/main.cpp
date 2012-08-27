#include <string>
#include "injections.h"

#include <iostream>
#include <boost/program_options.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>

#include <algorithm>

namespace po = boost::program_options;
namespace fs = boost::filesystem;

std::vector<fs::path> libraries;

void addLibraries(const std::vector<std::string> &libs);
void searchForLibraries(const std::vector<std::string> &directories);

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

    std::cout << "running " << appPath << std::endl;

    PROCESS_INFORMATION* pi = startexe(appPath.wstring().c_str());
    //Sleep(2000);
    BOOST_FOREACH(const fs::path & lib, libraries)
    {
        std::cout << "loading " << lib << std::endl;
        injectDll(pi, lib.wstring().c_str());
        //Sleep(2);
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

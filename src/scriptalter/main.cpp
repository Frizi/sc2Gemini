#define BOOST_SYSTEM_NO_DEPRECATED

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <boost/filesystem/path.hpp>
#include <csipc/Client.h>

#include <Windows.h>

namespace fs = boost::filesystem;
using namespace std;

void modifyScript(fs::path tempPath);
BOOL WINAPI ConsoleHandler(DWORD CEvent);

bool done = false;

int main(int argc, char* argv[])
{
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler,TRUE);
    CsIpc::Client client("scriptalter", "gemini");
    client.Register("mapSave");
    client.Register("gemini.exit");

    CsIpc::EventMessage msg;
    cout << "main loop" << endl;

    while(!done)
    {
        while(client.Peek(msg))
        {
            const std::string eventType = msg.getEventType();
            if(eventType == "gemini.exit")
            {
                cout << "got gemini.exit" << endl;
                done = true;
            }
            else if(eventType == "mapSave")
            {
                cout << "got mapSave" << endl;
                fs::path tempPath = msg.getParamWstring(0);
                modifyScript(tempPath);
                CsIpc::EventMessage retMsg("mapSaveReturn");
                client.SendTo(msg.getSender(), retMsg);
            }
        }

        boost::detail::Sleep(2);
    }
    return 0;
}


void modifyScript(fs::path tempPath)
{
    fs::path scriptPath = tempPath;
    scriptPath += "/MapScript.galaxy";

    ifstream scriptFile;

    string line;
    stringstream buffer;
    scriptFile.open(scriptPath.string().c_str());
    if (scriptFile.is_open())
    {
        if(scriptFile.good())
        {
            getline(scriptFile,line);
            if(line.find("// Gemini injected") != string::npos)
            {
                scriptFile.close();
                return;
            }
            else
            {
                buffer << "// Gemini injected" << endl;
                buffer << line << endl;
            }
        }
        while ( scriptFile.good())
        {
            getline(scriptFile,line);
            buffer << line << endl;
            if(line.find("// Trigger Initialization") != string::npos)
            {
                buffer <<
                "// Gemini" << endl <<
                "bool gtGeminiTest(bool testConds, bool runActions){" << endl <<
                "    UIDisplayMessage(PlayerGroupAll(), c_messageAreaSubtitle, StringToText(\"Gemini injection successful\"));" << endl <<
                "    return true;" << endl <<
                "}" << endl <<
                "void gtGeminiTestInit(){" << endl <<
                "    TriggerAddEventTimeElapsed(TriggerCreate(\"gtGeminiTest\"), 3.0, c_timeGame);" << endl <<
                "}" << endl;
            }
            else if(line.find("void InitTriggers () {") != string::npos)
            {
                buffer << "gtGeminiTestInit();" << endl;
            }

        }
        scriptFile.close();

        ofstream outputFile;
        outputFile.open(scriptPath.string().c_str());
        if(outputFile.is_open())
        {
            outputFile << buffer.str();
            outputFile.close();
        }
        else cout << "Unable to open output file";
    }
    else cout << "Unable to open input file";
    return;
}

BOOL WINAPI ConsoleHandler(DWORD CEvent)
{
    switch(CEvent)
    {
    case CTRL_C_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_SHUTDOWN_EVENT:
    case CTRL_LOGOFF_EVENT:
        done = true;
        break;
    }
    return TRUE;
}

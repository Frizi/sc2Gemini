#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

int main(int argc, char* argv[])
{
    char* output;
    if(argc < 2) return 1;
        char* input = argv[1];
    if(argc > 2)
        output = argv[2];
    else
        output = input;

    ifstream scriptFile;

    string line;
    stringstream buffer;
    scriptFile.open(input);
    if (scriptFile.is_open())
    {
        if(scriptFile.good())
        {
            getline(scriptFile,line);
            if(line.find("// Gemini injected") != string::npos)
            {
                scriptFile.close();
                return 0;
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
        outputFile.open(output);
        if(outputFile.is_open())
        {
            outputFile << buffer.str();
            outputFile.close();
        }
        else cout << "Unable to open output file";
    }
    else cout << "Unable to open input file";
    return 0;
}

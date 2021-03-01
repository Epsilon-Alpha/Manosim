#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <map>
#include <set>
using namespace std;

short int accumulator, org, AR, DR, PC; //Registers
string IR, currentLine;
vector<string> lines, preHalt, postHalt; //Preprocessing
map<string,int> valueOf, address; //address["A"] -> Gives memory address of A, valueOf["A"] -> Gives value of A
map<int,string> variables, hexmap; //variables[100] -> Variable stored at that address
multimap<string,int> memory;
set<pair<int,string> > memorySet; //Contains (address,register) pair
ifstream fin("file2.as"); //Read from this file

void process(string,string);
void loadFromAcc(string);
void load(short int);
void originate(string);
void add(short int);
void subtract(short int);
void multiply(short int);
void divide(short int);
void bitwiseXOR(short int);
void bitwiseOR(short int);
void bitwiseAND(short int);
void complement();
short int hexToInt(string);
string hexValue(short int);
void createHexMap();
void readLines();
void separateByHalt();
void processPostHalt();
void processPreHalt();
void createMemorySet();
void printMemory();
void printVariables();
void printHexMap();
void callProcess();
//display(hexValue(variable)) -> Displays the hexadecimal value of variable in correct format
template <class t> string display(t,int=4);

int main()
{
    readLines();
    separateByHalt();
    processPostHalt();
    processPreHalt();
    createHexMap(); //Create HexMap
    createMemorySet();
    printMemory();
    printVariables();
    printHexMap(); //Print Hexmap
    callProcess();

    cout << "Final value of accumulator " << hexValue(accumulator);
}

template <class t>
string display(t n, int w)
{
    stringstream ss;
    ss << setw(w) << setfill('0') << n;
    return ss.str();
}

void callProcess()
{
    for(int i = 0;i<preHalt.size(); i++)
    {
        string s = preHalt[i];
        currentLine = s;
        AR = PC;
        cout << "PROGRAM COUNTER -> " << hexValue(PC) << endl;
        cout << "ADDRESS REGISTER -> " << hexValue(AR) << endl;
        PC++;
        IR = hexmap[AR];
        cout << "INSTRUCTION REGISTER -> " << display(IR) << endl;
        short int space = s.find(' ');
        string instruction = s.substr(0,space);
        if(space == string::npos) //space not found, CMA etc
            process(instruction,"");
        else
        {
            string varName = s.substr(space+1);
            AR = address[varName];
            if(instruction!="STA")
                DR = valueOf[varName];
            cout << "AR Updated! -> " << display(hexValue(AR)) << endl;
            cout << "DR Updated! -> " << display(hexValue(DR)) << endl;
            process(instruction, varName);
        }
        cout << endl;
    }

    cout << "PROGRAM COUNTER -> " << display(hexValue(PC)) << endl;
    AR = 1;
    cout << "AR  -> " << display(hexValue(AR),3) << endl << "IR -> " << IR << endl;

}

void printHexMap()
{
    cout << endl << "HexMap" << endl;
    for(map<int,string>::iterator kv=hexmap.begin();kv!=hexmap.end();kv++)
        cout << hexValue(kv->first) << " -> " << display(kv->second) << endl;
}

void printVariables()
{
    cout << endl << "Variables" << endl;
    for(map<int,string>::iterator p=variables.begin();p!=variables.end();p++)
        cout << hexValue(p->first) << " -> " << p->second << endl;
}

void printMemory()
{
    cout << "Memory Diagram :" << endl;
    for(set<pair<int,string> >::iterator p=memorySet.begin();p!=memorySet.end();p++)
    {
        short int key = p->first;
        string value = p->second;
        cout << value << " -> " << hexValue(key) << endl;
    }
}

void createMemorySet()
{
    for(multimap<string,int>::iterator kv = memory.begin();kv!=memory.end();kv++)
    {
        short int key = kv->second + org;
        string value = kv->first;
        pair<int,string> p = make_pair(key,value);
        memorySet.insert(p);
    }
}

void processPreHalt()
{
    short int i=0;
    for(int i = 0;i<preHalt.size(); i++)
    {
        string s = preHalt[i];
        short int space = s.find(' ');
        string instruction = s.substr(0,space);
        memory.insert(make_pair(instruction,i));
    }
}

void processPostHalt()
{
    short int halt = memory.find("HLT")->second;
    for(short int i = 0 ; i<postHalt.size();)
    {
        string s = postHalt[i];
        short int comma = s.find(',');
        short int firstSpace = s.find(' ');
        short int lastSpace = s.rfind(' ');
        string var = s.substr(0,comma);
        string type = s.substr(firstSpace+1,lastSpace-(firstSpace+1));
        string value = s.substr(lastSpace+1);
        stringstream oss(value);
        int x;
        oss >> x;
        valueOf[var] = x;
        //memory[var] = memory["HLT"] + (++i);

        memory.insert(make_pair(var,(halt + (++i))));
        variables[(org+i+halt)] = var;
        address[var] = (org+i+halt);
    }
}

void separateByHalt()
{
    short int i=0;
    while(lines[i]!="HLT")
        preHalt.push_back(lines[i++]);

    preHalt.push_back(lines[i]);
    memory.insert(make_pair("HLT",i++)); //memory["HLT"] = i++;

    while(lines[i]!="END")
        postHalt.push_back(lines[i++]);
}

void readLines()
{
    string tempInput;
    while(getline(fin,tempInput))
    {
        if(tempInput.find("ORG")!=string::npos) //This line is ORG
            org = hexToInt(tempInput.substr(tempInput.find(' ')+1));
        else if(tempInput[0]!='/')
            lines.push_back(tempInput);
    }
    PC = org;
}

void createHexMap()
{
    short int i = org;
    for(vector<string>::iterator it = preHalt.begin(); it!=preHalt.end(); it++)
    {
        string l = preHalt[it-preHalt.begin()];
        if(l.find(' ')==string::npos) //Single word
        {
            if(l=="CMA")
                hexmap[i] = "7200";
            else if(l=="INC")
                hexmap[i] = "7020";
            else if(l=="CLA")
                hexmap[i] = "7800";
            else if(l=="HLT")
                hexmap[i] = "7001";
        }
        else
        {
            short int space = l.find(' ');
            string instruction = l.substr(0,space);
            string postInstruction = l.substr(space+1);
            string value = hexValue(address[postInstruction]);

            if(instruction == "ADD")
                hexmap[i] = "1";
            else if(instruction == "LDA")
                hexmap[i] = "2";
            else if(instruction == "STA")
                hexmap[i] = "3";
            else if(instruction == "SUB")
                hexmap[i] = "A";
            else if(instruction == "MUL")
                hexmap[i] = "B";
            else if(instruction == "DIV")
                hexmap[i] = "C";
            else if(instruction == "XOR")
                hexmap[i] = "D";
            else if(instruction == "OR")
                hexmap[i] = "E";
            else if(instruction == "AND")
                hexmap[i] = "F";

            hexmap[i] += value;
        }
        i++;
    }

    for(vector<string>::iterator it = postHalt.begin(); it!=postHalt.end(); it++)
    {
        string l = postHalt[it-postHalt.begin()];
        short int comma = l.find(',');
        short int firstSpace = l.find(' ');
        short int lastSpace = l.rfind(' ');
        string var = l.substr(0,comma);
        string type = l.substr(firstSpace+1,lastSpace-(firstSpace+1));
        string value = l.substr(lastSpace+1);

        hexmap[i] = hexValue(valueOf[var]);
        i++;
    }
}

short int hexToInt(string s)
{
    stringstream ss;
    ss << s;
    short int value;
    ss >> hex >> value;
    return value;
}

string hexValue(short int n)
{
    stringstream ss;
    ss << uppercase << hex << n;
    return ss.str();
}

void process(string instruction, string postInstruction)
{
    if(instruction == "ORG")
        originate(postInstruction);
    else if(instruction == "LDA")
        load(valueOf[postInstruction]);
    else if(instruction == "STA")
        loadFromAcc(postInstruction);
    else if(instruction == "SUB")
        subtract(valueOf[postInstruction]);
    else if(instruction == "ADD")
        add(valueOf[postInstruction]);
    else if(instruction == "MUL")
        multiply(valueOf[postInstruction]);
    else if(instruction == "DIV")
        divide(valueOf[postInstruction]);
    else if(instruction == "XOR")
        bitwiseXOR(valueOf[postInstruction]);
    else if(instruction == "OR")
        bitwiseOR(valueOf[postInstruction]);
    else if(instruction == "AND")
        bitwiseAND(valueOf[postInstruction]);
    else if(instruction == "INC")
        add(1);
    else if(instruction == "CMA")
    {
        complement();
        AR = hexToInt("200");
    }
    else if(instruction == "CLA")
        load(0);
}

void loadFromAcc(string value)
{
    valueOf[value] = accumulator;
}

void load(short int value)
{
    accumulator = value;
}

void originate(string value)
{
    org = hexToInt(value);
}

void add(short int value)
{
    accumulator += value;
}

void subtract(short int value)
{
    accumulator -= value;
}

void multiply(short int value)
{
    accumulator *= value;
}

void divide(short int value)
{
    accumulator /= value;
}

void bitwiseXOR(short int value)
{
    accumulator ^= value;
}

void bitwiseOR(short int value)
{
    accumulator |= value;
}

void bitwiseAND(short int value)
{
    accumulator &= value;
}

void complement()
{
    accumulator = ~accumulator;
}

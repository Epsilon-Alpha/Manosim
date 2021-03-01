#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <map>
#include <set>
using namespace std;

int accumulator, org, IR, AR, DR, PC; //Registers
vector<string> lines, preHalt, postHalt; //Preprocessing
map<string,int> mp, address; //address["A"] -> Gives memory address of A, mp["A"] -> Gives value
map<int,string> variables, hexmap; //variables[100] -> Variable stored at that address
multimap<string,int> memory;
set<pair<int,string>> memorySet; //Contains (address,register) pair
ifstream fin("add.as");

void process(string,string);
void loadFromAcc(string);
void load(int);
void originate(string);
void add(int);
void subtract(int);
void multiply(int);
void divide(int);
void bitwiseXOR(int);
void bitwiseOR(int);
void bitwiseAND(int);
void complement();
int intToHex(string);
string hexValue(int);
void postProcessing();
void readLines();
void separateByHalt();
void processPostHalt();
void processPreHalt();
void createMemorySet();
void printMemory();
void printVariables();
void printHexMap();

int main()
{
    readLines();
    separateByHalt();
    processPostHalt();
    processPreHalt();
    createMemorySet();
    printMemory();
    printVariables();
    postProcessing();
    printHexMap();
}

void printHexMap()
{
    cout << endl << "HexMap" << endl;
    for(auto& kv: hexmap)
        cout << kv.first << " -> " << kv.second << endl;
}

void printVariables()
{
    cout << endl << "Variables" << endl;
    for(auto& p: variables)
        cout << hex << p.first << " -> " << p.second << endl;
}

void printMemory()
{
    cout << "Memory Diagram :" << endl;
    bool found = false;
    for(auto& p: memorySet)
    {
        int key = p.first;
        string value = p.second;
        if(found)
        {
            variables[key] = value;
            address[value] = key;
        }
        cout << hex << value << " -> " << key << endl;
        if(p.second=="HLT")
            found = true;
    }
}

void createMemorySet()
{
    for(auto& kv: memory)
    {
        int key = kv.second + org;
        string value = kv.first;
        pair<int,string> p = {key,value};
        memorySet.insert(p);
    }
}

void processPreHalt()
{
    int i=0;
    for(string s: preHalt)
    {
        int space = s.find(' ');
        string instruction = s.substr(0,space);
        if(space == string::npos)
            process(instruction,"");
        else
        {
            string postInstruction = s.substr(space+1);
            process(instruction, postInstruction);
        }
        memory.insert({instruction,i++});
    }
}

void processPostHalt()
{
    int i = 0, halt = memory.find("HLT")->second;
    for(string s: postHalt)
    {
        int comma = s.find(',');
        int firstSpace = s.find(' ');
        int lastSpace = s.rfind(' ');
        string var = s.substr(0,comma);
        string type = s.substr(firstSpace+1,lastSpace-(firstSpace+1));
        string value = s.substr(lastSpace+1);
        mp[var] = stoi(value);
        //memory[var] = memory["HLT"] + (++i);
        memory.insert({var,(halt + (++i))});
    }
}

void separateByHalt()
{
    int i=0;
    while(lines[i]!="HLT")
        preHalt.push_back(lines[i++]);

    memory.insert({"HLT",i++}); //memory["HLT"] = i++;

    while(lines[i]!="END")
        postHalt.push_back(lines[i++]);
}

void readLines()
{
    string tempInput;
    while(getline(fin,tempInput))
    {
        if(tempInput.find("ORG")!=string::npos) //This line is ORG
            org = intToHex(tempInput.substr(tempInput.find(' ')+1));
        else if(tempInput[0]!='/')
            lines.push_back(tempInput);
    }
}

void postProcessing()
{
    int i = org;
    for(string l: preHalt)
    {
        if(l.find(' ')==string::npos) //Single word
        {
            if(l=="CMA")
                hexmap[i] = "7200";
            else if(l=="INC")
                hexmap[i] = "7020";
            else if(l=="CLA")
                hexmap[i] = "7800";
        }
        else
        {
            int space = l.find(' ');
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

            hexmap[i]+=value;
        }
        i++;
    }

    hexmap[i] = "7001"; //HLT
    i++;

    for(string l: postHalt)
    {
        int comma = l.find(',');
        int firstSpace = l.find(' ');
        int lastSpace = l.rfind(' ');
        string var = l.substr(0,comma);
        string type = l.substr(firstSpace+1,lastSpace-(firstSpace+1));
        string value = l.substr(lastSpace+1);

        hexmap[i] = hexValue(mp[var]);
        i++;
    }
}

int intToHex(string s)
{
    stringstream ss;
    ss << s;
    int value;
    ss >> hex >> value;
    return value;
}

string hexValue(int n)
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
        load(mp[postInstruction]);
    else if(instruction == "STA")
        loadFromAcc(postInstruction);
    else if(instruction == "SUB")
        subtract(mp[postInstruction]);
    else if(instruction == "ADD")
        add(mp[postInstruction]);
    else if(instruction == "MUL")
        multiply(mp[postInstruction]);
    else if(instruction == "DIV")
        divide(mp[postInstruction]);
    else if(instruction == "XOR")
        bitwiseXOR(mp[postInstruction]);
    else if(instruction == "OR")
        bitwiseOR(mp[postInstruction]);
    else if(instruction == "AND")
        bitwiseAND(mp[postInstruction]);
    else if(instruction == "INC")
        add(1);
    else if(instruction == "CMA")
        complement();
    else if(instruction == "CLA")
        load(0);
}

void loadFromAcc(string value)
{
    mp[value] = accumulator;
}

void load(int value)
{
    accumulator = value;
}

void originate(string value)
{
    org = intToHex(value);
}

void add(int value)
{
    accumulator += value;
}

void subtract(int value)
{
    accumulator -= value;
}

void multiply(int value)
{
    accumulator *= value;
}

void divide(int value)
{
    accumulator /= value;
}

void bitwiseXOR(int value)
{
    accumulator ^= value;
}

void bitwiseOR(int value)
{
    accumulator |= value;
}

void bitwiseAND(int value)
{
    accumulator &= value;
}

void complement()
{
    accumulator = ~accumulator;
}

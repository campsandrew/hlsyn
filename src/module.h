/***********************************************************************************
 * File: module.h
 * Author: Jason Tran, Andrew Camps
 * NetID: ichikasuto, andrewcamps
 * Date: November 17, 2017
 *
 * Description:
 *
 **********************************************************************************/

#ifndef module_h
#define module_h

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>

#include "input.h"
#include "output.h"
#include "variable.h"

using namespace std;

enum Type {
    INPUT_TYPE,
    OUTPUT_TYPE,
    VARIABLE_TYPE,
    OPERATION_TYPE,
    IFELSE_TYPE,
    FORLOOP_TYPE
};

class Module {
private:
    string name;
    bool readFile(string file);
    bool getDataType(string type, int *size);
    bool parseLine(vector<string> line);
    int getID(Operations operation);
    vector<string> split(string const &input);
public:
    Module(string name);
    string getName() { return name; }
    bool build_module(string file) { return readFile(file) ? true : false; }    /* Builds the data path graph */
    bool output_module(string file);                                            /* Prints the module in .v format */
    vector<Operation *> operations;
    vector<Output *> outputs;
    vector<Input *> inputs;
    vector<Variable *> variables;
};

#endif /* module_h */

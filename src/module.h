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
#include "node.h"
#include "type.h"

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
    int Latency;
    bool readFile(string file);
    bool getDataType(string type, int *size);
    bool parseLine(vector<string> line);
    int getID(Operations operation);
    vector<string> split(string const &input);
    bool scheduleOperations();
    bool getTimeFrames(vector<Operation *> nodes);
    void resetOutCycles();
    bool getALAPTimes(vector<Operation *> nodes);
    bool getASAPTimes(vector<Operation *> nodes);
    void getTypePropabilities();
    void getTotalForces(vector<Operation *> nodes);
    void getSelfForces(vector<Operation *> nodes);
    void getPredecessorForces();
    void getSuccessorForces();
    void scheduleNode(vector<Operation *> &nodes);
public:
    Module(string name, int latency);
    string getName() { return name; }
    int getLatency() { return Latency; }
    bool build_module(string file);    /* Builds the data path graph */
    bool output_module(string file);   /* Prints the module in .v format */
    vector<Operation *> operations;
    vector<Output *> outputs;
    vector<Input *> inputs;
    vector<Variable *> variables;
    vector<double> sum_AddSub;
    vector<double> sum_Mul;
    vector<double> sum_Logic;
    vector<double> sum_DivMod;
    bool prevIf;
    int ifIndex;
    vector<Input *> _inputs;
    vector<Output *> _outputs;
    vector<Variable *> _variables;
    vector<node *> nodes;
    
};

#endif /* module_h */

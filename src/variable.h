/***********************************************************************************
 * File: variable.h
 * Author: Jason Tran, Andrew Camps
 * NetID: ichikasuto, andrewcamps
 * Date: November 17, 2017
 *
 * Description: This defines the variable object for connecting inputs and outputs of nodes
 *
 **********************************************************************************/

#ifndef variable_h
#define variable_h

#include <string>
#include <vector>

#include "operation.h"

using namespace std;

class Operation;

class Variable {
private:
    bool usigned;
    int dataWidth;
    string name;
public:
    Variable(string name, int width, bool usigned);
    string getName() { return name; }
    int getWidth() { return dataWidth; }
    bool isUnsigned() { return usigned; }
    string toString();
    Operation *fromOperation = NULL;
    Operation *fromIfOp = NULL;
    vector<Operation *> toOperations;
    int outCycle = 0;
    int permOutCycle = 0;
    bool isScheduled = false;
};

#endif /* variable_h */

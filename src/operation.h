/***********************************************************************************
 * File: operation.h
 * Author: Jason Tran, Andrew Camps
 * NetID: ichikasuto, andrewcamps
 * Date: November 17, 2017
 *
 * Description:
 *
 **********************************************************************************/

#ifndef operation_h
#define operation_h

#include "output.h"
#include "input.h"
#include "variable.h"

#define NUM_INPUTS 3
#define INTERVAL_MAX 1
#define INTERVAL_MIN 0
#define NO_FORCE -1

using namespace std;

enum Operations {
    ADD,
    SUB,
    MUL,
    COMP_GT,
    COMP_LT,
    COMP_EQ,
    MUX2x1,
    SHR,
    SHL,
    DIV,
    MOD,
    INC,
    DEC
};

class Variable;
class Input;
class Output;

class Operation {
private:
    Operations operation;
    string sign;
    bool usigned;
    int operationID;
    int width;
public:
    Operation();
    Operations getOperation() { return operation; }
    void setOperation(Operations op) {operation = op; }
    int getOpID() { return operationID; }
    int getCycleDelay();
    void setOpID(int ID) { operationID = ID; }
    void calcWidth();
    void setSign();
    bool isUnsigned() { return usigned; }
    string toString();
    Output *outNext;
    Variable *varNext;
    Variable *inVar[NUM_INPUTS];
    Input *inInput[NUM_INPUTS];
    int timeASAP = -1;
    int timeALAP = -1;
    std::vector<double> operationProbability;
    std::vector<double> selfForce;
    std::vector<double> predecessorForce;
    std::vector<double> sucessorForce;
    std::vector<double> totalForce;
};

#endif /* operation_h */

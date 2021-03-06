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
#define NO_FORCE -999

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
    IF,
    ELSE,
    DEC
};

typedef struct TimeFrame {
    int min = 0;
    int max = 0;
    int getWidth(){
        return (max - min) + 1;
    }
} TimeFrame;

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
    vector<Operation *> inOperations; /* Operations (mainly if statements) that precede current operation */
    vector<Operation *> nextIf; /* Operations that are executed within an if statement operation */
    vector<Operation *> nextElse; /* Operations that are executed within an else statement operation */
    TimeFrame frame;
    int tempTime = 0;
    int scheduledTime = 0;
    double totalForce = 0;
    bool conditional = false;
    vector<double> selfForces;
    vector<double> predecessorForces;
    vector<double> sucessorForces;
};

#endif /* operation_h */

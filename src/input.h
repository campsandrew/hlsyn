/***********************************************************************************
 * File: input.h
 * Author: Jason Tran, Andrew Camps
 * NetID: ichikasuto, andrewcamps
 * Date: November 17, 2017
 *
 * Description:
 *
 **********************************************************************************/

#ifndef input_h
#define input_h

#include <string>
#include <vector>

#include "operation.h"

using namespace std;

class Operation;

class Input {
private:
    bool usigned;
    int dataWidth;
    string name;
public:
    Input(string name, int width, bool usigned);
    string getName() { return name; }
    int getWidth() { return dataWidth; }
    bool isUnsigned() { return usigned; }
    string toString();
    vector<Operation *> toOperations;
    int outCycle = -1;
};

#endif /* input_h */

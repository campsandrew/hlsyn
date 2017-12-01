/***********************************************************************************
 * File: output.h
 * Author: Jason Tran, Andrew Camps
 * NetID: ichikasuto, andrewcamps
 * Date: November 17, 2017
 *
 * Description:
 *
 **********************************************************************************/

#ifndef output_h
#define output_h

#include <string>

#include "operation.h"

using namespace std;

class Operation;

class Output {
private:
    bool usigned;
    int dataWidth;
    string name;
public:
    Output(string name, int width, bool usigned);
    string getName() { return name; }
    int getWidth() { return dataWidth; }
    bool isUnsigned() { return usigned; }
    string toString();
    Operation *fromOperation = NULL;
    int outCycle = 0;
    bool isScheduled = false;
};

#endif /* output_h */

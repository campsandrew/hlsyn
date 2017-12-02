/***********************************************************************************
 * File: ifelse.h
 * Author: Jason Tran, Andrew Camps
 * NetID: ichikasuto, andrewcamps
 * Date: November 29, 2017
 *
 * Description: This defines the if statement operation. This stores the graph inside of an ifelse statement
 *
 **********************************************************************************/

#ifndef ifelse_h
#define ifelse_h

#include "vector"

#include "operation.h"
#include "variable.h"
#include "input.h"

class Variable;
class Input;

class IfElse {
private:
public:
    Variable *varCondition = NULL;
    Input *inputCondition = NULL;
    bool inElseBlock = false;
    vector<Operation *> ifOperations;
    vector<Operation *> elseOperations;
    
    int ifEndTime = 0;
    int elseEndTime = 0;
};

#endif /* ifelse_h */

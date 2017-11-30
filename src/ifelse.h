/***********************************************************************************
 * File: ifelse.h
 * Author: Jason Tran, Andrew Camps
 * NetID: ichikasuto, andrewcamps
 * Date: November 29, 2017
 *
 * Description:
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
    Variable *varCondition;
    Input *inputCondition;
    bool inElseBlock = false;
    vector<Operation *> ifOperations;
    vector<Operation *> elseOperations;
    
    int ifTimeSchedule = 0;
    int ifEndTime = 0;
    int elseTimeSchedule = 0;
    int elseEndTime = 0;
};

#endif /* ifelse_h */

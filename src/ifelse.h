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
};

#endif /* ifelse_h */

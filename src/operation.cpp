/***********************************************************************************
 * File: operation.cpp
 * Author: Jason Tran, Andrew Camps
 * NetID: ichikasuto, andrewcamps
 * Date: November 17, 2017
 *
 * Description:
 *
 **********************************************************************************/

#include "operation.h"

/**
 * Constructor
 */
Operation::Operation(){
    for(int i = 0; i < NUM_INPUTS; i++){
        inVar[i] = NULL;
        inInput[i] = NULL;
    }
    
    outNext = NULL;
    varNext = NULL;
}

/**
 * Gets the width parameter of an operation based on various parameters layed out in the spec sheet.
 * All operations use the width of the output wire except comparator type operation
 */
void Operation::calcWidth(){
    int width0;
    int width1;
    
    if(operation == COMP_GT || operation == COMP_LT || operation == COMP_EQ){
        width0 = inVar[0] == NULL ? inInput[0]->getWidth() : inVar[0]->getWidth();
        width1 = inVar[1] == NULL ? inInput[1]->getWidth() : inVar[1]->getWidth();
        this->width = width0 > width1 ? width0 : width1;
    }else{
        this->width = outNext == NULL ? varNext->getWidth() : outNext->getWidth();
    }
}

/**
 * Sets the sign of the operation type based on the input wires to operation
 */
void Operation::setSign() {
    bool usigned = true;
    
    for(int i = 0; i < NUM_INPUTS; i++){
        if(inVar[i] != NULL){
            if(!inVar[i]->isUnsigned()){
                usigned = false;
                break;
            }
        }
        if(inInput[i] != NULL){
            if(!inInput[i]->isUnsigned()){
                usigned = false;
                break;
            }
        }
    }
    
    if(outNext != NULL){
        if(!outNext->isUnsigned())
            usigned = false;
    }
    if(varNext != NULL){
        if(!varNext->isUnsigned()){
            usigned = false;
        }
    }
    
    this->usigned = usigned;
    sign = usigned ? "" : "S";
}

int Operation::getCycleDelay(){
    switch(operation){
        case ADD:
        case SUB:
        case COMP_GT:
        case COMP_LT:
        case COMP_EQ:
        case MUX2x1:
        case SHR:
        case SHL:
        case INC:
        case DEC:
            return 1;
        case MUL:
            return 2;
        case DIV:
        case MOD:
            return 3;
        case IFELSE:
            /* Do nothing */
            break;
    }
    
    return 0;
}

/**
 * To string method that outputs each operation in verilog syntax with correct bit sizes for inputs and outputs as well
 * as mix sign types
 */
string Operation::toString(){
    string in[NUM_INPUTS];
    string out;
    
    if(this->varNext != NULL){
        out = this->varNext->getName();
    }else{
        out = this->outNext->getName();
    }
    
    for(int i = 0; i < NUM_INPUTS; i++){
        if(inVar[i] != NULL){
             in[i] = inVar[i]->getName();
        }else{
            if(inInput[i] != NULL){
                in[i] = inInput[i]->getName();
            }
        }
    }
    
    /* Printing operations */
    switch(operation){
        case INC:
            return out + " <= " + in[0] + " + 1;";
        case DEC:
            return out + " <= " + in[0] + " - 1;";
        case ADD:
            return out + " <= " + in[0] + " + " + in[1] + ";";
        case SUB:
            return out + " <= " + in[0] + " - " + in[1] + ";";
        case MUL:
            return out + " <= " + in[0] + " * " + in[1] + ";";
        case COMP_GT:
            return out + " <= " + in[0] + " > " + in[1] + ";";
        case COMP_LT:
            return out + " <= " + in[0] + " < " + in[1] + ";";
        case COMP_EQ:
            return out + " <= " + in[0] + " == " + in[1] + ";";
        case SHR:
            return out + " <= " + in[0] + " >> " + in[1] + ";";
        case SHL:
            return out + " <= " + in[0] + " << " + in[1] + ";";
        case DIV:
            return out + " <= " + in[0] + " / " + in[1] + ";";
        case MOD:
            return out + " <= " + in[0] + " % " + in[1] + ";";
        case MUX2x1:
            return out + " <= " + in[0] + " ? " + in[1] + " : " + in[2] + ";";
        case IFELSE:
            string cond = "";
//            if(this->ifelse->inputCondition != NULL){
//                cond += this->ifelse->inputCondition;
//            }else{
//                cond += this->ifelse->varCondition;
//            }
//
//            string print = "if (" + cond + ")\n"
            return cond;//print;
    }
    
    return NULL;
}

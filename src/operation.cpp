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
    }
    
    return 0;
}

/**
 * To string method that outputs each operation in verilog syntax with correct bit sizes for inputs and outputs as well
 * as mix sign types
 */
string Operation::toString(){
    Variable *outV = NULL;
    Output *out = NULL;
    Input *in0 = NULL;
    Variable *in0v = NULL;
    Input *in1 = NULL;
    Variable *in1v = NULL;
    Input *in2 = NULL;
    Variable *in2v = NULL;
    string outName;
    string in0n;
    string in1n;
    string in2n;
    
    /* Input/Output initialization */
    if(outNext == NULL){
        outV = varNext;
    }else{
        out = outNext;
    }
    if(inVar[0] == NULL){
        in0 = inInput[0];
    }else{
        in0v = inVar[0];
    }
    if(inVar[1] == NULL){
        if(inInput[1] != NULL){
            in1 = inInput[1];
        }
    }else{
        in1v = inVar[1];
    }
    if(inVar[2] == NULL){
        if(inInput[2] != NULL){
            in2 = inInput[2];
        }
    }else{
        in2v = inVar[2];
    }
    
    /* Getting proper bit widths and sign types */
    outName = out == NULL ? outV->getName() : out->getName();
    in0n = in0 == NULL ? in0v->getName() : in0->getName();
    if(in0v != NULL){
        if(in0v->getWidth() == width){
            if(!this->isUnsigned() && in0v->isUnsigned()){
                in0n = "$signed(" + in0n + ")";
            }
        }else if(in0v->getWidth() > width){
            if(!this->isUnsigned() && in0v->isUnsigned()){
                in0n = "$signed(" + in0n + "[" + to_string(width - 1) + ":0])";
            }else{
                in0n = in0n + "[" + to_string(width - 1) + ":0]";
            }
        }else{
            if(!in0v->isUnsigned()){
                in0n = "{{" + to_string(width - in0v->getWidth()) + "{" + in0n + "[" + to_string(in0v->getWidth() - 1) + "]" + "}}, " + in0n + "}";
            }else{
                if(!this->isUnsigned()){
                    in0n = "$signed({" + to_string(width - in0v->getWidth()) + "'b0, " + in0n + "})";
                }else{
                    in0n = "{" + to_string(width - in0v->getWidth()) + "'b0, " + in0n + "}";
                }
            }
        }
    }else{
        if(in0->getWidth() == width){
            if(!this->isUnsigned() && in0->isUnsigned()){
                in0n = "$signed(" + in0n + ")";
            }
        }else if(in0->getWidth() > width){
            if(!this->isUnsigned() && in0->isUnsigned()){
                in0n = "$signed(" + in0n + "[" + to_string(width - 1) + ":0])";
            }else{
                in0n = in0n + "[" + to_string(width - 1) + ":0]";
            }
        }else{
            if(!in0->isUnsigned()){
                in0n = "{{" + to_string(width - in0->getWidth()) + "{" + in0n + "[" + to_string(in0->getWidth() - 1) + "]" + "}}, " + in0n + "}";
            }else{
                if(!this->isUnsigned()){
                    in0n = "$signed({" + to_string(width - in0->getWidth()) + "'b0, " + in0n + "})";
                }else{
                    in0n = "{" + to_string(width - in0->getWidth()) + "'b0, " + in0n + "}";
                }
            }
        }
    }
    
    if(operation != INC && operation != DEC && operation != SHL && operation != SHR){
        in1n = in1 == NULL ? in1v->getName() : in1->getName();
        if(in1v != NULL){
            if(in1v->getWidth() == width){
                if(!this->isUnsigned() && in1v->isUnsigned()){
                    in1n = "$signed(" + in1n + ")";
                }
            }else if(in1v->getWidth() > width){
                if(!this->isUnsigned() && in1v->isUnsigned()){
                    in1n = "$signed(" + in1n + "[" + to_string(width - 1) + ":0])";
                }else{
                    in1n = in1n + "[" + to_string(width - 1) + ":0]";
                }
            }else{
                if(!in1v->isUnsigned()){
                    in1n = "{{" + to_string(width - in1v->getWidth()) + "{" + in1n + "[" + to_string(in1v->getWidth() - 1) + "]" + "}}, " + in1n + "}";
                }else{
                    if(!this->isUnsigned()){
                        in1n = "$signed({" + to_string(width - in1v->getWidth()) + "'b0, " + in1n + "})";
                    }else{
                        in1n = "{" + to_string(width - in1v->getWidth()) + "'b0, " + in1n + "}";
                    }
                }
            }
        }else{
            if(in1->getWidth() == width){
                if(!this->isUnsigned() && in1->isUnsigned()){
                    in1n = "$signed(" + in1n + ")";
                }
            }else if(in1->getWidth() > width){
                if(!this->isUnsigned() && in1->isUnsigned()){
                    in1n = "$signed(" + in1n + "[" + to_string(width - 1) + ":0])";
                }else{
                    in1n = in1n + "[" + to_string(width - 1) + ":0]";
                }
            }else{
                if(!in1->isUnsigned()){
                    in1n = "{{" + to_string(width - in1->getWidth()) + "{" + in1n + "[" + to_string(in1->getWidth() - 1) + "]" + "}}, " + in1n + "}";
                }else{
                    if(!this->isUnsigned()){
                        in1n = "$signed({" + to_string(width - in1->getWidth()) + "'b0, " + in1n + "})";
                    }else{
                        in1n = "{" + to_string(width - in1->getWidth()) + "'b0, " + in1n + "}";
                    }
                }
            }
        }
    }
    
    /* Printing operations */
    switch(operation){
        case INC:
            return "\t" + sign + "INC #(" + to_string(width) + ") inc" + to_string(getOpID()) + "(" + in0n + ", " + outName + ");";
        case DEC:
            return "\t" + sign + "DEC #(" + to_string(width) + ") dec" + to_string(getOpID()) + "(" + in0n + ", " + outName + ");";
        case ADD:
            return "\t" + sign + "ADD #(" + to_string(width) + ") add" + to_string(getOpID()) + "(" + in0n + ", " + in1n + ", " + outName + ");";
        case SUB:
            return "\t" + sign + "SUB #(" + to_string(width) + ") sub" + to_string(getOpID()) + "(" + in0n + ", " + in1n + ", " + outName + ");";
        case MUL:
            return "\t" + sign + "MUL #(" + to_string(width) + ") mul" + to_string(getOpID()) + "(" + in0n + ", " + in1n + ", " + outName + ");";
        case COMP_GT:
            outName = out == NULL ? outV->getName() : out->getName();
            if(outV != NULL){
                if(outV->getWidth() > 1){
                    outName = outName + "[0]";
                }
            }else{
                if(out->getWidth() > 1){
                    outName = outName + "[0]";
                }
            }
            return "\t" + sign + "COMP #(" + to_string(width) + ") comp_gt" + to_string(getOpID()) + "(" + in0n + ", " + in1n + ", " + outName + ", , " + ");";
        case COMP_LT:
            outName = out == NULL ? outV->getName() : out->getName();
            if(outV != NULL){
                if(outV->getWidth() > 1){
                    outName = outName + "[0]";
                }
            }else{
                if(out->getWidth() > 1){
                    outName = outName + "[0]";
                }
            }
            return "\t" + sign + "COMP #(" + to_string(width) + ") comp_lt" + to_string(getOpID()) + "(" + in0n + ", " + in1n + ", , " + outName + ", " + ");";
        case COMP_EQ:
            outName = out == NULL ? outV->getName() : out->getName();
            if(outV != NULL){
                if(outV->getWidth() > 1){
                    outName = outName + "[0]";
                }
            }else{
                if(out->getWidth() > 1){
                    outName = outName + "[0]";
                }
            }
            return "\t" + sign + "COMP #(" + to_string(width) + ") comp_eq" + to_string(getOpID()) + "(" + in0n + ", " + in1n + ", , , " + outName + ");";
        case SHR:
            in1n = in1 == NULL ? in1v->getName() : in1->getName();
            return "\t" + sign + "SHR #(" + to_string(width) + ") shr" + to_string(getOpID()) + "(" + in0n + ", " + in1n + ", " + outName + ");";
        case SHL:
            in1n = in1 == NULL ? in1v->getName() : in1->getName();
            return "\t" + sign + "SHL #(" + to_string(width) + ") shl" + to_string(getOpID()) + "(" + in0n + ", " + in1n + ", " + outName + ");";
        case DIV:
            return "\t" + sign + "DIV #(" + to_string(width) + ") div" + to_string(getOpID()) + "(" + in0n + ", " + in1n + ", " + outName + ");";
        case MOD:
            return "\t" + sign + "MOD #(" + to_string(width) + ") mod" + to_string(getOpID()) + "(" + in0n + ", " + in1n + ", " + outName + ");";
        case MUX2x1:
            in0n = in0 == NULL ? in0v->getName() : in0->getName();
            if(in0v != NULL){
                if(in0v->getWidth() > 1){
                    in0n = in0n + "[0]";
                }
            }else{
                if(in0->getWidth() > 1){
                    in0n = in0n + "[0]";
                }
            }
            in2n = in2 == NULL ? in2v->getName() : in2->getName();
            if(in2v != NULL){
                if(in2v->getWidth() == width){
                    if(!this->isUnsigned() && in2v->isUnsigned()){
                        in2n = "$signed(" + in2n + ")";
                    }
                }else if(in2v->getWidth() > width){
                    if(!this->isUnsigned() && in2v->isUnsigned()){
                        in2n = "$signed(" + in2n + "[" + to_string(width - 1) + ":0])";
                    }else{
                        in2n = in2n + "[" + to_string(width - 1) + ":0]";
                    }
                }else{
                    if(!in2v->isUnsigned()){
                        in2n = "{{" + to_string(width - in2v->getWidth()) + "{" + in2n + "[" + to_string(in2v->getWidth() - 1) + "]" + "}}, " + in2n + "}";
                    }else{
                        if(!this->isUnsigned()){
                            in2n = "$signed({" + to_string(width - in2v->getWidth()) + "'b0, " + in2n + "})";
                        }else{
                            in2n = "{" + to_string(width - in2v->getWidth()) + "'b0, " + in2n + "}";
                        }
                    }
                }
            }else{
                if(in2->getWidth() == width){
                    if(!this->isUnsigned() && in2->isUnsigned()){
                        in2n = "$signed(" + in2n + ")";
                    }
                }else if(in2->getWidth() > width){
                    if(!this->isUnsigned() && in2->isUnsigned()){
                        in2n = "$signed(" + in2n + "[" + to_string(width - 1) + ":0])";
                    }else{
                        in2n = in2n + "[" + to_string(width - 1) + ":0]";
                    }
                }else{
                    if(!in2->isUnsigned()){
                        in2n = "{{" + to_string(width - in2->getWidth()) + "{" + in2n + "[" + to_string(in2->getWidth() - 1) + "]" + "}}, " + in2n + "}";
                    }else{
                        if(!this->isUnsigned()){
                            in2n = "$signed({" + to_string(width - in2->getWidth()) + "'b0, " + in2n + "})";
                        }else{
                            in2n = "{" + to_string(width - in2->getWidth()) + "'b0, " + in2n + "}";
                        }
                    }
                }
            }
            return "\t" + sign + "MUX2x1 #(" + to_string(width) + ") mux" + to_string(getOpID()) + "(" + in0n + ", " + in1n + ", " + in2n + ", " + outName + ");";
    }
    
    return NULL;
}

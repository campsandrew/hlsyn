/***********************************************************************************
 * File: input.cpp
 * Author: Jason Tran, Andrew Camps
 * NetID: ichikasuto, andrewcamps
 * Date: November 17, 2017
 *
 * Description: Various functions to help create the input connects of the graph and print out the operations to the verilog file
 *
 **********************************************************************************/

#include "input.h"

/**
 * Constructor
 */
Input::Input(string name, int width, bool usigned){
    this->name = name;
    this->dataWidth = width;
    this->usigned = usigned;
}

/**
 * toString method that returns a string in format of verilog syntax
 */
string Input::toString(){
    string width = "";
    string sign = "";
    if(this->dataWidth > 1){
        width = "[" + to_string(this->dataWidth - 1) + ":0] ";
    }
    
    if(!this->isUnsigned()){
        sign = "signed ";
    }
    
    return "\tinput " + sign + width + this->name + ";";
}

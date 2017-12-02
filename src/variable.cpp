/***********************************************************************************
 * File: variable.cpp
 * Author: Jason Tran, Andrew Camps
 * NetID: ichikasuto, andrewcamps
 * Date: November 17, 2017
 *
 * Description: Various functions to help create the connects of the graph and print out the operations to the verilog file
 *
 **********************************************************************************/

#include "variable.h"

/**
 * Constructor
 */
Variable::Variable(string name, int width, bool usigned){
    this->name = name;
    this->dataWidth = width;
    this->usigned = usigned;
}

/**
 * To string method outputs verilog code syntax
 */
string Variable::toString(){
    string width = "";
    string sign = "";
    if(this->dataWidth > 1){
        width = "[" + to_string(this->dataWidth - 1) + ":0] ";
    }
    
    if(!this->isUnsigned()){
        sign = "signed ";
    }
    
    return "\treg " + sign + width + this->name + ";";
}

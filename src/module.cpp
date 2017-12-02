/***********************************************************************************
 * File: module.cpp
 * Author: Jason Tran, Andrew Camps
 * NetID: ichikasuto, andrewcamps
 * Date: November 17, 2017
 *
 * Description: This file creates the graph and outputs the code to the verilog file specified
 *
 **********************************************************************************/

#include "module.h"

/**
 * Constructor
 */
Module::Module(string name, int latency){
    this->name = name;
    this->Latency = latency;
}

bool Module::build_module(string file) {
    int status = readFile(file) ? true : false;
    if(!status){
        return status;
    }
    status = scheduleOperations(operations, 0, Latency);
    
    return status;
}

/**
 * Reads a file line by line
 */
bool Module::readFile(string file) {
    fstream in;
    
    in.open(file.c_str());
    
    if (!in.is_open()) {     // if file is available, open and read
        cout << "No input file found with the name: " << file << endl;
        return false;
    }
    
    /* Define Clk and Rst for all modules */
    this->inputs.push_back(new Input("Clk", 1, true));
    this->inputs.push_back(new Input("Rst", 1, true));
    this->inputs.push_back(new Input("Start", 1, true));
    this->outputs.push_back(new Output("Done", 1, true));
    
    while (!in.eof()) {     // execute until last line of file
        string line;
        getline(in, line);
        
        /* Removes all comments from line */
        string::size_type index;
        if ((index = line.find("//")) != string::npos) {
            line.erase(line.begin() + index, line.end());
        }
        
        /* Parse line */
        if(!parseLine(split(line))){
            cout << "Invalid line: " << line << endl;
            return false;
        }
    }
    
    in.close();
    
    return true;
}

/**
 * Parses the input .c files and creates the graph structure
 */
bool Module::parseLine(vector<string> line) {
    Type lineType;
    
    /* Skip empty empty lines */
    if(!line.size()) return true;
    
    /* Parse first word of each line */
    string type = line.front();
    if(type.compare("input") == 0){
        line.erase(line.begin());
        lineType = INPUT_TYPE;
    } /* Output */
    else if(type.compare("output") == 0){
        line.erase(line.begin());
        lineType = OUTPUT_TYPE;
    } /* Variable */
    else if(type.compare("variable") == 0){
        line.erase(line.begin());
        lineType = VARIABLE_TYPE;
    } /* If */
    else if(type.compare("if") == 0){
        line.erase(line.begin());
        lineType = IF_TYPE;
    } /* Else */
    else if(type.compare("else") == 0){
        line.erase(line.begin());
        lineType = ELSE_TYPE;
    }
    else if(type.compare("}") == 0){
        line.erase(line.begin());
        if(line.size() != 0){
            cout << "ERROR: Invalid if else bracket syntax" << endl;
            return false;
        }
        lineType = BRACKET_TYPE;
    }/* For loop */
    else if(type.compare("for") == 0){
        line.erase(line.begin());
        lineType = FORLOOP_TYPE;
    } /* Operation or invalid line  */
    else {
        /* Checks for invalid line */
        if(line.at(1).compare("=") != 0){
            return false;
        }
        
        lineType = OPERATION_TYPE;
    }
    
    /* Performs correct line parsing technique for each line type */
    int size;
    if(lineType == INPUT_TYPE || lineType == OUTPUT_TYPE || lineType == VARIABLE_TYPE) {
        
        /* Invalid line */
        if(!line.size()){
            return false;
        }
        
        /* Get the sign type and size of variable */
        bool usigned = getDataType(line.front(), &size);
        if(size < 0){
            cout << "Invalid data type: " << line.front() << endl;
            return false;
        }
        
        /* Invalid line */
        line.erase(line.begin());
        if(!line.size()){
            return false;
        }
        
        /* Loop until end of string */
        while(line.size() > 0){
            string name = line.front();
            
            /* Remove commas from variable list names */
            string::size_type index;
            if((index = name.find(',')) != string::npos){
                name.erase(name.begin() + index, name.end());
            }
            
            /* Create specific object type */
            switch(lineType){
                case INPUT_TYPE:
                    this->inputs.push_back(new Input(name, size, usigned));
                    break;
                case OUTPUT_TYPE:
                    this->outputs.push_back(new Output(name, size, usigned));
                    break;
                case VARIABLE_TYPE:
                    this->variables.push_back(new Variable(name, size, usigned));
                    break;
                default:
                    /* do nothing */
                    break;
            }
            
            line.erase(line.begin());
        }
    } else if(lineType == IF_TYPE) {
        
        /* Invalid line */
        if(!line.size()){
            cout << "ERROR: Invalid if statement syntax" << endl;
            return false;
        }
        /* Invalid line */
        line.erase(line.begin());
        if(!line.size()){
            cout << "ERROR: Invalid if statement syntax" << endl;
            return false;
        }
        
        /* Create new ifelse and operation objects */
        IfElse *newIfElse = new IfElse();
        Operation *newOp = new Operation();
        newOp->setOperation(IFELSE);
        newOp->ifelse = newIfElse;
        
        /* Gets parents if statement */
        if(openBlocks.size() != 0){
            newOp->inIfElse = true;
            newOp->parent = openBlocks.front();
        }
        
        /* Assign condition statement to ifelse operation */
        bool assigned = false;
        string condition = line.front();
        for(int i = 0; i < (signed)this->inputs.size(); i++){
            if(condition.compare(inputs.at(i)->getName()) == 0){
                newIfElse->inputCondition = inputs.at(i);
                assigned = true;
                break;
            }
        }
        if(!assigned){
            for(int i = 0; i < (signed)this->variables.size(); i++){
                if(condition.compare(variables.at(i)->getName()) == 0){
                    newIfElse->varCondition = variables.at(i);
                    assigned = true;
                    break;
                }
            }
        }
        
        /* No assignment was defined */
        if(!assigned){
            cout << "ERROR: Missing condtion for if statement: " << condition << endl;
            return false;
        }
        
        /* Invalid line */
        line.erase(line.begin());
        if(!line.size()){
            cout << "ERROR: Invalid if statement syntax" << endl;
            return false;
        }
        /* Invalid line */
        line.erase(line.begin());
        if(!line.size()){
            cout << "ERROR: Invalid if statement syntax" << endl;
            return false;
        }
        
        /* For nested if statements */
        if(!this->openBlocks.size()){
            /* Add all variables the if statement is dependent on */
            for(auto &i : operations){
                if(i->varNext != NULL){
                    bool added = false;
                    for(auto &j : newOp->incomingVars){
                        if(j->getName().compare(i->varNext->getName()) == 0){
                            added = true;
                            break;
                        }
                    }
                    
                    if(!added){
                        newOp->incomingVars.push_back(i->varNext);
                        i->varNext->toOperations.push_back(newOp);
                    }
                }
            }
            this->operations.push_back(newOp);
        }else{
            /* Add all variables the if statement is dependent on */
            for(auto &i : openBlocks.front()->ifelse->ifOperations){
                if(i->varNext != NULL){
                    bool added = false;
                    for(auto &j : newOp->incomingVars){
                        if(j->getName().compare(i->varNext->getName()) == 0){
                            added = true;
                            break;
                        }
                    }
                    
                    if(!added){
                        newOp->incomingVars.push_back(i->varNext);
                        i->varNext->toOperations.push_back(newOp);
                    }
                }
            }
            for(auto &i : openBlocks.front()->ifelse->elseOperations){
                if(i->varNext != NULL){
                    bool added = false;
                    for(auto &j : newOp->incomingVars){
                        if(j->getName().compare(i->varNext->getName()) == 0){
                            added = true;
                            break;
                        }
                    }
                    
                    if(!added){
                        newOp->incomingVars.push_back(i->varNext);
                        i->varNext->toOperations.push_back(newOp);
                    }
                }
            }
            if(openBlocks.front()->ifelse->inElseBlock){
                this->openBlocks.front()->ifelse->elseOperations.push_back(newOp);
            }else{
                this->openBlocks.front()->ifelse->ifOperations.push_back(newOp);
            }
        }
        this->openBlocks.insert(openBlocks.begin(), newOp);
    }else if(lineType == ELSE_TYPE){
        /* Addes else block to open else block queue */
        this->openBlocks.insert(openBlocks.begin(), this->elseCheck);
        this->elseCheck->ifelse->inElseBlock = true;
        
        /* Invalid line */
        if(!line.size()){
            cout << "ERROR: Invalid if statement syntax" << endl;
            return false;
        }

    } else if(lineType == BRACKET_TYPE){
        /* Close off a if else block when a close bracket is parsed */
        this->elseCheck = openBlocks.front();
        this->openBlocks.front()->ifelse->inElseBlock = false;
        this->openBlocks.erase(openBlocks.begin());
    } else if(lineType == FORLOOP_TYPE){
        //TODO: For loop if time
    } else {
        
        /* Gets parents if statement */
        Operation *newOp = new Operation();
        if(openBlocks.size() != 0){
            newOp->inIfElse = true;
            newOp->parent = openBlocks.front();
            if(newOp->parent->ifelse->inElseBlock){
                for(int i = 0; i < (signed)newOp->parent->ifelse->elseOperations.size(); i++){
                    if(newOp->parent->ifelse->elseOperations.at(i)->getOperation() == IFELSE){
                        newOp->toIfOp = newOp->parent->ifelse->elseOperations.at(i);
                        newOp->parent->ifelse->elseOperations.at(i)->afterIf.push_back(newOp);
                    }
                }
            }else{
                for(int i = 0; i < (signed)newOp->parent->ifelse->ifOperations.size(); i++){
                    if(newOp->parent->ifelse->ifOperations.at(i)->getOperation() == IFELSE){
                        newOp->toIfOp = newOp->parent->ifelse->ifOperations.at(i);
                        newOp->parent->ifelse->ifOperations.at(i)->afterIf.push_back(newOp);
                    }
                }
            }
        }else{
            for(int i = 0; i < (signed)operations.size(); i++){
                if(operations.at(i)->getOperation() == IFELSE){
                    newOp->toIfOp = operations.at(i);
                    operations.at(i)->afterIf.push_back(newOp);
                }
            }
        }
        
        /* Checks to see if the first variable is an output type */
        bool assigned = false;
        string var = line.front();
        for(int i = 0; i < (signed)this->outputs.size(); i++){
            if(var.compare(outputs.at(i)->getName()) == 0){
                newOp->outNext = outputs.at(i);
                outputs.at(i)->fromOperation = newOp;
                assigned = true;
                break;
            }
        }
        if(!assigned){
            for(int i = 0; i < (signed)this->variables.size(); i++){
                if(var.compare(variables.at(i)->getName()) == 0){
                    newOp->varNext = variables.at(i);
                    /* Add to outgoing if block variables */
                    if(openBlocks.size() > 0){
                        for(auto &op : openBlocks){
                            bool added = false;
                            for(auto &j : op->outgoingVars){
                                if(j->getName().compare(variables.at(i)->getName()) == 0){
                                    added = true;
                                    break;
                                }
                            }
                            
                            if(!added){
                                op->outgoingVars.push_back(variables.at(i));
                            }
                        }
                    }
                    
                    variables.at(i)->fromOperation = newOp;
                    if(newOp->inIfElse){
                        variables.at(i)->fromIfOp = openBlocks.back();
                    }
                    assigned = true;
                    break;
                }
            }
        }
        
        /* No assignment was defined */
        if(!assigned){
            cout << "ERROR: Missing variable or ouput definition for: " << var << endl;
            return false;
        }
        
        /* Remove first variable and equals sign in operation line */
        line.erase(line.begin());
        line.erase(line.begin());
        if(!line.size()){
            return false;
        }
        
        /* Assigns first Input or Variable to operation */
        assigned = false;
        var = line.front();
        for(int i = 0; i < (signed)this->inputs.size(); i++){
            if(var.compare(inputs.at(i)->getName()) == 0){
                newOp->inInput[0] = inputs.at(i);
                inputs.at(i)->toOperations.push_back(newOp);
                assigned = true;
                break;
            }
        }
        if(!assigned){
            for(int i = 0; i < (signed)this->variables.size(); i++){
                if(var.compare(variables.at(i)->getName()) == 0){
                    newOp->inVar[0] = variables.at(i);
                    if(variables.at(i)->fromIfOp != NULL){
                        variables.at(i)->fromIfOp->outgoingOps.push_back(newOp);
                    }
                    
//                    bool in = false;
//                    if(openBlocks.size() != 0){
//                        for(auto &j : variables.at(i)->toOperations){
//                            if(j == openBlocks.front()){
//                                in = true;
//                                break;
//                            }
//                        }
//                    }
                    variables.at(i)->toOperations.push_back(newOp);
                    assigned = true;
                    break;
                }
            }
        }
        
        /* No assignment was defined */
        if(!assigned){
            cout << "ERROR: Missing input or wire varible definition for: " << var << endl;
            return false;
        }
        
        /* Remove second variable in operation line */
        line.erase(line.begin());
        if(!line.size()){
            return false;
        }
        
        /* Gets operation type */
        var = line.front();
        if(var.compare("+") == 0){
            line.erase(line.begin());
            var = line.front();
            if(var.compare("1") == 0){
                newOp->setOperation(INC);
                newOp->setOpID(getID(INC));
                newOp->calcWidth();
                newOp->setSign();
                if(!this->openBlocks.size()){
                    this->operations.push_back(newOp);
                }else{
                    if(openBlocks.front()->ifelse->inElseBlock){
                        this->openBlocks.front()->ifelse->elseOperations.push_back(newOp);
                    }else{
                        this->openBlocks.front()->ifelse->ifOperations.push_back(newOp);
                    }
                }
                return true;
            }
            newOp->setOperation(ADD);
            newOp->setOpID(getID(ADD));
        } /* SUB or DEC */
        else if(var.compare("-") == 0){
            line.erase(line.begin());
            var = line.front();
            if(var.compare("1") == 0){
                newOp->setOperation(DEC);
                newOp->setOpID(getID(DEC));
                newOp->calcWidth();
                newOp->setSign();
                if(!this->openBlocks.size()){
                    this->operations.push_back(newOp);
                }else{
                    if(openBlocks.front()->ifelse->inElseBlock){
                        this->openBlocks.front()->ifelse->elseOperations.push_back(newOp);
                    }else{
                        this->openBlocks.front()->ifelse->ifOperations.push_back(newOp);
                    }
                }
                return true;
            }
            newOp->setOperation(SUB);
            newOp->setOpID(getID(SUB));
        } /* MUL */
        else if(var.compare("*") == 0){
            newOp->setOperation(MUL);
            newOp->setOpID(getID(MUL));
        } /* COMP_GT */
        else if(var.compare(">") == 0){
            newOp->setOperation(COMP_GT);
            newOp->setOpID(getID(COMP_GT));
        } /* COMP_LT */
        else if(var.compare("<") == 0){
            newOp->setOperation(COMP_LT);
            newOp->setOpID(getID(COMP_LT));
        } /* COMP_EQ */
        else if(var.compare("==") == 0){
            newOp->setOperation(COMP_EQ);
            newOp->setOpID(getID(COMP_EQ));
        } /* MUX2x1 */
        else if(var.compare("?") == 0){
            newOp->setOperation(MUX2x1);
            newOp->setOpID(getID(MUX2x1));
        } /* SHR */
        else if(var.compare(">>") == 0){
            newOp->setOperation(SHR);
            newOp->setOpID(getID(SHR));
        } /* SHL */
        else if(var.compare("<<") == 0){
            newOp->setOperation(SHL);
            newOp->setOpID(getID(SHL));
        } /* DIV */
        else if(var.compare("/") == 0){
            newOp->setOperation(DIV);
            newOp->setOpID(getID(DIV));
        } /* MOD */
        else if(var.compare("%") == 0){
            newOp->setOperation(MOD);
            newOp->setOpID(getID(MOD));
        } /* Invalid Line/Operator */
        else{
            cout << "ERROR: Invalid type -> " << var << endl;
            return false;
        }
        
        /* Removes oporator if it hasn't been removed already */
        if(newOp->getOperation() != ADD || newOp->getOperation() != SUB){
            line.erase(line.begin());
        }
        
        /* Assign out Input or Variable to operation */
        assigned = false;
        var = line.front();
        for(int i = 0; i < (signed)this->inputs.size(); i++){
            if(var.compare(inputs.at(i)->getName()) == 0){
                newOp->inInput[1] = inputs.at(i);
                inputs.at(i)->toOperations.push_back(newOp);
                assigned = true;
                break;
            }
        }
        if(!assigned){
            for(int i = 0; i < (signed)this->variables.size(); i++){
                if(var.compare(variables.at(i)->getName()) == 0){
                    newOp->inVar[1] = variables.at(i);
                    if(variables.at(i)->fromIfOp != NULL){
                        variables.at(i)->fromIfOp->outgoingOps.push_back(newOp);
                    }
//                    bool in = false;
//                    if(openBlocks.size() != 0){
//                        for(auto &j : variables.at(i)->toOperations){
//                            if(j == openBlocks.front()){
//                                in = true;
//                                break;
//                            }
//                        }
//                    }
                    variables.at(i)->toOperations.push_back(newOp);
                    assigned = true;
                    break;
                }
            }
        }
        
        /* No assignment was defined */
        if(!assigned){
            cout << "ERROR: Missing input or variable definition for: " << var << endl;
            return false;
        }
        
        if(newOp->getOperation() == MUX2x1){
            
            line.erase(line.begin());
            line.erase(line.begin());
            
            /* Assign out Input or Wire to operation */
            assigned = false;
            var = line.front();
            for(int i = 0; i < (signed)this->inputs.size(); i++){
                if(var.compare(inputs.at(i)->getName()) == 0){
                    newOp->inInput[2] = inputs.at(i);
                    inputs.at(i)->toOperations.push_back(newOp);
                    assigned = true;
                    break;
                }
            }
            if(!assigned){
                for(int i = 0; i < (signed)this->variables.size(); i++){
                    if(var.compare(variables.at(i)->getName()) == 0){
                        newOp->inVar[2] = variables.at(i);
                        if(variables.at(i)->fromIfOp != NULL){
                            variables.at(i)->fromIfOp->outgoingOps.push_back(newOp);
                        }
//                        bool in = false;
//                        if(openBlocks.size() != 0){
//                            for(auto &j : variables.at(i)->toOperations){
//                                if(j == openBlocks.front()){
//                                    in = true;
//                                    break;
//                                }
//                            }
//                        }
                        variables.at(i)->toOperations.push_back(newOp);
                        assigned = true;
                        break;
                    }
                }
            }
            
            /* No assignment was defined */
            if(!assigned){
                cout << "ERROR: Missing input or variable definition for: " << var << endl;
                return false;
            }
        }
        
        /* Adds operation to module */
        newOp->calcWidth();
        newOp->setSign();
        if(!this->openBlocks.size()){
            this->operations.push_back(newOp);
        }else{
            if(openBlocks.front()->ifelse->inElseBlock){
                this->openBlocks.front()->ifelse->elseOperations.push_back(newOp);
            }else{
                this->openBlocks.front()->ifelse->ifOperations.push_back(newOp);
            }
        }
        
    }
    
    return true;
}

/**
 * This function outputs the scheduled graph structure to the .v file with Verilog syntax
 */
bool Module::output_module(string file) {
    ofstream out;
    
    /* Wrong type of output file */
    if(file.find(".") == string::npos){
        cout << "Usage: dpgen netlistFile verilogFile (with .v extension)" << endl;
        return false;
    }
    
    out.open(file.c_str());
    
    /* if file is available, open and read */
    if (!out.is_open()) {
        cout << "Error opening up file: " << file << endl;
        return false;
    }
    
    /* Prints heading */
    out << "`timescale 1ns / 1ns" << endl << endl;
    out << "module " << this->name << "(";
    out << inputs.at(0)->getName() << ", ";
    out << inputs.at(1)->getName() << ", ";
    out << inputs.at(2)->getName() << ", ";
    out << outputs.at(0)->getName() << ", ";
    for(int i = 3; i < (signed)this->inputs.size(); i++){
        out << inputs.at(i)->getName() << ", ";;
    }
    for(int i = 1; i < (signed)this->outputs.size(); i++){
        if(i == (signed)this->outputs.size() - 1){
            out << outputs.at(i)->getName();
            break;
        }
        out << outputs.at(i)->getName() << ", ";
    }
    out << ");" << endl << endl;
    
    /* Prints all inputs, outputs, regs */
    for(int i = 0; i < (signed)this->inputs.size(); i++){
        out << inputs.at(i)->toString() << endl;
    }
    out << endl;
    for(int i = 0; i < (signed)this->outputs.size(); i++){
        out << outputs.at(i)->toString() << endl;
    }
    out << endl;
    for(int i = 0; i < (signed)this->variables.size(); i++){
        out << variables.at(i)->toString() << endl;
    }
    out << endl;
    
    /* Print state info */
    int count = 0;
    for(auto &i : operations){
        if(i->getOperation() == IFELSE){
            count = i->endFrame - i->scheduledTime;
        }
    }
    
    out << "\treg [" << ceil(log2(count + Latency + 2)) - 1 << ":0] state;" << endl;
    out << "\tparameter Wait = 0";
    for(int i = 1; i <= Latency; i++){
        out << ", S" << i << " = " << i;
    }
    for(int i = 1; i <= count; i++){
        out << ", if_S" << i << " = " << i;
    }
    out << ", Final = " << Latency + 1 << ";" << endl << endl;
    
    /* Print states and operations */
    out << "\talways @(posedge Clk) begin" << endl;
    out << "\t\tif(Rst) begin" << endl;
    out << "\t\t\tstate <= Wait;" << endl;
    for(auto &i : outputs){
        out << "\t\t\t" << i->getName() << " <= 0;" << endl;
    }
    for(auto &i : variables){
        out << "\t\t\t" << i->getName() << " <= 0;" << endl;
    }
    out << "\t\tend" << endl;
    out << "\t\telse begin" << endl;
    out << "\t\t\tcase(state)" << endl;
    
    out << "\t\t\t\tWait: begin" << endl;
    out << "\t\t\t\t\tDone <= 0;" << endl;
    out << "\t\t\t\t\tif (Start)" << endl;
    out << "\t\t\t\t\t\tstate <= S1;" << endl;
    out << "\t\t\t\t\telse" << endl;
    out << "\t\t\t\t\t\tstate <= Wait;" << endl;
    out << "\t\t\t\tend" << endl;
    
    int ifCount = 1;
    int elseCount = 1;
    for(int i = 1; i <= Latency; i++){
        int ifOpIndex = -1;
        out << "\t\t\t\tS" << i << ": begin" << endl;
        for(int j = 0; j < (signed)operations.size(); j++){
            if(operations.at(j)->scheduledTime == i){
                if(operations.at(j)->getOperation() != IFELSE){
                    out << "\t\t\t\t\t" << operations.at(j)->toString() << endl;
                }else{
                    ifOpIndex = j;
                }
            }
        }
        
        if(ifOpIndex != -1){
            string cond = "";
            if(operations.at(ifOpIndex)->ifelse->inputCondition != NULL){
                cond += operations.at(ifOpIndex)->ifelse->inputCondition->getName();
            }else{
                cond += operations.at(ifOpIndex)->ifelse->varCondition->getName();
            }
            
            out << "\t\t\t\t\tif(" + cond + ")" << endl;
            out << "\t\t\t\t\t\tstate <= if_S1;" << endl;
            out << "\t\t\t\t\telse" << endl;
            
            if(operations.at(ifOpIndex)->ifelse->elseOperations.size() != 0){
                out << "\t\t\t\t\t\tstate <= else_S1;" << endl;
            }else{
                if(operations.at(ifOpIndex)->ifelse->ifEndTime >= Latency){
                    out << "\t\t\t\t\t\tstate <= Final;" << endl;
                }else{
                    out << "\t\t\t\t\t\tstate <= S" << i + 1 << ";" << endl;
                }
            }
        }else{
            if(i != Latency){
                out << "\t\t\t\t\tstate <= S" << i + 1 << ";" << endl;
            }else{
                out << "\t\t\t\t\tstate <= Final;" << endl;
            }
        }
        out << "\t\t\t\tend" << endl;
        
        int nested = 0, nestedCount = 0, ifState = 1, elseState = 1;
        
        /* Print if states */
        if (ifOpIndex != -1) {
            outputIfBlock(out, ifOpIndex, true, Latency, ifCount, elseCount, nested, nestedCount, ifState);
            if (operations.at(ifOpIndex)->ifelse->elseOperations.size() != 0) {
                outputElseBlock(out, ifOpIndex, true, Latency, ifCount, elseCount, nested, nestedCount, elseState);
            }
        }
        
    }
    
    out << "\t\t\t\tFinal: begin" << endl;
    out << "\t\t\t\t\tDone <= 1;" << endl;
    out << "\t\t\t\t\tstate <= Wait;" << endl;
    out << "\t\t\t\tend" << endl;
    
    out << "\t\t\tendcase" << endl;
    out << "\t\tend" << endl;
    out << "\tend" << endl;
    out << "endmodule" << endl;
    out.close();
    
    return true;
}

//void Module::countParam(int &count, Operation *node){
//
//}

/**
 * Outputs the if block of an if statement to verilog file
 */
void Module::outputIfBlock(ofstream &outF, int index, bool first, int prevEnd, int &ifCount, int &elseCount, int &nestedIndex, int &nestedCount, int &ifState){
    if(index == -1){
        return;
    }
    bool finished = false;
    int orig = index;
    int origIfCount = ifCount;
    int origElseCount = elseCount;
    int temp = 0;
    Operation *ifOp = new Operation();
    
    if (ifCount < 2) {
        ifOp = operations.at(index);
    }else {
        ifOp = operations.at(index)->ifelse->ifOperations.at(nestedIndex);
    }
    
    int ifOpIndex = -1;
    outF << "\t\t\t\tif_S" << ifState << ": begin" << endl;
    for (int i = 0; i < ifOp->ifelse->ifOperations.size(); i++) {
        ifOpIndex = -1;
        if (ifCount < 2) {
            if(ifOp->ifelse->ifOperations.at(i)->getOperation() != IFELSE){
                outF << "\t\t\t\t\t" << ifOp->ifelse->ifOperations.at(i)->toString() << endl;
                ifOp->printedOps++;
            }else{
                ifOpIndex = i;
                ifCount++;
                nestedCount++;
                ifState++;
            }
        }else{
            if(ifOp->ifelse->ifOperations.at(i)->getOperation() != IFELSE){
                outF << "\t\t\t\t\t" << ifOp->ifelse->ifOperations.at(i)->toString() << endl;
                ifOp->printedOps++;
                if(ifOp->printedOps != ifOp->ifelse->ifOperations.size()) {
                    continue;
                }
            }else{
                ifOpIndex = i;
                ifCount++;
                nestedCount++;
                ifState++;
            }
        }
    /*
    for(int i = ifOp->frame.min; i <= ifOp->ifelse->ifEndTime; i++){
        int ifOpIndex = -1;
        outF << "\t\t\t\tif_S" << ifState << ": begin" << endl;
        for(int j = 0; j < (signed)ifOp->ifelse->ifOperations.size(); j++){
            if(ifCount < 2) {
                if((ifOp->ifelse->ifOperations.at(j)->scheduledTime - 1) == i){
                    if(ifOp->ifelse->ifOperations.at(j)->getOperation() != IFELSE){
                        outF << "\t\t\t\t\t" << ifOp->ifelse->ifOperations.at(j)->toString() << endl;
                        ifOp->printedOps++;
                    }else{
                        ifOpIndex = j;
                        ifCount++;
                        nestedCount++;
                        ifState++;
                    }
                }
            }else {
                if(ifOp->ifelse->ifOperations.at(j)->scheduledTime == i){
                    if(ifOp->ifelse->ifOperations.at(j)->getOperation() != IFELSE){
                        outF << "\t\t\t\t\t" << ifOp->ifelse->ifOperations.at(j)->toString() << endl;
                        ifOp->printedOps++;
                    }else{
                        if(ifOp->ifelse->ifOperations.at(j)->printedOps == (signed)ifOp->ifelse->ifOperations.at(j)->ifelse->ifOperations.size()) {
                            operations.at(index)->ifelse->ifOperations.erase(operations.at(index)->ifelse->ifOperations.begin());
                            j--;
                            i++;
                        }else if (ifOp->ifelse->ifOperations.at(j)->printedOps == (signed)ifOp->ifelse->ifOperations.at(j)->ifelse->elseOperations.size()) {
                            operations.at(index)->ifelse->elseOperations.erase(operations.at(index)->ifelse->elseOperations.begin());
                            j--;
                            i++;
                        }else {
                            ifOpIndex = j;
                            ifCount++;
                            nestedCount++;
                            ifState++;
                        }
                    }
                }
            }
        }
        */
        int ifPrevEnd = 0;
        int elsePrevEnd = 0;
        if(ifOpIndex != -1){
            string cond = "";
            if(ifOp->ifelse->ifOperations.at(ifOpIndex)->ifelse->inputCondition != NULL){
                cond += ifOp->ifelse->ifOperations.at(ifOpIndex)->ifelse->inputCondition->getName();
            }else{
                cond += ifOp->ifelse->ifOperations.at(ifOpIndex)->ifelse->varCondition->getName();
            }
            
            outF << "\t\t\t\t\tif(" + cond + ")" << endl;
            //outF << "\t\t\t\t\t\tstate <= if_S" << ifCount << ";" << endl;
            outF << "\t\t\t\t\t\tstate <= if_S" << ifState << ";" << endl;
            outF << "\t\t\t\t\telse" << endl;
            
            ifPrevEnd = ifOp->ifelse->ifOperations.at(ifOpIndex)->ifelse->ifEndTime;
            if(ifOp->ifelse->ifOperations.at(ifOpIndex)->ifelse->elseOperations.size() != 0){
                outF << "\t\t\t\t\t\tstate <= else_S" << elseCount << ";" << endl;
                elseCount++;
                elsePrevEnd = ifOp->ifelse->ifOperations.at(ifOpIndex)->ifelse->elseEndTime;
            }else{
                outF << "\t\t\t\t\t\tstate <= if_S" << ifState + 1 << ";" << endl;
            }
        }else{
            if(ifOp->printedOps != ifOp->ifelse->ifOperations.size()){
                outF << "\t\t\t\t\tstate <= if_S" << ifCount + 1 << ";" << endl;
            }else{
                if(ifOp->ifelse->ifEndTime >= prevEnd && first){
                    outF << "\t\t\t\t\t\tstate <= Final;" << endl;
                }else if(first){
                    outF << "\t\t\t\t\tstate <= S" << ifOp->scheduledTime + 1 << ";" << endl;
                    finished = true;
                }else{
                    ifState++;
                    ifOp->ifelse->finished = true;
                    outF << "\t\t\t\t\tstate <= if_S" << ifState << ";" << endl;
                    
                }
            }
        }
        outF << "\t\t\t\tend" << endl;
        
        /* Print if states */
        if(ifCount > origIfCount && !finished) {
            outputIfBlock(outF, orig, false, ifPrevEnd, ifCount, elseCount, ifOpIndex, nestedCount, ifState);
            if(ifOp->ifelse->ifOperations.at(i)->ifelse->finished){
                operations.at(index)->ifelse->ifOperations.erase(operations.at(index)->ifelse->ifOperations.begin());
                i--;
                outF << "\t\t\t\tif_S" << ifState << ": begin" << endl;
            }
        }
        if(elseCount > origElseCount && !finished) {
            outputElseBlock(outF, orig, false, elsePrevEnd, ifCount, elseCount, ifOpIndex, nestedCount, ifState);
        }
    }
}

/**
 * Outputs the else block of an if statement to verilog file
 */
void Module::outputElseBlock(ofstream &outF, int index, bool first, int prevEnd, int &ifCount, int &elseCount, int &nestedIndex, int &nestedCount, int &ifState){
    if(index == -1){
        return;
    }
    int orig = index;
    Operation *ifOp = new Operation();
    if (elseCount < 2) {
        ifOp = operations.at(index);
    }else {
        ifOp = operations.at(orig)->ifelse->elseOperations.at(nestedIndex);
    }
    for(int i = ifOp->frame.min; i <= ifOp->ifelse->elseEndTime; i++){
        int ifOpIndex = -1;
        outF << "\t\t\t\telse_S" << elseCount + nestedIndex << ": begin" << endl;
        for(int j = 0; j < (signed)ifOp->ifelse->elseOperations.size(); j++){
            if(ifOp->ifelse->elseOperations.at(j)->scheduledTime == i){
                if(ifOp->ifelse->elseOperations.at(j)->getOperation() != IFELSE){
                    outF << "\t\t\t\t\t" << ifOp->ifelse->elseOperations.at(j)->toString() << endl;
                }else{
                    ifOpIndex = j;
                }
            }
        }
        
        int ifPrevEnd = 0;
        int elsePrevEnd = 0;
        if(ifOpIndex != -1){
            string cond = "";
            if(ifOp->ifelse->ifOperations.at(ifOpIndex)->ifelse->inputCondition != NULL){
                cond += ifOp->ifelse->ifOperations.at(ifOpIndex)->ifelse->inputCondition->getName();
            }else{
                cond += ifOp->ifelse->ifOperations.at(ifOpIndex)->ifelse->varCondition->getName();
            }
            
            outF << "\t\t\t\t\tif(" + cond + ")" << endl;
            outF << "\t\t\t\t\t\tstate <= if_S" << ifCount + 1 << ";" << endl;
            outF << "\t\t\t\t\telse" << endl;
            
            ifPrevEnd = ifOp->ifelse->ifOperations.at(ifOpIndex)->ifelse->ifEndTime;
            if(ifOp->ifelse->ifOperations.at(ifOpIndex)->ifelse->elseOperations.size() != 0){
                outF << "\t\t\t\t\t\tstate <= else_S" << elseCount << ";" << endl;
                elsePrevEnd = ifOp->ifelse->ifOperations.at(ifOpIndex)->ifelse->elseEndTime;
            }else{
                outF << "\t\t\t\t\t\tstate <= S" << ifCount + 1 << ";" << endl;
            }
        }else{
            if(i != ifOp->ifelse->ifEndTime){
                outF << "\t\t\t\t\tstate <= else_S" << elseCount + 1 << ";" << endl;
            }else{
                if(ifOp->ifelse->ifEndTime >= prevEnd && first){
                    outF << "\t\t\t\t\tstate <= Final;" << endl;
                }else if(first){
                    outF << "\t\t\t\t\tstate <= S" << ifOp->ifelse->ifEndTime + 1 << ";" << endl;
                }else{
                    outF << "\t\t\t\t\tstate <= if_S" << ifOp->ifelse->ifEndTime + 1 << ";" << endl;
                }
            }
        }
        outF << "\t\t\t\tend" << endl;
        elseCount++;
        
        /* Print if states */
        outputIfBlock(outF, orig, false, ifPrevEnd, ifCount, elseCount, ifOpIndex, nestedCount, ifState);
        outputElseBlock(outF, ifOpIndex, false, elsePrevEnd, ifCount, elseCount, ifOpIndex, nestedCount, ifState);
    }
}

/**
 * Calculates the next available operation id
 */
int Module::getID(Operations operation){
    int id = 1;
    for(int i = 0; i < (signed)this->operations.size(); i++){
        if(this->operations.at(i)->getOperation() == operation){
            id++;
        }
    }
    
    return id;
}

/**
 * Gets the sign and size of a particular data type when parsing high level code file
 */
bool Module::getDataType(string type, int *size){
    
    /* Return data width */
    if(!type.compare("Int1") || !type.compare("UInt1")){
        *size = 1;
    }
    else if(!type.compare("Int2") || !type.compare("UInt2")){
        *size = 2;
    }
    else if(!type.compare("Int8") || !type.compare("UInt8")){
        *size = 8;
    }
    else if(!type.compare("Int16") || !type.compare("UInt16")){
        *size = 16;
    }
    else if(!type.compare("Int32") || !type.compare("UInt32")){
        *size = 32;
    }
    else if(!type.compare("Int64") || !type.compare("UInt64")){
        *size = 64;
    } /* Invalid data type */
    else{
        *size = -1; // error
    }
    
    /* return signed or unsigned */
    return type.find("UInt") ? false : true;
}

/**
 * Schedules the operations using force directed scheduling algorithm. An attemp was made to implement if statments and schedule
 * the contents of the if statements as well
 */
bool Module::scheduleOperations(vector<Operation *> nodes, int min, int max) {
    vector<Operation *> unscheduled = nodes;
    vector<Operation *> scheduled;
    
    /* Perform force directed scheduling */
    while(unscheduled.size() != 0){
        bool onlyIfelse = true;
        for(auto &i : unscheduled){
            if(i->getOperation() != IFELSE){
                onlyIfelse = false;
                break;
            }
        }
        
        if(!onlyIfelse){
            if(!getTimeFrames(scheduled, unscheduled, min, max)){
                return false;
            }
            getTypePropabilities();
            getTotalForces(unscheduled);
            scheduleNode(scheduled, unscheduled); // Removes scheduled node
        }else{
            if(!getTimeFrames(scheduled, unscheduled, min, max)){
                return false;
            }
            int max = 0;
            for(auto &i : unscheduled.front()->outgoingOps){
                if(max < i->scheduledTime){
                    max = i->scheduledTime;
                }
            }
            for(auto &i : unscheduled.front()->afterIf){
                if(max < i->scheduledTime){
                    max = i->scheduledTime;
                }
            }
            
            scheduleOperations(unscheduled.front()->ifelse->ifOperations, unscheduled.front()->frame.min - 1, max - 1);
            max = 0;
            for(auto &i : unscheduled.front()->ifelse->ifOperations){
                if(max < (i->scheduledTime + i->getCycleDelay()) - 1){
                    max = (i->scheduledTime + i->getCycleDelay()) - 1;
                }
            }
            unscheduled.front()->ifelse->ifEndTime = max;
            //TODO: Schedule else operations
            int time = 0;
            for(auto &i : unscheduled.front()->incomingVars){
                int temp = (i->fromOperation->scheduledTime + i->fromOperation->getCycleDelay()) - 1;
                if(temp > time){
                    time = temp;
                }
//                for(auto &j : operations){
//                    if(j->varNext != NULL && j->varNext->getName().compare(i->getName())){
//                        for(auto &k : j->varNext->toOperations){
//                            if(k == unscheduled.front()){
//                                if((k->scheduledTime + k->getCycleDelay()) - 1 < time && (k->scheduledTime + k->getCycleDelay()) - 1 > 0){
//                                    time = (k->scheduledTime + k->getCycleDelay()) - 1;
//                                }
//                            }
//                        }
//                    }
//                }
            }
            
            if(!unscheduled.front()->incomingVars.size()){
                time = unscheduled.front()->frame.min;
            }
            
            unscheduled.front()->scheduledTime = time;
            scheduled.push_back(unscheduled.front());
            unscheduled.erase(unscheduled.begin());
        }
    }
    
    return true;
}

/**
 * This gets the time frames for scheduling using force directed algorithm
 */
bool Module::getTimeFrames(vector<Operation *> scheduled, vector<Operation *> unscheduled, int min, int max){
    
    /* Calculate ASAP */
    resetUnscheduled(unscheduled, true);
    resetScheduled(scheduled, false);
    if(!getASAPTimes(unscheduled, min)){
        return false;
    }
    
    /* Calculate ALAP */
    resetUnscheduled(unscheduled, false);
    resetScheduled(scheduled, true);
    if(!getALAPTimes(unscheduled, max)){
        return false;
    }
    
    return true;
}

/**
 * A reset funtion for some variables
 */
void Module::resetUnscheduled(vector<Operation *> unscheduled, bool ASAP){
    
    /* Resets variables for calculating frames */
    for(auto &i : inputs){
        if(!i->isScheduled){
            i->outCycle = -1;
        }
    }
    for(auto &i : outputs){
        if(!i->isScheduled){
            i->outCycle = -1;
        }
    }
    for(auto &i : variables){
        if(!i->isScheduled){
            i->outCycle = -1;
        }
    }
    
    for(auto &i : unscheduled){
        if(ASAP){
            i->frame.min = 0;
            if(i->getOperation() == IFELSE){
                i->endFrame = -1;
                for(auto &j : i->ifelse->ifOperations){
                    j->frame.min = 0;
                }
                for(auto &j : i->ifelse->elseOperations){
                    j->frame.min = 0;
                }
            }
        }else{
            i->frame.max = 0;
            if(i->getOperation() == IFELSE){
                for(auto &j : i->ifelse->ifOperations){
                    j->frame.max = 0;
                }
                for(auto &j : i->ifelse->elseOperations){
                    j->frame.max = 0;
                }
            }
        }
    }
}

/**
 * A reset function for scheduled nodes
 */
void Module::resetScheduled(vector<Operation *> scheduled, bool ALAP){
    
    for(int i = 0; i < (signed)scheduled.size(); i++){
        if(scheduled.at(i)->varNext != NULL){
            bool notApart = true;
            for(auto &j : scheduled.at(i)->varNext->toOperations){
                if(j->getOperation() == IFELSE && ALAP){
                    notApart = false;
                    break;
                }
            }
            
            if(notApart){
                scheduled.at(i)->varNext->outCycle = (scheduled.at(i)->scheduledTime + scheduled.at(i)->getCycleDelay()) - 1;
                scheduled.at(i)->varNext->permOutCycle = scheduled.at(i)->scheduledTime;
                scheduled.at(i)->varNext->isScheduled = true;
            }else{
                scheduled.at(i)->varNext->outCycle = -1;
            }
        }else{
            scheduled.at(i)->outNext->outCycle = (scheduled.at(i)->scheduledTime + scheduled.at(i)->getCycleDelay()) - 1;
            scheduled.at(i)->outNext->isScheduled = true;
        }
        for(int j = 0; j < NUM_INPUTS; j++){
            if(scheduled.at(i)->inVar[j] != NULL){
                if(!scheduled.at(i)->inVar[j]->isScheduled){
                    scheduled.at(i)->inVar[j]->outCycle = scheduled.at(i)->scheduledTime;
                    scheduled.at(i)->inVar[j]->permOutCycle = scheduled.at(i)->scheduledTime;
                }else{
                    if(scheduled.at(i)->inVar[j]->fromIfOp != NULL && ALAP){
                        /* Gets the outter most if in case of nesting */
                        int min = 999;
                        Operation *ifOp = scheduled.at(i)->inVar[j]->fromIfOp;
                        for(auto &k : ifOp->outgoingOps){
                            for(int l = 0; l < NUM_INPUTS; l++){
                                if(k->inVar[l] != NULL && k->inVar[l]->getName().compare(scheduled.at(i)->inVar[j]->getName()) == 0){
                                    if(scheduled.at(i)->scheduledTime < min){
                                        min = scheduled.at(i)->scheduledTime;
                                    }
                                }
                            }
                        }
                        
                        if(min == 999){
                            scheduled.at(i)->inVar[j]->outCycle = scheduled.at(i)->inVar[j]->permOutCycle;
                        }else{
                            scheduled.at(i)->inVar[j]->outCycle = min;
                        }
                    }else{
                        scheduled.at(i)->inVar[j]->outCycle = scheduled.at(i)->inVar[j]->permOutCycle;
                    }
                }
                scheduled.at(i)->inVar[j]->isScheduled = true;
            }else{
                if(scheduled.at(i)->inInput[j] != NULL){
                    if(!scheduled.at(i)->inInput[j]->isScheduled){
                        scheduled.at(i)->inInput[j]->outCycle = scheduled.at(i)->scheduledTime;
                        scheduled.at(i)->inInput[j]->permOutCycle = scheduled.at(i)->scheduledTime;
                    }else{
                        scheduled.at(i)->inInput[j]->outCycle = scheduled.at(i)->inInput[j]->permOutCycle;
                    }
                    scheduled.at(i)->inInput[j]->isScheduled = true;
                }
            }
        }
    }
}

/**
 * Gets the ASAP time frames of a graph
 */
bool Module::getASAPTimes(vector<Operation *> nodes, int startTime) {
    vector<Operation *> operationQueue = nodes;
    
    /* Loop until all variables and output delays have been updated */
    while(operationQueue.size() > 0){
        
        /* Iterate through all operations to update delays */
        bool opRemoved = false;
        for (int i = 0; i < (signed)nodes.size(); i++) {
            bool inCyclesCalculated = true;
            int tempCycle = 0;
            int maxInCycle = startTime;
            
            /* Get the current maximum delay from operation inputs */
            if(nodes.at(i)->getOperation() != IFELSE){
                if(nodes.at(i)->toIfOp != NULL){
                    if(nodes.at(i)->toIfOp->endFrame != -1){
                        maxInCycle = nodes.at(i)->toIfOp->endFrame;
                    }else{
                        inCyclesCalculated = false;
                        break;
                    }
                }
                for(int j = 0; j < NUM_INPUTS; j++){
                    if(nodes.at(i)->inVar[j] != NULL){
                        if(nodes.at(i)->inVar[j]->outCycle == -1){
                            inCyclesCalculated = false;
                            break;
                        }else{
                            /* Checks if var is coming from an if statement */
                            if(nodes.at(i)->inVar[j]->fromIfOp == NULL){
                                if(maxInCycle < nodes.at(i)->inVar[j]->outCycle){
                                    maxInCycle = nodes.at(i)->inVar[j]->outCycle;
                                }
                            }else{
                                /* Gets where the if statement will end since node is dependent */
                                Operation *ifOp = nodes.at(i)->inVar[j]->fromIfOp;
                                for(auto &k : ifOp->outgoingVars){
                                    if(maxInCycle < k->outCycle){
                                        maxInCycle = k->outCycle;
                                    }
                                    if(nodes.at(i)->parent == k->fromOperation->parent){
                                        maxInCycle = k->outCycle;
                                        break;
                                    }
                                }
                            }
                        }
                    }else{
                        if(nodes.at(i)->toIfOp != NULL){
                            if(nodes.at(i)->toIfOp->endFrame != -1){
                                if(maxInCycle < nodes.at(i)->toIfOp->endFrame){
                                    maxInCycle = nodes.at(i)->toIfOp->endFrame;
                                }
                            }else{
                                inCyclesCalculated = false;
                                break;
                            }
                        }else{
                           continue;
                        }
                    }
                }
            }else{
                if(!nodes.at(i)->incomingVars.size()){
                    maxInCycle++;
                }else{
                    for(auto &j : nodes.at(i)->incomingVars){
                        if(j->outCycle == -1){
                            inCyclesCalculated = false;
                            break;
                        }else{
                            if(maxInCycle < j->outCycle){
                                maxInCycle = j->outCycle;
                            }
                        }
                    }
                }
            }
            
            /* Check if all delay dependencies have been calculated */
            if(inCyclesCalculated){
                
                /* Pass delay of operation output */
                nodes.at(i)->frame.min = maxInCycle + 1;
                if(nodes.at(i)->frame.min > Latency){
                    cout << "ERROR: Not enough cycles to schedule graph" << endl;
                    return false;
                }
                if(nodes.at(i)->getOperation() != IFELSE){
                    tempCycle = maxInCycle + nodes.at(i)->getCycleDelay();
                    if(nodes.at(i)->varNext != NULL){
                        nodes.at(i)->varNext->outCycle = tempCycle;
                    }else{
                        nodes.at(i)->outNext->outCycle = tempCycle;
                    }
                }else{
                    if(!getASAPTimes(nodes.at(i)->ifelse->ifOperations, nodes.at(i)->frame.min - 1)){
                        return false;
                    }
                    int max = 0;
                    for(auto &j : nodes.at(i)->ifelse->ifOperations){
                        if(max < (j->frame.min + j->getCycleDelay()) - 1){
                            max = (j->frame.min + j->getCycleDelay()) - 1;
                        }
                    }
                    nodes.at(i)->endFrame = max;
                    //TODO: Recursive call for else block
                }
                
                /* Remove currently calculated operation from the operation queue */
                for(int j = 0; j < (signed)operationQueue.size(); j++){
                    if(nodes.at(i)->getOperation() == operationQueue.at(j)->getOperation()
                       && nodes.at(i)->getOpID() == operationQueue.at(j)->getOpID()){
                        operationQueue.erase(operationQueue.begin() + j);
                        opRemoved = true;
                        break;
                    }
                }
            }
            
        }
        
        if(!opRemoved){
            cout << "ERROR: Unconnected operation input wire" << endl;
            return false;
        }
    }
    
    return true;
}

/**
 * Gets the ALAP time frames of a graph
 */
bool Module::getALAPTimes(vector<Operation *> nodes, int endTime) {
    vector<Operation *> operationQueue = nodes;
    
    for(auto &i : nodes){
        if(i->getOperation() != IFELSE){
            if(i->varNext != NULL){
                i->isUpdated = false;
                i->frame.max = 0;
            }
            if(i->inIfElse){
                i->isUpdated = false;
                i->frame.max = 0;
            }
        }else{
            i->isUpdated = false;
            i->frame.max = 0;
        }
    }
    
    /* Loop until all variables and output delays have been updated */
    while(operationQueue.size() > 0){
        
        /* Iterate through all operations to update delays */
        bool opRemoved = false;
        for (int i = 0; i < (signed)nodes.size(); i++) {
            bool inCyclesCalculated = true;
            double tempCycle = 0;
            double maxInCycle = endTime + 1;
            
            if(nodes.at(i)->frame.max != 0){
                continue;
            }
            
            if(nodes.at(i)->getOperation() != IFELSE){
                if(nodes.at(i)->varNext != NULL){
                    if(!nodes.at(i)->varNext->toOperations.size() && nodes.at(i)->parent != NULL){
                        maxInCycle = nodes.at(i)->parent->endFrame + 1;
                    }else{
                        for(auto &op : nodes.at(i)->varNext->toOperations){
                            if(!op->isUpdated || nodes.at(i)->varNext->outCycle == -1){
                                inCyclesCalculated = false;
                                break;
                            }
                        }
                        if(inCyclesCalculated){
                            if(nodes.at(i)->parent != NULL && nodes.at(i)->parent->parent != NULL){
                                if(nodes.at(i)->varNext->outCycle > nodes.at(i)->parent->parent->endFrame){
                                    maxInCycle = nodes.at(i)->parent->endFrame + 1;
                                }
                            }else{
                                maxInCycle = nodes.at(i)->varNext->outCycle;
                            }
                        }
                    }
                }else{
                    if(nodes.at(i)->inIfElse){
                        for(auto &j : nodes.at(i)->parent->afterIf){
                            if(j->isUpdated){
                                if(maxInCycle > j->frame.max){
                                    maxInCycle = j->frame.max;
                                }
                            }
                        }
                    }
                }
            }else{
                for(auto &var : nodes.at(i)->outgoingVars){
                    for(auto &op : var->toOperations){
                        if((!op->isUpdated || var->outCycle == -1) && op->getOperation() != IFELSE && !op->inIfElse){
                            inCyclesCalculated = false;
                            break;
                        }
                    }
                    
                    if(inCyclesCalculated && maxInCycle > var->outCycle && var->outCycle > 0){
                        maxInCycle = var->outCycle;
                    }
                }
                for(auto &op : nodes.at(i)->afterIf){
                    if(!op->isUpdated){
                        inCyclesCalculated = false;
                        break;
                    }else{
                        if(inCyclesCalculated && op->frame.max > 0 && maxInCycle > op->frame.max){
                            maxInCycle = op->frame.max - 1;
                        }
                    }
                }
            }
            
            /* Check if all delay dependencies have been calculated */
            if(inCyclesCalculated){
                
                /* Pass delay of operation output */
                if(nodes.at(i)->getOperation() != IFELSE){
                    tempCycle = maxInCycle - nodes.at(i)->getCycleDelay();
                    for(int j = 0; j < NUM_INPUTS; j++){
                        if(nodes.at(i)->inVar[j] != NULL){
                            if(nodes.at(i)->inVar[j]->outCycle == -1 || tempCycle < nodes.at(i)->inVar[j]->outCycle){
                                nodes.at(i)->inVar[j]->outCycle = tempCycle;
                            }
                        }else{
                            if(nodes.at(i)->inInput[j] != NULL){
                                nodes.at(i)->inInput[j]->outCycle = tempCycle;
                            }
                        }
                    }
                }else{
                    nodes.at(i)->isUpdated = true;
                    if(!getALAPTimes(nodes.at(i)->ifelse->ifOperations, maxInCycle)){
                        return false;
                    }
                    
                    /* Update all variables to continue Alap process */
                    tempCycle = maxInCycle;
                    if(nodes.at(i)->ifelse->ifOperations.size() != 0){
                        if(nodes.at(i)->ifelse->ifOperations.at(0)->getOperation() == IFELSE){
                            if(nodes.at(i)->ifelse->ifOperations.at(0)->frame.max - 1 < tempCycle){
                                tempCycle = nodes.at(i)->ifelse->ifOperations.at(0)->frame.max - 1;
                            }
                        }
                    }
                    for(auto &j : nodes.at(i)->ifelse->ifOperations){
                        if(j->frame.max < tempCycle){
                            tempCycle = j->frame.max;
                        }
                    }
                    for(auto &j : nodes.at(i)->incomingVars){
                        if(tempCycle < j->outCycle){
                            j->outCycle = tempCycle;
                        }
                    }
                    nodes.at(i)->ifelse->varCondition->outCycle = tempCycle;
                    
                    //TODO: Recursive call for else block
                }
                
                /* Checks if graph fits in latency contraint */
                nodes.at(i)->frame.max = tempCycle;
                if(tempCycle < 1){
                    cout << "ERROR: Not enough cycles to schedule graph" << endl;
                    return false;
                }
                
                /* Remove currently calculated operation from the operation queue */
                for(int j = 0; j < (signed)operationQueue.size(); j++){
                    if(nodes.at(i)->getOperation() == operationQueue.at(j)->getOperation()
                       && nodes.at(i)->getOpID() == operationQueue.at(j)->getOpID()){
                        operationQueue.erase(operationQueue.begin() + j);
                        opRemoved = true;
                        nodes.at(i)->isUpdated = true;
                        break;
                    }
                }
            }
        }
        
        if(!opRemoved){
            cout << "ERROR: Unconnected operation" << endl;
            return false;
        }
    }
    
    return true;
}

/**
 * Gets the type propabilities for force directed scheduling
 */
void Module::getTypePropabilities(){
    vector<Operation *> res_AddSub;
    vector<Operation *> res_Mul;
    vector<Operation *> res_Logic;
    vector<Operation *> res_DivMod;
    
    for(auto &i : operations){
        switch(i->getOperation()){
            case ADD:
            case SUB:
            case INC:
            case DEC:
                res_AddSub.push_back(i);
                break;
            case MUL:
                res_Mul.push_back(i);
                break;
            case DIV:
            case MOD:
                res_DivMod.push_back(i);
                break;
            case COMP_EQ:
            case COMP_GT:
            case COMP_LT:
            case MUX2x1:
            case SHL:
            case SHR:
                res_Logic.push_back(i);
                break;
            case IFELSE:
                /* Do nothing */
                break;
        }
    }
    
    /* Adds the propabilities to sum matrix */
    sum_AddSub.clear();
    sum_Mul.clear();
    sum_Logic.clear();
    sum_DivMod.clear();
    for(int i = 0; i < Latency; i++){
        sum_AddSub.push_back(0);
        sum_Mul.push_back(0);
        sum_Logic.push_back(0);
        sum_DivMod.push_back(0);
    }
    
    for(int i = 0; i < (signed)res_AddSub.size(); i++){
        double prop = 1.0 / (double)res_AddSub.at(i)->frame.getWidth();
        for(int j = 1; j <= Latency; j++){
            if(j >= res_AddSub.at(i)->frame.min && j <= res_AddSub.at(i)->frame.max){
                sum_AddSub.at(j - 1) += prop;
            }
        }
    }
    
    for(int i = 0; i < (signed)res_Mul.size(); i++){
        double prop = 1.0 / (double)res_Mul.at(i)->frame.getWidth();
        for(int j = 1; j <= Latency; j++){
            if(j >= res_Mul.at(i)->frame.min && j <= res_Mul.at(i)->frame.max){
                sum_Mul.at(j - 1) += prop;
            }
        }
    }
    for(int i = 0; i < (signed)res_Logic.size(); i++){
        double prop = 1.0 / (double)res_Logic.at(i)->frame.getWidth();
        for(int j = 1; j <= Latency; j++){
            if(j >= res_Logic.at(i)->frame.min && j <= res_Logic.at(i)->frame.max){
                sum_Logic.at(j - 1) += prop;
            }
        }
    }
    for(int i = 0; i < (signed)res_DivMod.size(); i++){
        double prop = 1.0 / (double)res_DivMod.at(i)->frame.getWidth();
        for(int j = 1; j <= Latency; j++){
            if(j >= res_DivMod.at(i)->frame.min && j <= res_DivMod.at(i)->frame.max){
                sum_DivMod.at(j - 1) += prop;
            }
        }
    }
}

/**
 * Calculates the total forces of all the nodes at each time in their interval
 */
void Module::getTotalForces(vector<Operation *> nodes){
    
    getForces(nodes);
    
    for(int i = 0; i < (signed)nodes.size(); i++){
        int j = nodes.at(i)->frame.min;
        int tempTime = j;
        double minForce = nodes.at(i)->selfForces.at(j - 1) + nodes.at(i)->sucessorForces.at(j - 1) +
            nodes.at(i)->predecessorForces.at(j - 1);
        while(j <= nodes.at(i)->frame.max){
            double curForce = nodes.at(i)->selfForces.at(j - 1) + nodes.at(i)->sucessorForces.at(j - 1) +
                nodes.at(i)->predecessorForces.at(j - 1);
            if(minForce > curForce){
                minForce = curForce;
                tempTime = j;
            }
            j++;
        }
        
        nodes.at(i)->totalForce = minForce;
        nodes.at(i)->tempTime = tempTime;
    }
    
}

/**
 * Helper functino to get the self forces
 */
void Module::getForces(vector<Operation *> nodes) {
    
    /* Clear force variables */
    for(int i = 0; i < (signed)nodes.size(); i++){
        nodes.at(i)->selfForces.clear();
        nodes.at(i)->sucessorForces.clear();
        nodes.at(i)->predecessorForces.clear();
    }
    
    /* Get the self forces at each time in nodes time frame */
    for(int i = 1; i <= Latency; i++){
        for(int j = 0; j < (signed)nodes.size(); j++){
            double selfForce = 0;
            double prop = 1.0 / (double)nodes.at(j)->frame.getWidth();
            if(i >= nodes.at(j)->frame.min && i <= nodes.at(j)->frame.max){
                switch(nodes.at(j)->getOperation()){
                    case ADD:
                    case SUB:
                    case INC:
                    case DEC:
                        for(int k = nodes.at(j)->frame.min; k <= nodes.at(j)->frame.max; k++){
                            if(i == k){
                                selfForce += sum_AddSub.at(k - 1) * (1 - prop);
                            }else{
                                selfForce += sum_AddSub.at(k - 1) * (0 - prop);
                            }
                        }
                        break;
                    case MUL:
                        for(int k = nodes.at(j)->frame.min; k <= nodes.at(j)->frame.max; k++){
                            if(i == k){
                                selfForce += sum_Mul.at(k - 1) * (1 - prop);
                            }else{
                                selfForce += sum_Mul.at(k - 1) * (0 - prop);
                            }
                        }
                        break;
                    case DIV:
                    case MOD:
                        for(int k = nodes.at(j)->frame.min; k <= nodes.at(j)->frame.max; k++){
                            if(i == k){
                                selfForce += sum_DivMod.at(k - 1) * (1 - prop);
                            }else{
                                selfForce += sum_DivMod.at(k - 1) * (0 - prop);
                            }
                        }
                        break;
                    case COMP_EQ:
                    case COMP_GT:
                    case COMP_LT:
                    case MUX2x1:
                    case SHL:
                    case SHR:
                        for(int k = nodes.at(j)->frame.min; k <= nodes.at(j)->frame.max; k++){
                            if(i == k){
                                selfForce += sum_Logic.at(k - 1) * (1 - prop);
                            }else{
                                selfForce += sum_Logic.at(k - 1) * (0 - prop);
                            }
                        }
                        break;
                    case IFELSE:
                        /* Do Nothing */
                        selfForce = NO_FORCE;
                        break;
                }
                
                nodes.at(j)->selfForces.push_back(selfForce);
                nodes.at(j)->sucessorForces.push_back(0.0);
                nodes.at(j)->predecessorForces.push_back(0.0);
            }else{
                nodes.at(j)->selfForces.push_back(NO_FORCE);
                nodes.at(j)->sucessorForces.push_back(NO_FORCE);
                nodes.at(j)->predecessorForces.push_back(NO_FORCE);
            }
        }
    }
    
    /* Get the pred and succ forces at each time in nodes time frame */
    for(int i = 1; i <= Latency; i++){
        for(int j = 0; j < (signed)nodes.size(); j++){
            if(nodes.at(j)->selfForces.at(i - 1) != NO_FORCE){
                nodes.at(j)->sucessorForces.at(i - 1) = getSuccessorForces(nodes.at(j), i);
                nodes.at(j)->predecessorForces.at(i - 1) = getPredecessorForces(nodes.at(j), i);
            }
        }
    }
    
}

/**
 * Gets the successor forces of a node
 */
double Module::getSuccessorForces(Operation *node, int latency) {
    
    /* Node successor node */
    if(node->varNext == NULL){
        return 0.0;
    }
    
    /* Recursive call to get all successor forces */
    for(int i = 0; i < (signed)node->varNext->toOperations.size(); i++){
        Operation *suc = node->varNext->toOperations.at(i);
        if(latency >= suc->frame.min){
            return suc->selfForces.at(latency) + getSuccessorForces(suc, latency + 1);
        }
    }
    
    return 0.0;
}

/**
 * Gets the prdecessor forces of a node
 */
double Module::getPredecessorForces(Operation *node, int latency) {
    
    double predForceSum = 0.0;
    for(int i = 0; i < NUM_INPUTS; i++){
        if(node->inVar[i] == NULL){
            continue;
        }
        
        Operation *pred = node->inVar[i]->fromOperation;
        if(!pred->selfForces.size()){
            break;
        }
        if(latency <= pred->frame.max){
            predForceSum += pred->selfForces.at(latency - 2) + getPredecessorForces(pred, latency - 1);
        }
    }
    
    return predForceSum;
}

/**
 * Schedules the node with the smallest total force
 */
void Module::scheduleNode(vector<Operation *> &scheduled ,vector<Operation *> &unscheduled){
    double minForce = 999;
//    if(!unscheduled.empty()){
//        minForce = unscheduled.at(0)->totalForce;
//    }
    
    /* Find node to be scheduled */
    int index = 0;
    for(int i = 0; i < (signed)unscheduled.size(); i++){
        if(minForce > unscheduled.at(i)->totalForce && unscheduled.at(i)->getOperation() != IFELSE){
            minForce = unscheduled.at(i)->totalForce;
            index = i;
        }
    }
    
    /* Update scheduled node and remove from unscheduled vector */
    unscheduled.at(index)->scheduledTime = unscheduled.at(index)->tempTime;
    unscheduled.at(index)->frame.min = unscheduled.at(index)->scheduledTime;
    unscheduled.at(index)->frame.max = unscheduled.at(index)->scheduledTime;
    scheduled.push_back(unscheduled.at(index));
    unscheduled.erase(unscheduled.begin() + index);
}

/**
 * Delimeter function that splits a string at spaces and tabs and returns a vector of strings
 */
vector<string> Module::split(string const &input) {
    istringstream buffer(input);
    vector<string> ret((istream_iterator<string>(buffer)), istream_iterator<string>());
    return ret;
}

/***********************************************************************************
 * File: module.cpp
 * Author: Jason Tran, Andrew Camps
 * NetID: ichikasuto, andrewcamps
 * Date: November 17, 2017
 *
 * Description:
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
 *x
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
        
        /* Checks to see if the first variable is an output type */
        bool assigned = false;
        string var = line.front();
        Operation *newOp = new Operation();
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
 *
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
    out << "\treg [" << ceil(log2(Latency + 2)) - 1 << ":0] state;" << endl;
    out << "\tparameter Wait = 0";
    for(int i = 1; i <= Latency; i++){
        out << ", S" << i << " = " << i;
    }
    out << " Final = " << Latency + 1 << ";" << endl << endl;
    
    /* Print states and operations */
    out << "\talways @(posedge Clk) begin" << endl;
    out << "\t\tif(Rst) begin" << endl;
    out << "\t\t\tState <= Wait;" << endl;
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
    
    for(int i = 1; i <= Latency; i++){
        out << "\t\t\t\tS" << i << ": begin" << endl;
        bool status = false;
        for(auto &op : operations){
            if(op->scheduledTime == i){
                if(op->getOperation() != IFELSE){
                    out << "\t\t\t\t\t" << op->toString() << endl;
                }else{
                    status = outputIfBlock(&out, op);
                }
            }else if (op->getOperation() == IFELSE) {
                if(op->ifelse->ifTimeSchedule == i) {
                    status = outputIfBlock(&out, op);
                }
            }
        }
        
        if(!status){
            if(i != Latency){
                out << "\t\t\t\t\tstate <= S" << i + 1 << ";" << endl;
            }else{
                out << "\t\t\t\t\tstate <= Final;" << endl;
            }
            out << "\t\t\t\tend" << endl;
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

bool Module::outputIfBlock(ofstream *outFile, Operation *node){
    /* Distinguish if current operation is an if operation or else operation*/
    /* TODO: Check operations after if statement for dependency */
    if (node->ifelse->ifTimeSchedule != 0) {
        if (node->ifelse->varCondition != NULL) {
            *outFile << "\t\t\t\t\tif (" << node->ifelse->varCondition->getName() << ") begin" << endl;
        }else {
            *outFile << "\t\t\t\t\t\tif (" << node->ifelse->inputCondition->getName() << ") begin" << endl;
        }
        for (auto &i : node->ifelse->ifOperations) {
            if (i->getOperation() == IFELSE) {
                outputIfBlock(outFile, i);
            }else {
                *outFile << "\t\t\t\t\t\t" << i->toString() << endl;
                return false;
            }
        }
        *outFile << "\t\t\t\t\tend" << endl;
    }
    else{
        if (node->ifelse->varCondition != NULL) {
            *outFile << "\t\t\t\t\t\tif (" << node->ifelse->varCondition->getName() << ") begin" << endl;
        }else {
            *outFile << "\t\t\t\t\t\tif (" << node->ifelse->inputCondition->getName() << ") begin" << endl;
        }
        for (auto &i : node->ifelse->ifOperations) {
            if (i->getOperation() == IFELSE) {
                outputIfBlock(outFile, i);
            }else {
                *outFile << "\t\t\t\t\t\t\t" << i->toString() << endl;
            }
        }
        *outFile << "\t\t\t\t\tend" << endl;
    }
    
    return false;
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
 *
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
            scheduleOperations(unscheduled.front()->ifelse->ifOperations, unscheduled.front()->frame.min - 1, unscheduled.front()->frame.max);
            //TODO: Schedule else operations
            int time = 0;
            for(auto &i : unscheduled.front()->incomingVars){
                int temp = (i->fromOperation->scheduledTime + i->fromOperation->getCycleDelay()) - 1;
                if(temp > time){
                    time = temp;
                }
            }
            unscheduled.front()->ifelse->ifTimeSchedule = time - 1;
            scheduled.push_back(unscheduled.front());
            unscheduled.erase(unscheduled.begin());
        }
    }
    
    return true;
}

/**
 *
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
 *
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
 *
 */
void Module::resetScheduled(vector<Operation *> scheduled, bool ALAP){
    
    for(int i = 0; i < (unsigned)scheduled.size(); i++){
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
                    scheduled.at(i)->inVar[j]->outCycle = scheduled.at(i)->inVar[j]->permOutCycle;
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
 *
 */
bool Module::getASAPTimes(vector<Operation *> nodes, int startTime) {
    vector<Operation *> operationQueue = nodes;
    
    /* Loop until all variables and output delays have been updated */
    while(operationQueue.size() > 0){
        
        /* Iterate through all operations to update delays */
        bool opRemoved = false;
        for (int i = 0; i < (unsigned)nodes.size(); i++) {
            bool inCyclesCalculated = true;
            double tempCycle = 0;
            double maxInCycle = startTime;
            
            /* Get the current maximum delay from operation inputs */
            if(nodes.at(i)->getOperation() != IFELSE){
                for(int j = 0; j < NUM_INPUTS; j++){
                    if(nodes.at(i)->inVar[j] != NULL){
                        if(nodes.at(i)->inVar[j]->outCycle == -1){
                            inCyclesCalculated = false;
                            break;
                        }else{
                            if(maxInCycle < nodes.at(i)->inVar[j]->outCycle){
                                maxInCycle = nodes.at(i)->inVar[j]->outCycle;
                            }
                        }
                    }else{
                        continue;
                    }
                }
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
            
            /* Check if all delay dependencies have been calculated */
            if(inCyclesCalculated){
                
                /* Pass delay of operation output */
                nodes.at(i)->frame.min = maxInCycle + 1;
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
 *
 */
bool Module::getALAPTimes(vector<Operation *> nodes, int endTime) {
    vector<Operation *> operationQueue = nodes;
    
    for(auto &i : nodes){
        if(i->getOperation() != IFELSE){
            if(i->varNext != NULL){
                i->isUpdated = false;
            }
        }else{
            i->isUpdated = false;
        }
    }
    
    /* Loop until all variables and output delays have been updated */
    while(operationQueue.size() > 0){
        
        /* Iterate through all operations to update delays */
        bool opRemoved = false;
        for (int i = 0; i < (unsigned)nodes.size(); i++) {
            bool inCyclesCalculated = true;
            double tempCycle = 0;
            double maxInCycle = endTime + 1;
            
            if(nodes.at(i)->frame.max != 0){
                continue;
            }
            
            if(nodes.at(i)->getOperation() != IFELSE){
                if(nodes.at(i)->varNext != NULL){
                    for(auto &op : nodes.at(i)->varNext->toOperations){
                        if(!op->isUpdated || nodes.at(i)->varNext->outCycle == -1){
                            inCyclesCalculated = false;
                            break;
                        }
                    }
                    
                    if(inCyclesCalculated){
                        maxInCycle = nodes.at(i)->varNext->outCycle;
                    }
                }
            }else{
                for(auto &var : nodes.at(i)->outgoingVars){
                    for(auto &op : var->toOperations){
                        if((!op->isUpdated || var->outCycle == -1) && op->getOperation() != IFELSE){
                            inCyclesCalculated = false;
                            break;
                        }
                    }
                    
                    if(inCyclesCalculated && maxInCycle > var->outCycle){
                        maxInCycle = var->outCycle;
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
                for(int j = 0; j < (unsigned)operationQueue.size(); j++){
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
 *
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
    
    for(int i = 0; i < res_AddSub.size(); i++){
        double prop = 1.0 / (double)res_AddSub.at(i)->frame.getWidth();
        for(int j = 1; j <= Latency; j++){
            if(j >= res_AddSub.at(i)->frame.min && j <= res_AddSub.at(i)->frame.max){
                sum_AddSub.at(j - 1) += prop;
            }
        }
    }
    
    for(int i = 0; i < res_Mul.size(); i++){
        double prop = 1.0 / (double)res_Mul.at(i)->frame.getWidth();
        for(int j = 1; j <= Latency; j++){
            if(j >= res_Mul.at(i)->frame.min && j <= res_Mul.at(i)->frame.max){
                sum_Mul.at(j - 1) += prop;
            }
        }
    }
    for(int i = 0; i < res_Logic.size(); i++){
        double prop = 1.0 / (double)res_Logic.at(i)->frame.getWidth();
        for(int j = 1; j <= Latency; j++){
            if(j >= res_Logic.at(i)->frame.min && j <= res_Logic.at(i)->frame.max){
                sum_Logic.at(j - 1) += prop;
            }
        }
    }
    for(int i = 0; i < res_DivMod.size(); i++){
        double prop = 1.0 / (double)res_DivMod.at(i)->frame.getWidth();
        for(int j = 1; j <= Latency; j++){
            if(j >= res_DivMod.at(i)->frame.min && j <= res_DivMod.at(i)->frame.max){
                sum_DivMod.at(j - 1) += prop;
            }
        }
    }
}

/**
 *
 */
void Module::getTotalForces(vector<Operation *> nodes){
    
    getForces(nodes);
    
    for(int i = 0; i < (unsigned)nodes.size(); i++){
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
 *
 */
void Module::getForces(vector<Operation *> nodes) {
    
    /* Clear force variables */
    for(int i = 0; i < (unsigned)nodes.size(); i++){
        nodes.at(i)->selfForces.clear();
        nodes.at(i)->sucessorForces.clear();
        nodes.at(i)->predecessorForces.clear();
    }
    
    /* Get the self forces at each time in nodes time frame */
    for(int i = 1; i <= Latency; i++){
        for(int j = 0; j < (unsigned)nodes.size(); j++){
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
        for(int j = 0; j < (unsigned)nodes.size(); j++){
            if(nodes.at(j)->selfForces.at(i - 1) != NO_FORCE){
                nodes.at(j)->sucessorForces.at(i - 1) = getSuccessorForces(nodes.at(j), i);
                nodes.at(j)->predecessorForces.at(i - 1) = getPredecessorForces(nodes.at(j), i);
            }
        }
    }
    
}

/**
 *
 */
double Module::getSuccessorForces(Operation *node, int latency) {
    
    /* Node successor node */
    if(node->varNext == NULL){
        return 0.0;
    }
    
    /* Recursive call to get all successor forces */
    for(int i = 0; i < (unsigned)node->varNext->toOperations.size(); i++){
        Operation *suc = node->varNext->toOperations.at(i);
        if(latency >= suc->frame.min){
            return suc->selfForces.at(latency) + getSuccessorForces(suc, latency + 1);
        }
    }
    
    return 0.0;
}

/**
 *
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
 *
 */
void Module::scheduleNode(vector<Operation *> &scheduled ,vector<Operation *> &unscheduled){
    double minForce = 999;
//    if(!unscheduled.empty()){
//        minForce = unscheduled.at(0)->totalForce;
//    }
    
    /* Find node to be scheduled */
    int index = 0;
    for(int i = 0; i < (unsigned)unscheduled.size(); i++){
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

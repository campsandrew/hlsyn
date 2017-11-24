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
    scheduleOperations();
    
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
    } /* If else */
    else if(type.compare("if") == 0){
        line.erase(line.begin());
        lineType = IFELSE_TYPE;
    } /* For loop */
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
    } else if(lineType == IFELSE_TYPE || lineType == FORLOOP_TYPE) {
        
    } else {
        
        /* Checks to see if the first variable is an output type */
        bool assigned = false;
        string var = line.front();
        Operation *newOp = new Operation();
        for(int i = 0; i < (signed)this->outputs.size(); i++){
            /* Check if this is a not REG operation to output */
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
                this->operations.push_back(newOp);
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
                this->operations.push_back(newOp);
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
        this->operations.push_back(newOp);
        
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
    
    /* Prints operations */
    //for(int i = 0; i < (signed)this->operations.size(); i++){
    //    out << operations.at(i)->toString() << endl;
    //}
    //out << endl;
    
    /* Prints ending */
    out << "endmodule" << endl;
    out.close();
    
    return true;
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
bool Module::scheduleOperations() {
    getASAPTimes();
    
    /* Resets variables for calculating frames */
    for(auto &i : inputs){
        i->outCycle = -1;
    }
    for(auto &i : variables){
        i->outCycle = -1;
    }
    getALAPTimes();
    
    return true;
}

/**
 *
 */
bool Module::getASAPTimes() {
    vector<Operation *> operationQueue;
    
    /* Put all operations on queue */
    for (int i = 0; i < (signed)operations.size(); i++) {
        operationQueue.push_back(operations.at(i));
    }
    
    /* Loop until all variables and output delays have been updated */
    while((signed)operationQueue.size() > 0){
        
        /* Iterate through all operations to update delays */
        bool opRemoved = false;
        for (int i = 0; i < (signed)operations.size(); i++) {
            bool inCyclesCalculated = true;
            double tempCycle = 0;
            double maxInCycle = 0;
            
            /* Get the current maximum delay from operation inputs */
            for(int j = 0; j < NUM_INPUTS; j++){
                if(operations.at(i)->inVar[j] != NULL){
                    if(operations.at(i)->inVar[j]->outCycle == -1){
                        inCyclesCalculated = false;
                        break;
                    }else{
                        if(maxInCycle < operations.at(i)->inVar[j]->outCycle){
                            maxInCycle = operations.at(i)->inVar[j]->outCycle;
                            
                        }
                    }
                }else{
                    continue;
                }
            }
            
            /* Check if all delay dependencies have been calculated */
            if(inCyclesCalculated){
                
                /* Pass delay of operation output */
                operations.at(i)->timeASAP = maxInCycle + 1;
                tempCycle = maxInCycle + operations.at(i)->getCycleDelay();
                if(operations.at(i)->varNext != NULL){
                    operations.at(i)->varNext->outCycle = tempCycle;
                }else{
                    operations.at(i)->outNext->outCycle = tempCycle;
                }
                
                /* Remove currently calculated operation from the operation queue */
                for(int j = 0; j < (signed)operationQueue.size(); j++){
                    if(operations.at(i)->getOperation() == operationQueue.at(j)->getOperation()
                       && operations.at(i)->getOpID() == operationQueue.at(j)->getOpID()){
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
bool Module::getALAPTimes() {
    vector<Operation *> operationQueue;
    
    /* Put all operations on queue */
    for (int i = 0; i < (signed)operations.size(); i++) {
        operationQueue.push_back(operations.at(i));
    }
    
    /* Loop until all variables and output delays have been updated */
    while((signed)operationQueue.size() > 0){
        
        /* Iterate through all operations to update delays */
        bool opRemoved = false;
        for (int i = 0; i < (signed)operations.size(); i++) {
            bool inCyclesCalculated = true;
            double tempCycle = 0;
            double maxInCycle = Latency + 1;
            
            if(operations.at(i)->varNext != NULL){
                if(operations.at(i)->varNext->outCycle == -1){
                    inCyclesCalculated = false;
                    continue;
                }else{
                    maxInCycle = operations.at(i)->varNext->outCycle;
                }
            }
            
            /* Check if all delay dependencies have been calculated */
            if(inCyclesCalculated){
                
                /* Pass delay of operation output */
                tempCycle = maxInCycle - operations.at(i)->getCycleDelay();
                if(tempCycle < 1){
                    cout << "ERROR: Not enough cycles to schedule graph" << endl;
                    return false;
                }
                operations.at(i)->timeALAP = tempCycle;
                for(int j = 0; j < NUM_INPUTS; j++){
                    if(operations.at(i)->inVar[j] != NULL){
                        operations.at(i)->inVar[j]->outCycle = tempCycle;
                    }else{
                        if(operations.at(i)->inInput[j] != NULL){
                            operations.at(i)->inInput[j]->outCycle = tempCycle;
                        }
                    }
                }
                
                /* Remove currently calculated operation from the operation queue */
                for(int j = 0; j < (signed)operationQueue.size(); j++){
                    if(operations.at(i)->getOperation() == operationQueue.at(j)->getOperation()
                       && operations.at(i)->getOpID() == operationQueue.at(j)->getOpID()){
                        operationQueue.erase(operationQueue.begin() + j);
                        opRemoved = true;
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

bool Module::fds() {
    
    if (!latencyCheck(Latency)) {
        return false;
    }
    
}

/**
 * Checks if the entered latency will meet the operation cycle delay.
 */
bool Module::latencyCheck(int latency) {
    bool multiply = false, divide = false, etc = false, output = false;
    int i = 0;
    
    for (i = 0; i < operations.size(); i++) {
        if (operations.at(i)->getOperation() == MUL) {
            multiply = true;
        }
        else if (operations.at(i)->getOperation() == DIV) {
            divide = true;
        }
        else {
            etc = true;
        }
    }
    
    if (etc) {
        output = (latency < 1) ? false : true;
    }
    if (multiply) {
        output = (latency < 2) ? false : true;
    }
    if (divide) {
        output = (latency < 3) ? false : true;
    }
    
    return output;
}


/**
 * Delimeter function that splits a string at spaces and tabs and returns a vector of strings
 */
vector<string> Module::split(string const &input) {
    istringstream buffer(input);
    vector<string> ret((istream_iterator<string>(buffer)), istream_iterator<string>());
    return ret;
}

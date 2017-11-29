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
    bool ifFlag = false, elseFlag = false;
    int nestedIfIndex;
    
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
        vector<string> lines = split(line);
        Type lineType;
        
        /* Skip empty empty lines */
        if(!lines.size()) continue;
        
        /* Parse first word of each line */
        string type = lines.front();
        if(type.compare("input") == 0){
            lines.erase(lines.begin());
            lineType = INPUT_TYPE;
        } /* Output */
        else if(type.compare("output") == 0){
            lines.erase(lines.begin());
            lineType = OUTPUT_TYPE;
        } /* Variable */
        else if(type.compare("variable") == 0){
            lines.erase(lines.begin());
            lineType = VARIABLE_TYPE;
        } /* If type */
        else if(type.compare("if") == 0){
            lines.erase(lines.begin());
            lineType = IF_TYPE;
        } /* Else type */
        else if (type.compare("else") == 0) {
            lines.erase(lines.begin());
            lineType = ELSE_TYPE;
        } /* For loop */
        else if(type.compare("for") == 0){
            lines.erase(lines.begin());
            lineType = FORLOOP_TYPE;
        } /* Operation or invalid line  */
        else {
            /* Checks for invalid line */
            if(lines.at(1).compare("=") != 0){
                return false;
            }
            
            lineType = OPERATION_TYPE;
        }
        
        /* Performs correct line parsing technique for each line type */
        int size;
        if(lineType == INPUT_TYPE || lineType == OUTPUT_TYPE || lineType == VARIABLE_TYPE) {
            
            /* Invalid line */
            if(!lines.size()){
                return false;
            }
            
            /* Get the sign type and size of variable */
            bool usigned = getDataType(lines.front(), &size);
            if(size < 0){
                cout << "Invalid data type: " << line.front() << endl;
                return false;
            }
            
            /* Invalid line */
            lines.erase(lines.begin());
            if(!lines.size()){
                return false;
            }
            
            /* Loop until end of string */
            while(lines.size() > 0){
                string name = lines.front();
                
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
                
                lines.erase(lines.begin());
            }
        } else if(lineType == IF_TYPE || lineType == ELSE_TYPE || lineType == FORLOOP_TYPE) {
            switch(lineType){
                case IF_TYPE: {
                    Operation *currOp = NULL;
                    int currIndex = -1, inIndex = -1, outIndex = -1, varIndex = -1;
                    
                    currOp = new Operation();
                    currOp->setOperation(IF);
                    currOp->setOpID(getID(IF));
                    /* SKIPPING WIDTH GENERATION FOR IF OPERATION,
                     POSSIBLY MOVING WIDTH GENERATION AT END OF IF STATEMENT PARSING */
                    operations.push_back(currOp);
                    currIndex = operations.size() - 1;
                    currOp->conditional = true;
                    
                    string condition = lines.at(1);
                    /* Check if variable in condition exists */
                    /* If condition was found, add variable to node based off its type */
                    if (conditionCheck(condition, &inIndex, &varIndex, &outIndex)) {
                        if (varIndex != -1) {
                            currOp->inVar[0] = variables.at(varIndex);
                        }
                        else if (inIndex != -1) {
                            currOp->inInput[0] = inputs.at(inIndex);
                        }
                    } /* If not found, failure occurs since variable has to exist */
                    else {
                        cout << "If condition variable " << condition << " does not exist (or something went wrong there). Debug this." << endl;
                        return -9;
                    }
                    
                    /* Grab contents within if statement */
                    getline(in, line);
                    lines = split(line);
                    
                    /* Empty line, continue searching until an entry is found */
                    while (!lines.size()) {
                        getline(in, line);
                        lines = split(line);
                    }
                    
                    condition = lines.front();
                    
                    while(condition.compare("}")) {
                        /* Empty line */
                        if (!lines.size()) continue;
                        
                        /* Deal with a nested if statement */
                        if (!condition.compare("if")) {
                            /* Add nested if statement to dedicated "next if statement" vector */
                            nestedIfIndex = nestedIf(&in, &lines);
                            currOp->nextIf.push_back(operations.at(nestedIfIndex));
                            operations[nestedIfIndex]->inOperations.push_back(currOp);
                        } /* Deal with operations inside of the current if statement */
                        else {
                            /* Check existing updated variable/output like before, but parse the operation when found */
                            outIndex = -1, varIndex = -1, inIndex = -1;
                            if (conditionCheck(condition, &inIndex, &varIndex, &outIndex)) {
                                if (varIndex != -1) {
                                    if(!parseOperation(&in, &lines)) {
                                        return -10;
                                    }
                                }
                                else if (outIndex != -1) {
                                    if(!parseOperation(&in, &lines)) {
                                        return -11;
                                    }
                                }
                                if (ifFlag && elseFlag) {
                                    operations[currIndex]->nextElse.push_back(operations.at(operations.size() - 1));
                                }
                                /* Adding if statement as previous operation for current operation */
                                operations.at(operations.size() - 1)->inOperations.push_back(operations.at(currIndex));
                                /* Add current operation as next operation for if statement operation */
                                currOp->nextIf.push_back(operations.at(operations.size() - 1));
                            } /* Updated variable/output should be defined, if not program fails */
                            else {
                                cout << "Variable " << condition << " does not exist (or something went wrong there). Debug this." << endl;
                                return -15;
                            }
                        }
                        ifFlag = true;
                        
                        /* Grab next line */
                        getline(in, line);
                        lines = split(line);
                        condition = lines.front();
                    }
                    if (currIndex == -1) {
                        cout << "Error with processing if statement" << endl;
                        return -28;
                    }
                    ifFlag = true;
                }
                    break;
                case ELSE_TYPE: {
                    elseFlag = true;
                    int nextIndex = -1, currIndex = -1, inIndex = -1, outIndex = -1, varIndex = -1;
                    
                    string condition = lines.at(1);
                    lines.erase(lines.begin());
                    /* Check if variable in condition exists */
                    /* If condition was found, add variable to node based off its type */
                    if (conditionCheck(condition, &inIndex, &varIndex, &outIndex)) {
                        
                        
                        
                    }
                    else {
                        cout << "If condition variable does not exist (or something went wrong there). Debug this." << endl;
                        in.close();
                        return -20;
                    }
                    
                } break;
                case FORLOOP_TYPE: {
                    
                }
                    break;
                    
                default: {
                    
                }
                    break;
            }
        } else {
            if(!parseOperation(&in, &lines)) {
                return false;
            }
        }
    }
    
    in.close();
    
    return true;
}

/**
 *
 */
bool Module::conditionCheck(string name, int *inIndex, int *varIndex, int *outIndex) {
    bool result = false;
    
    for (int i = 0; i < inputs.size(); ++i) {
        if (!inputs.at(i)->getName().compare(name)) {
            *inIndex = i;
            result = true;
            break;
        }
    }
    for (int i = 0; i < outputs.size(); ++i) {
        if (!outputs.at(i)->getName().compare(name)) {
            *outIndex = i;
            result = true;
            break;
        }
    }
    for (int i = 0; i < variables.size(); ++i) {
        if (!variables.at(i)->getName().compare(name)) {
            *varIndex = i;
            result = true;
            break;
        }
    }
    return result;
}

/**
 *
 */
bool Module::parseOperation(fstream *inFile, vector<string> *line) {
    /* Checks to see if the first variable is an output type */
    bool assigned = false;
    string var = line->front();
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
    line->erase(line->begin());
    line->erase(line->begin());
    if(!line->size()){
        return false;
    }
    
    /* Assigns first Input or Variable to operation */
    assigned = false;
    var = line->front();
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
    line->erase(line->begin());
    if(!line->size()){
        return false;
    }
    
    /* Gets operation type */
    var = line->front();
    if(var.compare("+") == 0){
        line->erase(line->begin());
        var = line->front();
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
        line->erase(line->begin());
        var = line->front();
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
        line->erase(line->begin());
    }
    
    /* Assign out Input or Variable to operation */
    assigned = false;
    var = line->front();
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
        
        line->erase(line->begin());
        line->erase(line->begin());
        
        /* Assign out Input or Wire to operation */
        assigned = false;
        var = line->front();
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
    return true;
}



/**
 *
 */
int Module::nestedIf(fstream *inFile, vector<string> *line) {
    Operation *currOp = NULL;
    int nestedIfIndex = -1, currOpIndex = -1, inIndex = -1, outIndex = -1, varIndex = -1;
    bool varCheck = false;
    string tempLine;
    
    currOp = new Operation();
    currOp->setOperation(IF);
    currOp->setOpID(getID(IF));
    /* SKIPPING WIDTH GENERATION FOR IF OPERATION,
     POSSIBLY MOVING WIDTH GENERATION AT END OF IF STATEMENT PARSING */
    operations.push_back(currOp);
    currOpIndex = operations.size() - 1;
    currOp->conditional = true;
    
    string condition = line->at(2);
    
    if (conditionCheck(condition, &inIndex, &varIndex, &outIndex)) {
        if (varIndex != -1) {
            currOp->inVar[0] = variables.at(varIndex);
        }
        else if (inIndex != -1) {
            currOp->inInput[0] = inputs.at(inIndex);
        }
    }
    else {
        cout << "If condition variable " << condition << " does not exist (or something went wrong there). Debug this." << endl;
        return -1;
    }
    
    /* Grab contents within if statement */
    getline(*inFile, tempLine);
    *line = split(tempLine);
    
    /* Empty line, continue searching until an entry is found */
    while (!line->size()) {
        getline(*inFile, tempLine);
        *line = split(tempLine);
    }
    
    condition = line->front();
    
    while (condition.compare("}")) {
        /* Empty line */
        if (!line->size()) continue;
        
        /* Deal with a nested if statement */
        if (!condition.compare("if")) {
            /* Add nested if statement to dedicated "next if statement" vector */
            nestedIfIndex = nestedIf(inFile, line);
            currOp->nextIf.push_back(operations.at(nestedIfIndex));
            operations[nestedIfIndex]->inOperations.push_back(currOp);
        } /* Deal with else statement that for if/else scenario */
        else if (!condition.compare("else")) {
            outIndex = -1, inIndex = -1, varIndex = -1;
            if (conditionCheck(condition, &inIndex, &varIndex, &outIndex)) {
                if (varIndex != -1) {
                    if(!parseOperation(inFile, line)) {
                        return -10;
                    }
                }
                else if (outIndex != -1) {
                    if(!parseOperation(inFile, line)) {
                        return -11;
                    }
                }
                
                operations.at(operations.size() - 1)->inOperations.push_back(operations.at(nestedIfIndex));
                operations[nestedIfIndex]->nextElse.push_back(operations.at(operations.size() - 1));
                
            }
            else {
                cout << "If condition variable " << condition << " does not exist (or something went wrong there). Debug this." << endl;
                return -1;
            }
            
        }
        else {
            outIndex = -1, inIndex = -1, varIndex = -1;
            if (conditionCheck(condition, &inIndex, &varIndex, &outIndex)) {
                if (varIndex != -1) {
                    if(!parseOperation(inFile, line)) {
                        return -10;
                    }
                }
                else if (outIndex != -1) {
                    if(!parseOperation(inFile, line)) {
                        return -11;
                    }
                }
                /* Adding if statement as previous operation for current operation */
                operations.at(operations.size() - 1)->inOperations.push_back(operations.at(currOpIndex));
                /* Add current operation as next operation for if statement operation */
                currOp->nextIf.push_back(operations.at(operations.size() - 1));
            }
            else {
                cout << "If condition variable " << condition << " does not exist (or something went wrong there). Debug this." << endl;
                return -1;
            }
            
            
        }
        
        /* Grab next line */
        getline(*inFile, tempLine);
        *line = split(tempLine);
        condition = line->front();
    }
    return currOpIndex;
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
    
    /* Prints Rst and state transition block */
    out << "\talways @(posedge Clk) begin" << endl;
    out << "\t\tif(Rst) begin" << endl;
    out << "\t\t\tState <= Wait;" << endl;
    //TODO: Set all regs to 0
    out << "\t\tend" << endl;
    out << "\t\telse begin" << endl;
    out << "\t\t\tState <= StateNext;" << endl;
    out << "\t\tend" << endl;
    out << "\t" << endl;
    
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
    vector<Operation *> unscheduled = operations;
    vector<Operation *> scheduled;
    
    /* Perform force directed scheduling */
    while(unscheduled.size() != 0){
        if(!getTimeFrames(scheduled, unscheduled)){
            return false;
        }
        getTypePropabilities();
        getTotalForces(unscheduled);
        scheduleNode(scheduled, unscheduled); // Removes scheduled node
    }
    
    return true;
}

/**
 *
 */
bool Module::getTimeFrames(vector<Operation *> scheduled ,vector<Operation *> unscheduled){
    
    /* Calculate ASAP */
    resetUnscheduled();
    resetScheduled(scheduled);
    if(!getASAPTimes(unscheduled)){
        return false;
    }
    
    /* Calculate ALAP */
    resetUnscheduled();
    resetScheduled(scheduled);
    if(!getALAPTimes(unscheduled)){
        return false;
    }
    
    return true;
}

/**
 *
 */
void Module::resetUnscheduled(){
    
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
}

/**
 *
 */
void Module::resetScheduled(vector<Operation *> scheduled){
    
    for(int i = 0; i < (unsigned)scheduled.size(); i++){
        if(scheduled.at(i)->varNext != NULL){
            scheduled.at(i)->varNext->outCycle = (scheduled.at(i)->scheduledTime + scheduled.at(i)->getCycleDelay()) - 1;
            scheduled.at(i)->varNext->isScheduled = true;
        }else{
            scheduled.at(i)->outNext->outCycle = (scheduled.at(i)->scheduledTime + scheduled.at(i)->getCycleDelay()) - 1;
            scheduled.at(i)->outNext->isScheduled = true;
        }
        for(int j = 0; j < NUM_INPUTS; j++){
            if(scheduled.at(i)->inVar[j] != NULL){
                scheduled.at(i)->inVar[j]->outCycle = scheduled.at(i)->scheduledTime;
                scheduled.at(i)->inVar[j]->isScheduled = true;
            }else{
                if(scheduled.at(i)->inInput[j] != NULL){
                    scheduled.at(i)->inInput[j]->outCycle = scheduled.at(i)->scheduledTime;
                    scheduled.at(i)->inInput[j]->isScheduled = true;
                }
            }
        }
    }
}

/**
 *
 */
bool Module::getASAPTimes(vector<Operation *> nodes) {
    vector<Operation *> operationQueue = nodes;
    
    /* Loop until all variables and output delays have been updated */
    while(operationQueue.size() > 0){
        
        /* Iterate through all operations to update delays */
        bool opRemoved = false;
        for (int i = 0; i < (signed)nodes.size(); i++) {
            bool inCyclesCalculated = true;
            double tempCycle = 0;
            double maxInCycle = 0;
            
            /* Get the current maximum delay from operation inputs */
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
            
            /* Check if all delay dependencies have been calculated */
            if(inCyclesCalculated){
                
                /* Pass delay of operation output */
                nodes.at(i)->frame.min = maxInCycle + 1;
                tempCycle = maxInCycle + nodes.at(i)->getCycleDelay();
                if(nodes.at(i)->varNext != NULL){
                    nodes.at(i)->varNext->outCycle = tempCycle;
                }else{
                    nodes.at(i)->outNext->outCycle = tempCycle;
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
bool Module::getALAPTimes(vector<Operation *> nodes) {
    vector<Operation *> operationQueue = nodes;
    
    /* Loop until all variables and output delays have been updated */
    while(operationQueue.size() > 0){
        
        /* Iterate through all operations to update delays */
        bool opRemoved = false;
        for (int i = 0; i < (unsigned)nodes.size(); i++) {
            bool inCyclesCalculated = true;
            double tempCycle = 0;
            double maxInCycle = Latency + 1;
            
            if(nodes.at(i)->varNext != NULL){
                if(nodes.at(i)->varNext->outCycle == -1){
                    inCyclesCalculated = false;
                    continue;
                }else{
                    maxInCycle = nodes.at(i)->varNext->outCycle;
                }
            }
            
            /* Check if all delay dependencies have been calculated */
            if(inCyclesCalculated){
                
                /* Pass delay of operation output */
                tempCycle = maxInCycle - nodes.at(i)->getCycleDelay();
                if(tempCycle < 1){
                    cout << "ERROR: Not enough cycles to schedule graph" << endl;
                    return false;
                }
                nodes.at(i)->frame.max = tempCycle;
                for(int j = 0; j < NUM_INPUTS; j++){
                    if(nodes.at(i)->inVar[j] != NULL){
                        nodes.at(i)->inVar[j]->outCycle = tempCycle;
                    }else{
                        if(nodes.at(i)->inInput[j] != NULL){
                            nodes.at(i)->inInput[j]->outCycle = tempCycle;
                        }
                    }
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
                        for(int k = nodes.at(j)->frame.min; k <= operations.at(j)->frame.max; k++){
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
    double minForce;
    if(!unscheduled.empty()){
        minForce = unscheduled.at(0)->totalForce;
    }
    
    /* Find node to be scheduled */
    int index = 0;
    for(int i = 0; i < (unsigned)unscheduled.size(); i++){
        if(minForce > unscheduled.at(i)->totalForce){
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

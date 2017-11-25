//
//  node.cpp
//  hlsyn
//
//  Created by Jason Tran on 11/25/17.
//

#include "node.h"

node::node(){
    nodeNum = 0;
    delay = 0;
    visited = 'w';
    cycleAllowed = 0;
    timeAsap = -1;
    timeAlap = -1;
    scheduled = false;
    condition = false;
}

node::node(string n, int num, vector<type *> inputs, vector<type *> outputs){
    operation = n;
    nodeNum = num;
    this->inputs = inputs;
    this->outputs = outputs;
    visited = 'w';
    delay = 0;
    cycleAllowed = 0;
    timeAsap = -1;
    timeAlap = -1;
    scheduled = false;
    condition = false;
}

std::string node::getOperation(){
    return operation;
}

void node::setOperation(string op){
    operation = op;
}

int node::getNodeNum(){
    return nodeNum;
}

void node::setNodeNum(int num){
    nodeNum = num;
}

vector<type *> node::getInputs(){
    return inputs;
}

void node::setInputs(vector<type*> inputs){
    this->inputs = inputs;
}

vector<type *> node::getOutputs(){
    return outputs;
}

void node::setOutputs(vector<type *> outputs){
    this->outputs = outputs;
}

vector<node *> node::getPreceedingNodes(){
    return preceedingNodes;
}

void node::setPreceedingNodes(vector<node *> preceeding){
    preceedingNodes = preceeding;
}

vector<node *> node::getSucceedingNodes(){
    return succeedingNodes;
}

void node::setSucceedingNodes(vector<node *> succeeding){
    succeedingNodes = succeeding;
}

vector<node *> node::getSucceedingIf(){
    return succeedingIf;
}

void node::setSucceedingIf(vector<node *> succeedingIf){
    this->succeedingIf = succeedingIf;
}

vector<node *> node::getSucceedingElse(){
    return succeedingElse;
}

void node::setSucceedingElse(vector<node *> succeedingElse){
    this->succeedingElse = succeedingElse;
}


vector<double> node::getOperationProbability(){
    return operationProbability;
}

void node::setOperationProbability(vector<double> prob){
    operationProbability = prob;
}

vector<double> node::getTotalForces(){
    return totalForce;
}

void node::setTotalForces(vector<double> total){
    totalForce = total;
}

char node::getVisted(){
    return visited;
}

void node::setVisted(char visited){
    this->visited = visited;
}

int node::getDelay(){
    return delay;
}

void node::setDelay(int delay){
    this->delay = delay;
}

int node::getCycleAllowed(){
    return cycleAllowed;
}

void node::setCycleAllowed(int cycles){
    cycleAllowed = cycles;
}

int node::getTimeAsap(){
    return timeAsap;
}

void node::setTimeAsap(int timeAsap){
    this->timeAsap = timeAsap;
}

int node::getTimeAlap(){
    return timeAlap;
}

void node::setTimeAlap(int timeAlap){
    this->timeAlap = timeAlap;
}

int node::getTimeFds(){
    return timeFds;
}

void node::setTimeFds(int timeFds){
    this->timeFds = timeFds;
}

bool node::getScheduled(){
    return scheduled;
}

void node::setScheduled(bool scheduled){
    this->scheduled = scheduled;
}

bool node::getConditional(){
    return condition;
}

void node::setConditional(bool condition){
    this->condition = condition;
}


void node::appendIn(type* input){
    inputs.push_back(input);
}

void node::appendOut(type* output){
    outputs.push_back(output);
}

void node::appendPreceeding(node* preceeding){
    preceedingNodes.push_back(preceeding);
}

void node::appendSucceeding(node* succeeding){
    succeedingNodes.push_back(succeeding);
}

void node::appendSucceedingIf(node* succeedingIf){
    this->succeedingIf.push_back(succeedingIf);
}

void node::appendSucceedingElse(node* succeedingElse){
    this->succeedingElse.push_back(succeedingElse);
}

void node::OperationProbabililty(int latency){
    if (operationProbability.size() > 0) {
        operationProbability.clear();
    }
    
    for (int i = 0; i < latency; ++i) {
        if (i >= timeAsap && i <= timeAlap) {
            if ((timeAlap - timeAsap) != 0) {
                operationProbability.push_back(1 / (timeAlap - timeAsap + 1));
            }
            else {
                operationProbability.push_back(1);
            }
        }
        else {
            operationProbability.push_back(0);
        }
    }
}

void node::SelfForce(vector<double> dist_type){
    double tempSF = 0, currTemp = 0;
    
    if (selfForce.size() > 0) {
        selfForce.clear();
    }
    
    /* Intializing in accordance to total time cycles. */
    for (int i = 0; i < operationProbability.size(); ++i) {
        selfForce.push_back(NO_FORCE);
    }
    
    for (int i = 0; i < operationProbability.size(); ++i) {
        tempSF = 0;
        /* Check if current time is within ASAP and ALAP time frame*/
        if (i >= timeAsap && i <= timeAlap) {
            /* Start self force assignment */
            currTemp = dist_type.at(i) * (1 - operationProbability.at(i));
            for (int j = timeAsap; j <= timeAlap; ++j) {
                /* If operation's cycle delay is outside of its frame, assign accordingly */
                if (j != i) {
                    tempSF += dist_type.at(j) * (0 - operationProbability.at(j));
                }
                /* If it is actually scheduled at within the time frame, assign accordingly */
                else {
                    tempSF += currTemp;
                }
            }
            // Assign self force here
            selfForce[i] = tempSF;
        }
    }
}

void node::PredecessorForce(vector<double> dist_mult, vector<double> dist_addSub, vector<double> dist_modDiv, vector<double> dist_logic) {
    
}

void node::SuccessorForce(vector<double> dist_mult, vector<double> dist_addSub, vector<double> dist_modDiv, vector<double> dist_logic) {
    
}

void node::TotalForce(){
    
}

void node::assignDelay() {
    if (!operation.compare("+") || !operation.compare("-")) {
        delay = DELAY_ADDSUB;
    }
    else if (!operation.compare("/") || !operation.compare("%")) {
        delay = DELAY_DIVMOD;
    }
    else if (!operation.compare("*")) {
        delay = DELAY_MULT;
    }
    else {
        delay = DELAY_LOGIC;
    }
}

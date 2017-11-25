//
//  node.h
//  hlsyn_project
//
//  Created by Jason Tran on 11/25/17.
//

#ifndef NODE_H
#define NODE_H

#include "type.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <algorithm>

#define DELAY_DIVMOD 3
#define DELAY_MULT 2
#define DELAY_ADDSUB 1
#define DELAY_LOGIC 1
#define NO_FORCE -999

using namespace std;

class node {
private:
    vector<node*> preceedingNodes;  // nodes that are before current node
    vector<node*> succeedingNodes;  // nodes that are after current node
    vector<node*> succeedingIf;     // if nodes that are after current node
    vector<node*> succeedingElse;   // else nodes that are after current node
    vector<type*> inputs;   // components that are inputs
    vector<type*> outputs;  // components that are outputs
    vector<double> operationProbability;
    vector<double> selfForce;
    vector<double> predecessorForce;
    vector<double> successorForce;
    vector<double> totalForce;
    string operation;
    int nodeNum;
    char visited;
    int delay;
    int cycleAllowed;   // for delay of specific nodes
    int timeFds;
    int timeAsap;
    int timeAlap;
    bool condition;     // if currrent node is a conditional statement
    bool scheduled;     // if current node has been scheduled already
public:
    node();
    node(string n, int num, vector<type*> inputs, vector<type*> outputs);
    string getOperation();
    void setOperation(string op);
    int getNodeNum();
    void setNodeNum(int num);
    vector<type*> getInputs();
    void setInputs(vector<type*> inputs);
    vector<type*> getOutputs();
    void setOutputs(vector<type*> _outputs);
    vector<node*> getPreceedingNodes();
    void setPreceedingNodes(vector<node*> preceeding);
    vector<node*> getSucceedingNodes();
    void setSucceedingNodes(vector<node*> succeeding);
    vector<node*> getSucceedingIf();
    void setSucceedingIf(vector<node*> succeedingIf);
    vector<node*> getSucceedingElse();
    void setSucceedingElse(vector<node*> succeedingElse);
    vector<double> getOperationProbability();
    void setOperationProbability(vector<double> prob);
    vector<double> getTotalForces();
    void setTotalForces(vector<double> total);
    char getVisted();
    void setVisted(char visited);
    int getDelay();
    void setDelay(int ddelay);
    int getCycleAllowed();
    void setCycleAllowed(int cycles);
    int getTimeAsap();
    void setTimeAsap(int timeAsap);
    int getTimeAlap();
    void setTimeAlap(int timeAlap);
    int getTimeFds();
    void setTimeFds(int timeFds);
    bool getScheduled();
    void setScheduled(bool scheduled);
    bool getConditional();
    void setConditional(bool conditional);
    void appendIn(type* input);
    void appendOut(type* output);
    void appendPreceeding(node* preceeding);
    void appendSucceeding(node* succeeding);
    void appendSucceedingIf(node* succeedingIf);
    void appendSucceedingElse(node* succeedingElse);
    void OperationProbabililty(int latency);
    void SelfForce(std::vector<double> typeDistribution);
    void PredecessorForce(vector<double> dist_mul, vector<double> dist_addSub, vector<double> dist_modDiv, vector<double> dist_logic);
    void SuccessorForce(vector<double> dist_mul, vector<double> dist_addSub, vector<double> dist_modDiv, vector<double> dist_logic);
    void TotalForce();
    void assignDelay();
    
};

#endif // node_h

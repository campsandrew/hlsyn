/*
Students: Brett Bushnell (Undergrad), Matt Dzurick (Grad)
Date Create: 11/15/2016
Assignment: 3
File: Node.h
Description: Node Class for hlsyn program
*/

#ifndef NODE_H
#define NODE_H

#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<cstring>
#include<algorithm>

#include"DataType.h"

//Delays of type of operation (cycles)
#define MULT_DELAY 2
#define DIV_MOD_DELAY 3
#define LOGIC_DELAY 1
#define ADD_SUB_DELAY 1

//when calculating total forces
#define NO_FORCE_IN_TIME_CYCLE 9999

class Node {
private:
	std::string operation;
	int nodeNum;
	std::vector<DataType*> _componentInputs;
	std::vector<DataType*> _componentOutputs;
	std::vector<Node*> _previousNodes;
	std::vector<Node*> _nextNodes;
	std::vector<Node*> _nextIfNodes;
	std::vector<Node*> _nextElseNodes;
	std::vector<double> _operationProbability;
	std::vector<double> _selfForce;
	std::vector<double> _predecessorForce;
	std::vector<double> _sucessorForce;
	std::vector<double> _totalForce;
	int delay;
	int cycleAllowed;   //this accounts for the delay of certain nodes
	int asapTime;
	int alapTime;
	int fdsTime;
	char visited;
	bool scheduled;
	bool conditional;


public:
	/*Constructors*/
	Node();
	Node(std::string n, int num, std::vector<DataType*> _inputs, std::vector<DataType*> _outputs);

	/*Methods*/
	std::string getOperation();
	void setOperation(std::string o);
	int getNodeNum();
	void setNodeNum(int num);
	std::vector<DataType*> getInputs();
	void setInputs(std::vector<DataType*> _inputs);
	std::vector<DataType*> getOutputs();
	void setOutputs(std::vector<DataType*> _outputs);
	std::vector<Node*> getPreviousNodes();
	void setPreviousNodes(std::vector<Node*> pN);
	std::vector<Node*> getNextNodes();
	void setNextNodes(std::vector<Node*> nN);
	std::vector<Node*> getNextIfNodes();
	void setNextIfNodes(std::vector<Node*> nIN);
	std::vector<Node*> getNextElseNodes();
	void setNextElseNodes(std::vector<Node*> nEN);
	std::vector<double> getOperationProbability();
	void setOperationProbability(std::vector<double> oP);
	std::vector<double> getTotalForces();
	void setTotalForces(std::vector<double> tF);
	char getVisted();
	void setVisted(char v);
	int getDelay();
	void setDelay(int d);
	int getCycleAllowed();
	void setCycleAllowed(int cA);
	int getAsapTime();
	void setAsapTime(int asT);
	int getAlapTime();
	void setAlapTime(int alT);
	int getFDSTime();
	void setFDSTime(int fT);
	bool getScheduled();
	void setScheduled(bool s);
	bool getConditional();
	void setConditional(bool c);
	void addInput(DataType* newInput);
	void addOutput(DataType* newOutput);
	void addPreviousNode(Node* previousNode);
	void addNextNode(Node* nextNode);
	void addNextIfNode(Node* nextNode);
	void addNextElseNode(Node* nextNode);
	void assignOperationProbability(int latency);
	void calculateSelfForce(std::vector<double> typeDistribution);
	void calculatePredecessorForce(std::vector<double> multDistribution, std::vector<double> addSubDistribution, std::vector<double> modDivDistribution, std::vector<double> logicDistribution);
	void calculateSuccessorForce(std::vector<double> multDistribution, std::vector<double> addSubDistribution, std::vector<double> modDivDistribution, std::vector<double> logicDistribution);
	void calculateTotalForce();
	void assignDelay();

};

#endif // NODE_H
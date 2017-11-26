/*
Students: Brett Bushnell (Undergrad), Matt Dzurick (Grad)
Date Create: 11/14/2016
Assignment: 3
File: HLSM.h
Description: HLSM Class for hlsyn program
*/

#ifndef HLSM_H
#define HLSM_H

#include <ctime>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstring>
#include <algorithm>

#include "DataType.h"
#include "Input.h"
#include "Output.h"
#include "Variable.h"
#include "Node.h"
#include "State.h"

#define DATAWIDTH_1 1
#define DATAWIDTH_2 2
#define DATAWIDTH_8 8 
#define DATAWIDTH_16 16 
#define DATAWIDTH_32 32
#define DATAWIDTH_64 64

class HLSM {

private:
	std::string outputCircuit;
	std::vector<Input*> _inputs;
	std::vector<Output*> _outputs;
	std::vector<Variable*> _variables;
	std::vector<Node*> _nodes;
	std::vector<double> _multDistribution;
	std::vector<double> _addSubDistribution;
	std::vector<double> _logicDistribution;
	std::vector<double> _modDivDistribution;
	std::vector<std::vector< Node*> > _asapSchedule;
	std::vector<std::vector< Node*> > _alapShcedule;
	std::vector<std::vector< Node*> > _forceDirectedSchedule;
	std::vector<State> _states;
	
public:
	/*Constructors*/
	HLSM();

	/*Methods*/
	int asapScheduleSize();
	bool readFile(char* fileName);
	void createUnscheduledGraph();
	bool scheduleGraph(int latency);
	void asapSchedule(int latency);
	bool alapSchedule(int latency);
	void calculateOperationProbability(int latency);
	void calculateTypeDistributionProbability(int latency);
	void calculateNodeSelfForces();
	void calculateNodePredecessorSuccessorForces();
	void calculateNodeTotalForces();
	void selectNodeToSchedule();
	bool writeToFile(char* fileName);
	bool writeStates(std::ofstream *outputFile);
	//void determineCriticalPath();
	//void visitComponent(DatapathComponent * compoenent, double currTime, double* cP);
	void createNewInputVariable(std::string checkString, int dataWidthIndex);
	void createNewOutputVariable(std::string checkString, int dataWidthIndex);
	void createNewVariableVariable(std::string checkString, int dataWidthIndex);
	// void createNewRegisterVariable(std::string checkString, int dataWidthIndex);
	void createNewInput(std::string name, bool sign, int dataWidth);
	void createNewOutput(std::string name, bool sign, int dataWidth);
	void createNewVariable(std::string name, bool sign, int dataWidth);
	// void createNewRegister(std::string name, bool sign, int dataWidth);
	bool checkVariable(std::string checkName, int* outputIndex, int* inputIndex, int* wireIndex);
	bool determineOperation(std::string line, DataType* output);
	void createNewNode(std::string name, int num, std::vector<DataType*> _Cinputs, std::vector<DataType*> _Coutputs);
	bool checkValidSymbol(std::string checkSymbol, std::string* dPType);
	bool writeInputsToFile(std::ofstream *outputFile, int i, int j);
	bool writeVarsToFile(std::ofstream *outputFile);
	bool checkIfComment(std::string checkString);
	void clearAlgothrimVectors();
	int ifCheckStringIsIf(std::ifstream *inputFile, std::string checkString);
	int createNestedIf(std::ifstream *inputFile, std::string checkString);
	void createStates();
	void createNewState();
	bool checkLatency(int latency);
};

#endif // HLSM_H
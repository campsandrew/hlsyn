/*
Students: Brett Bushnell (Undergrad), Matt Dzurick (Grad)
Date Create: 10/3/2016
Assignment: 3
File: HLSM.cpp
Description: HLSM class definitions for hlysn program
*/

#include "HLSM.h"
using namespace std;

HLSM::HLSM() {

}

int HLSM::asapScheduleSize()
{
	return _asapSchedule.size();
}

bool HLSM::readFile(char* fileName)
{
	ifstream inputFile;
	string checkString;
	string checkChar;
	bool foundDataType;
	bool elseFlag = false;
	bool flagIf = false;
	const std::string validDataTypes[12] = { "Int1", "Int2", "Int8", "Int16", "Int32", "Int64",
		"UInt1", "UInt2", "UInt8", "UInt16", "UInt32","UInt64" };
	int i = 0;
	int j = 0;
	int componentOutputIndex = -1;
	int componentInputIndex = -1;
	int componentWireIndex = -1;
	int ifNodeIndex = -1;

	inputFile.open(fileName);

	//checking if open
	if (!inputFile.is_open()) {
		// cout << "Could not open " << fileName << endl;
		return false;
	}

	//while there is something on the line
	while (!inputFile.eof()) {
		inputFile >> checkString;

		//first check if input or output or wire
		if (!checkString.compare("")) {
			break;
		}
		if (!checkString.compare("input")) {
			//getline(inputFile, checkString);
			inputFile >> checkString;

			foundDataType = false;

			for (i = 0; i < 12; ++i) {
				//go through valid data types and see which one it is
				if (!checkString.compare(validDataTypes[i])) {
					foundDataType = true;
					break;
				}
			}

			if (foundDataType) {
				getline(inputFile, checkString);
				createNewInputVariable(checkString, i);
			}
			else {
				cout << "Error: Invalid Data type: " << checkString << " Exiting Program." << endl;
				inputFile.close();
				return false;
			}
		}
		else if (!checkString.compare("output")) {
			inputFile >> checkString;

			foundDataType = false;

			for (i = 0; i < 12; ++i) {
				//go through valid data types and see which one it is
				if (!checkString.compare(validDataTypes[i])) {
					foundDataType = true;
					break;
				}
			}

			if (foundDataType) {
				getline(inputFile, checkString);
				createNewOutputVariable(checkString, i);
			}
			else {
				cout << "Error: Invalid Data type: " << checkString << " Exiting Program." << endl;
				inputFile.close();
				return false;
			}
		}
		else if (!checkString.compare("variable") || !checkString.compare("register")) {
			inputFile >> checkString;

			foundDataType = false;

			for (i = 0; i < 12; ++i) {
				//go through valid data types and see which one it is
				if (!checkString.compare(validDataTypes[i])) {
					foundDataType = true;
					break;
				}
			}

			if (foundDataType) {
				getline(inputFile, checkString);
				createNewVariableVariable(checkString, i);
			}
			else {
				cout << "Error: Invalid Data type: " << checkString << " Exiting Program." << endl;
				inputFile.close();
				return false;
			}
		}
		else {
			if (!checkString.compare("if")) {
				ifNodeIndex = ifCheckStringIsIf(&inputFile, checkString);
				if (ifNodeIndex == -1) {
					cout << "Error in Netlist Behavior File, please adjust." << endl;
					inputFile.close();
					return false;
				}
				flagIf = true;
			}
			else {
				if (!checkString.compare("else")) {
					elseFlag = true;
					inputFile >> checkString;
					inputFile >> checkString;
				}
				/* To always have a fresh set of eyes... */
				componentOutputIndex = -1;
				componentInputIndex = -1;
				componentWireIndex = -1;
				/* Check where the inputs/outputs/wires are for this datapath component. */
				if (!checkVariable(checkString, &componentOutputIndex, &componentInputIndex, &componentWireIndex)) {
					cout << "Variable '" << checkString << "' not found, please correct Netlist Behavior File." << endl;
					inputFile.close();
					return false;
				}
				else {
					/* Create some datapath components. */
					getline(inputFile, checkString);
					if (componentOutputIndex != -1) {
						if (!determineOperation(checkString, _outputs.at(componentOutputIndex))) {
							return false;
						}
					}
					else if (componentWireIndex != -1) {
						if (!determineOperation(checkString, _variables.at(componentWireIndex))) {
							return false;
						}
					}
					if (flagIf && elseFlag) {
						_nodes[ifNodeIndex]->addNextElseNode(_nodes.at(_nodes.size() - 1));
					}
				}
			}

		}
		checkString = "";
	}
	for (i = 0; i < (int)_nodes.size(); ++i) {
		//_datapathComponents.at(i).determineDataWidth();
		_nodes.at(i)->assignDelay();
		//_datapathComponents.at(i).checkIfSigned();
		//set going to for inputs
		for (j = 0; j < (int)_nodes.at(i)->getInputs().size(); ++j) {
			_nodes.at(i)->getInputs().at(j)->addToGoingTo(_nodes.at(i));
		}
		//sets coming from for outputs
		for (j = 0; j < (int)_nodes.at(i)->getOutputs().size(); ++j) {
			_nodes.at(i)->getOutputs().at(j)->addToComingFrom(_nodes.at(i));
		}
	}

	createUnscheduledGraph();
	// cout << &_nodes[3] << endl;

	inputFile.close();
	return true;

}

void HLSM::createUnscheduledGraph()
{
	int i = 0;
	int j = 0;
	int k = 0;

	for (i = 0; i < (int)_nodes.size(); ++i) {
		//if current node is not a mux
		if (_nodes.at(i)->getOperation().compare("?")) {
			for (j = 0; j < (int)_nodes.at(i)->getOutputs().size(); ++j) {
				for (k = 0; k < (int)_nodes.at(i)->getOutputs().at(j)->getGoingTo().size(); ++k) {
					//want to see to make sure next node is not a mux, unless current node is a equal to, greater than, or less than node
					if ((_nodes.at(i)->getOutputs().at(j)->getGoingTo().at(k)->getOperation().compare("?"))
						|| (!_nodes.at(i)->getOperation().compare("==")
						|| !_nodes.at(i)->getOperation().compare(">")
						|| !_nodes.at(i)->getOperation().compare("<"))) {
						_nodes.at(i)->addNextNode(_nodes.at(i)->getOutputs().at(j)->getGoingTo().at(k));
					}
				}
			}
			for (j = 0; j < (int)_nodes.at(i)->getInputs().size(); ++j) {
				if (_nodes.at(i)->getInputs().at(j)->getComingFrom().size() != 0) {
					for (k = 0; k < (int)_nodes.at(i)->getInputs().at(j)->getComingFrom().size(); ++k) {
						_nodes.at(i)->addPreviousNode(_nodes.at(i)->getInputs().at(j)->getComingFrom().at(k));
					}
				}
			}
		}
		else {
			if (!_nodes.at(i)->getConditional()) {
				_nodes.at(i)->setNextNodes(_nodes.at(i)->getOutputs().at(0)->getGoingTo());
				_nodes.at(i)->setPreviousNodes(_nodes.at(i)->getInputs().at(0)->getComingFrom());
			}
		}
	}
}

/*FDS( G(V,E), lambda){
	repeat{
		Compute the time frames; (DONE)
		Compute the operations and type probabilities; (DONE)
		Compute the self-forces (DONE?), predcessor/successor forces (TODO) and total forces (DONE);
		Schedule the operation with least force and update its time-frame; (DONE)
	} until (all operations scheduled);
	return (t);
*/

bool HLSM::scheduleGraph(int latency)
{
	int scheduledNodes = 0;
	int i = 0;

	/* Staging. */
	for (i = 0; i < latency; ++i) {
		_forceDirectedSchedule.push_back(vector<Node*>());
	}

	if (!checkLatency(latency)) {
		return false;
	}

	/* Does ASAP/ALAP Scheduling both are req'd for FDS. */
	asapSchedule(latency);
	if ((int)_asapSchedule.size() > latency) {
		return false;
	}
	if (!alapSchedule(latency)) {
		return false;
	}

	/* Schedules all nodes. */
	while (scheduledNodes < (int)_nodes.size()) {
		calculateOperationProbability(latency);
		calculateTypeDistributionProbability(latency);
		calculateNodeSelfForces();
		calculateNodePredecessorSuccessorForces();
		calculateNodeTotalForces();
		selectNodeToSchedule();
		clearAlgothrimVectors();

		++scheduledNodes;
	}

	/* Creates the states to print. */
	createStates();

	return true;
}

void HLSM::asapSchedule(int latency)
{
	int i = 0;
	int j = 0;
	int k = 0;
	int m = 0;
	int scheduledNodes = 0;
	bool unscheduledPreviousNode = false;

	//create empty vectors in vector of vectors
	//for (i = 0; i < latency; ++i) {
	//	_asapSchedule.push_back(vector<Node*>());
	//}
	//go through each time cycle
	while(scheduledNodes < (int)_nodes.size()){//for (i = 0; i < latency; ++i) {
		//go through each unscheduled node
		_asapSchedule.push_back(vector<Node*>());
		
		for (j = 0; j < (int)_nodes.size(); ++j) {
			unscheduledPreviousNode = false;
			if (i == 0) {
				//if there are no previous nodes, current node goes into time 1 for asap
				if (_nodes.at(j)->getPreviousNodes().size() == 0) {
					_asapSchedule[i].push_back(_nodes.at(j));
					_nodes.at(j)->setAsapTime(i);
					++scheduledNodes;
					
					//Need to account for the delay if there is one
					for (k = 0; k < (int)_nodes.at(j)->getNextNodes().size(); ++k) {
						if (i + _nodes.at(j)->getDelay() > _nodes.at(j)->getNextNodes().at(k)->getCycleAllowed()) {
							_nodes.at(j)->getNextNodes().at(k)->setCycleAllowed(i + _nodes.at(j)->getDelay());
						}
					}
				}
			}
			else {
				//schedule node if allowed cycle is equal to i
				for (m = 0; m < (int)_nodes.at(j)->getPreviousNodes().size(); ++m) {
					if (_nodes.at(j)->getPreviousNodes().at(m)->getAsapTime() == -1) {
						unscheduledPreviousNode = true;
					}
				}
				if (_nodes.at(j)->getCycleAllowed() == i && !unscheduledPreviousNode) {
					_asapSchedule[i].push_back(_nodes.at(j));
					_nodes.at(j)->setAsapTime(i);
					++scheduledNodes;
					//update nodes allowed cycle time
					for (k = 0; k < (int)_nodes.at(j)->getNextNodes().size(); ++k) {
						if (i + _nodes.at(j)->getDelay() > _nodes.at(j)->getNextNodes().at(k)->getCycleAllowed()) {
							_nodes.at(j)->getNextNodes().at(k)->setCycleAllowed(i + _nodes.at(j)->getDelay());
						}
					}
					if (_nodes.at(j)->getConditional()) {
						for (k = 0; k < (int)_nodes.at(j)->getNextIfNodes().size(); ++k) {
							if (i + _nodes.at(j)->getDelay() > _nodes.at(j)->getNextIfNodes().at(k)->getCycleAllowed()) {
								_nodes.at(j)->getNextIfNodes().at(k)->setCycleAllowed(i + _nodes.at(j)->getDelay());
							}
						}
						for (k = 0; k < (int)_nodes.at(j)->getNextElseNodes().size(); ++k) {
							if (i + _nodes.at(j)->getDelay() > _nodes.at(j)->getNextElseNodes().at(k)->getCycleAllowed()) {
								_nodes.at(j)->getNextElseNodes().at(k)->setCycleAllowed(i + _nodes.at(j)->getDelay());
							}
						}
					}
				}
			}
		}
		++i;
	}
}

//NOTE: this assigns the correct time cycle, however if you look at the alap shcedule variable, there is 1 too many nodes. will fix later, but have all info I need
bool HLSM::alapSchedule(int latency)
{
	int i = 0;
	int j = 0;
	int k = 0;

	for (i = 0; i < (int)_nodes.size(); ++i) {
		_nodes.at(i)->setCycleAllowed(latency - 1);
	}

	//adding empty vectors to the times
	for (i = 0; i < latency; ++i) {
		_alapShcedule.push_back(vector<Node*>());
	}

	for (i = latency - 1; i >= 0; --i) {
		for (j = 0; j < (int)_nodes.size(); ++j) {
			if (i == latency - 1) {
				if (_nodes.at(j)->getNextNodes().size() == 0 && _nodes.at(j)->getNextIfNodes().size() == 0 ){//&& _nodes.at(j)->getNextIfNodes().size() == 0 && _nodes.at(j)->getNextElseNodes().size() == 0) {
					if (_nodes.at(j)->getDelay() > 1) {
						_alapShcedule[i - _nodes.at(j)->getDelay()+1].push_back(_nodes.at(j));
						_nodes.at(j)->setAlapTime(i - _nodes.at(j)->getDelay() +1);

						for (k = 0; k < (int)_nodes.at(j)->getPreviousNodes().size(); ++k) {
							if (i - _nodes.at(j)->getDelay() + 1 < _nodes.at(j)->getPreviousNodes().at(k)->getCycleAllowed()) {
								_nodes.at(j)->getPreviousNodes().at(k)->setCycleAllowed(i - _nodes.at(j)->getDelay() + 1);
							}
						}
					}
					else {
						_alapShcedule[i].push_back(_nodes.at(j));
						_nodes.at(j)->setAlapTime(i);

						for (k = 0; k < (int)_nodes.at(j)->getPreviousNodes().size(); ++k) {
							if (i - _nodes.at(j)->getDelay() < _nodes.at(j)->getPreviousNodes().at(k)->getCycleAllowed()) {
								_nodes.at(j)->getPreviousNodes().at(k)->setCycleAllowed(i - _nodes.at(j)->getDelay());
							}
						}
					}
				}
			}
			else {
				if (_nodes.at(j)->getCycleAllowed() == i) {
					if (_nodes.at(j)->getDelay() > 1) {
						_alapShcedule[i - _nodes.at(j)->getDelay()].push_back(_nodes.at(j));
						_nodes.at(j)->setAlapTime(i - _nodes.at(j)->getDelay());

						for (k = 0; k < (int)_nodes.at(j)->getPreviousNodes().size(); ++k) {
							if (i - _nodes.at(j)->getDelay() - 1 < _nodes.at(j)->getPreviousNodes().at(k)->getCycleAllowed()) {
								_nodes.at(j)->getPreviousNodes().at(k)->setCycleAllowed(i - _nodes.at(j)->getDelay());
							}
						}
					}
					else {
						_alapShcedule[i].push_back(_nodes.at(j));
						_nodes.at(j)->setAlapTime(i);

						for (k = 0; k < (int)_nodes.at(j)->getPreviousNodes().size(); ++k) {
							if (i - _nodes.at(j)->getDelay() < _nodes.at(j)->getPreviousNodes().at(k)->getCycleAllowed()) {
								_nodes.at(j)->getPreviousNodes().at(k)->setCycleAllowed(i - _nodes.at(j)->getDelay());
							}
						}
					//update nodes allowed cycle time
					}

				}
			}
		}
	}
	for (i = 0; i < (int)_nodes.size(); ++i) {
		if (_nodes.at(i)->getCycleAllowed() < 0) {
			return false;
		}
	}
	return true;
}

void HLSM::calculateOperationProbability(int latency)
{
	int i = 0;

	for (i = 0; i < (int)_nodes.size(); ++i) {
		_nodes.at(i)->assignOperationProbability(latency);
	}
}

void HLSM::calculateTypeDistributionProbability(int latency)
{
	int i = 0;  //time cycle counter
	int j = 0;  //node counter

	//loop for time cycles
	for (i = 0; i < latency; ++i) {
		//inalize distributions to zero
		_addSubDistribution.push_back(0.0);
		_multDistribution.push_back(0.0);
		_modDivDistribution.push_back(0.0);
		_logicDistribution.push_back(0.0);
		//loop for the nodes
		for (j = 0; j < (int)_nodes.size(); ++j) {
			if (_nodes.at(j)->getOperation() == "*") {
				_multDistribution[i] = _multDistribution[i] + _nodes.at(j)->getOperationProbability().at(i);
			}
			else if (_nodes.at(j)->getOperation() == "+" || _nodes.at(j)->getOperation() == "-") {
				_addSubDistribution[i] = _addSubDistribution[i] + _nodes.at(j)->getOperationProbability().at(i);
			}
			else if (_nodes.at(j)->getOperation() == "/" || _nodes.at(j)->getOperation() == "%") {
				_modDivDistribution[i] = _modDivDistribution[i] + _nodes.at(j)->getOperationProbability().at(i);
			}
			else {
				_logicDistribution[i] = _logicDistribution[i] + _nodes.at(j)->getOperationProbability().at(i);
			}
		}
	}
	
}

//I think this is giving the right results, need to go through and calculate everything
void HLSM::calculateNodeSelfForces()
{
	int i = 0;

	for (i = 0; i < (int)_nodes.size(); ++i) {
		if (_nodes.at(i)->getOperation() == "*") {
			_nodes.at(i)->calculateSelfForce(_multDistribution);
		}
		else if (_nodes.at(i)->getOperation() == "+" || _nodes.at(i)->getOperation() == "-") {
			_nodes.at(i)->calculateSelfForce(_addSubDistribution);
		}
		else if (_nodes.at(i)->getOperation() == "/" || _nodes.at(i)->getOperation() == "%") {
			_nodes.at(i)->calculateSelfForce(_modDivDistribution);
		}
		else {
			_nodes.at(i)->calculateSelfForce(_logicDistribution);
		}
	}
    
}

void HLSM::calculateNodePredecessorSuccessorForces()
{
	int i = 0;
	//int j = 0;
	std::vector<Node*> prevNodes;
	std::vector<Node*> succNodes;

	/* Calculate the successor and predecessor forces for all nodes. */
	for (i = 0; i < (int)_nodes.size(); ++i) {
		_nodes.at(i)->calculateSuccessorForce(_multDistribution, _addSubDistribution, _modDivDistribution, _logicDistribution);
		_nodes.at(i)->calculatePredecessorForce(_multDistribution, _addSubDistribution, _modDivDistribution, _logicDistribution);
	}

}

void HLSM::calculateNodeTotalForces() 
{
	int i = 0;

	for (i = 0; i < (int)_nodes.size(); ++i) {
		_nodes.at(i)->calculateTotalForce();
	}
}

void HLSM::selectNodeToSchedule()
{
	int i = 0;
	int j = 0;
	double mostNegativeForce = 9999.0;
	int timeToBeScheduled = -1;
	Node* nodeToBeScheduled = NULL;

	for (i = 0; i < (int)_nodes.size(); ++i) {
		for (j = 0; j < (int)_nodes[i]->getTotalForces().size(); ++j) {
			if (_nodes[i]->getTotalForces().at(j) < mostNegativeForce && !_nodes[i]->getScheduled()) {
				mostNegativeForce = _nodes[i]->getTotalForces().at(j);
				timeToBeScheduled = j;
				nodeToBeScheduled = _nodes[i];
			}
		}
	}
	nodeToBeScheduled->setScheduled(true);
	_forceDirectedSchedule[timeToBeScheduled].push_back(nodeToBeScheduled);
	nodeToBeScheduled->setAlapTime(timeToBeScheduled);
	nodeToBeScheduled->setAsapTime(timeToBeScheduled);
	nodeToBeScheduled->setFDSTime(timeToBeScheduled);
}

bool HLSM::writeToFile(char* fileName)
{
	/* Set up variables. */
	ofstream outputFile;
	time_t timeNow = time(0);
	string tempFileName = "";
	string moduleName = "";
	int i = 0;
	//int j = 0;

	/* Staging file name. */
	tempFileName = fileName;
	for (i = tempFileName.size() - 1; i > 0; i--) {
		if (tempFileName.at(i) == '\\' || tempFileName.at(i) == '/') {
			break;
		}
	}
	if (i != 0) {
		for (i = i + 1; i < (int)tempFileName.size() - 1; i++) {
			if (tempFileName.at(i) == '.') {
				break;
			}
			moduleName += tempFileName.at(i);
		}
	}
	else {
		/* If there was no directory provided... */
		moduleName = tempFileName;
		moduleName.pop_back();
		moduleName.pop_back();
	}

	/* Open the output file. */
	outputFile.open(fileName);
	if (!outputFile.is_open()) {
		return false;
	}

	/* Check for empty netlist behavior files. */
	if (_inputs.empty()) {
		outputFile.close();
		return false;
	}
	if (_outputs.empty()) {
		outputFile.close();
		return false;
	}

	/* Print Setup. */
	outputFile << "`timescale 1ns / 1ps" << endl;
	outputFile << "//////////////////////////////////////////////////////////////////////////////////" << endl;
	outputFile << "//" << endl;
	outputFile << "//Students: Brett Bushnell (Undergrad), Matt Dzurick (Grad)" << endl;
	outputFile << "//Date Created: " << asctime(localtime(&timeNow));
	outputFile << "//Assignment: " << "3" << endl;
	outputFile << "//File: " << moduleName << ".v" << endl;
	outputFile << "//Description: An HLSM module which represents the C-like behavioral description " << endl;
	outputFile << "//             into a scheduled high-level statement machine implemented in Verilog." << endl;
	outputFile << "//" << endl;
	outputFile << "//////////////////////////////////////////////////////////////////////////////////" << endl;
	outputFile << endl;

	/* Start of Module. */
	// outputFile << "module " << moduleName << "(clk, rst, ";
	outputFile << "module " << "HLSM" << "(Clk, Rst, Start, Done, ";
	for (i = 0; i < (int)_inputs.size(); i++) {
		outputFile << (*_inputs.at(i)).getName();
		outputFile << ", ";
	}
	for (i = 0; i < (int)_outputs.size(); i++) {
		outputFile << (*_outputs.at(i)).getName();
		if (i != (int)_outputs.size() - 1) {
			outputFile << ", ";
		}
	}
	outputFile << ");" << endl;

	/* Print Clk, Rst. */
	// outputFile << "\t" << "input clk, rst;" << endl;
	outputFile << "\t" << "input Clk, Rst, Start;" << endl;
	outputFile << "\t" << "output reg Done;" << endl << endl;

	/* Print Inputs, Outputs, Wires. */
	writeVarsToFile(&outputFile);

	/* Print the state */
	outputFile << "\t" << "reg[";
	outputFile << ceil(log2(_states.size() + 2));
	outputFile << ":0] state;" << endl;

	/* Print the parameters */
	outputFile << "\t" << "parameter ";
	outputFile << "sWait = 0,";

	/* Print out all parameters (nodes, really) */
	for (i = 0; i < (int)_states.size(); i++) {
		outputFile << " s" << i + 2 << " = " << i + 1 << ",";
	}
	outputFile << " sFinal = " << i + 1 << ";" << endl << endl;

	/* Create the case statements. */
	outputFile << "\t" << "always@(";
	outputFile << "posedge Clk)";
	outputFile << " begin" << endl;
	
	/* Reset condition. */
	outputFile << "\t\t" << "if(Rst == 1) begin" << endl;
	outputFile << "\t\t\t" << "state <= sWait;" << endl;
	for (i = 0; i < (int)_variables.size(); i++) {
		outputFile << "\t\t\t";
		outputFile << _variables.at(i)->getName();
		outputFile << " <= 0;" << endl;
	}
	for (i = 0; i < (int)_outputs.size(); i++) {
		outputFile << "\t\t\t" << _outputs.at(i)->getName() << " <= 0;" << endl;
	}
	outputFile << "\t\t\t" << "Done <= 0;" << endl;
	outputFile << "\t\t" << "end" << endl;

	/* HLSM Now! Go! */
	outputFile << "\t\t" << "else begin" << endl;
	outputFile << "\t\t\t" << "case(state)" << endl;
	
	/* sWait State. */
	outputFile << "\t\t\t\t" << "sWait: begin" << endl;
	outputFile << "\t\t\t\t\t" << "Done <= 0;" << endl;
	outputFile << "\t\t\t\t\t" << "if (Start == 1)" << endl;
	outputFile << "\t\t\t\t\t\t" << "state <= s2;" << endl;
	outputFile << "\t\t\t\t\t" << "else" << endl;
	outputFile << "\t\t\t\t\t\t" << "state <= sWait;" << endl;
	outputFile << "\t\t\t\t" << "end" << endl;

	/* The actual states. */
	writeStates(&outputFile);

	/* Final State. */
	outputFile << "\t\t\t\t" << "sFinal: begin" << endl;
	outputFile << "\t\t\t\t\t" << "Done <= 1;" << endl;
	outputFile << "\t\t\t\t\t" << "state <= sWait;" << endl;
	outputFile << "\t\t\t\t" << "end" << endl;
	outputFile << "\t\t\t" << "endcase" << endl;
	outputFile << "\t\t" << "end" << endl;
	outputFile << "\t" << "end" << endl;

	/* End Module. */
	outputFile << endl << "endmodule" << endl;

	/* Close the output file. */
	outputFile.close();

	return true;

}

bool HLSM::writeStates(std::ofstream *outputFile) {

	int i = 0;
	int j = 0;
	// int k = 0;

	/* Make sure it is still open. */
	if (!(*outputFile).is_open()) {
		return false;
	}

	/* Bookkeeping. */
	// Funct	If		Print	vTest
	// ADD		Y		Y		N
	// SUB		Y		Y		N
	// MULT		Y		Y		N
	// GREATER	Y		Y		N
	// LESSER	Y		N		N
	// EQUAL	Y		N		N
	// MUX		Y		Y		N
	// SHR		Y		N		N
	// SHL		Y		N		N
	// DIV		Y		N		N
	// MODULO	Y		N		N
	// INC		Y		N		N
	// DEC		Y		N		N
	// IF		N		N		N
	// FOR		N		N		N
	// const std::string validSymbols[13] = { "=","+" ,"-", "*", ">", "<","==", "?", ":", ">>", "<<", "/", "%" };

	/* Write the operations. */
	// k = 3;
	for (i = 0; i < (int)_states.size(); i++) {
		*outputFile << "\t\t\t\t" << "s";
		*outputFile << (i + 2);
		*outputFile << ": begin" << endl;
		for (j = 0; j < (int)_states.at(i).getAssignedNodes().size(); j++) {
			 // if (_nodes.at(j)->getFDSTime() == i) {
				/* ADDITION */ /* SUBTRACTION */
				/* INCREMENT */ /* DECREMENT */
				if (_states.at(i).getAssignedNodes().at(j)->getOperation() == "+" ||
					_states.at(i).getAssignedNodes().at(j)->getOperation() == "-") {
					*outputFile << "\t\t\t\t\t";
					*outputFile << _states.at(i).getAssignedNodes().at(j)->getOutputs().at(0)->getName();
					// *outputFile << _nodes.at(j)->getOutputs().at(0)->getName();
					*outputFile << " <= ";
					/* Inc/Dec */
					if (_states.at(i).getAssignedNodes().at(j)->getInputs().at(1)->getName() == "1") {
					//if (_nodes.at(j)->getInputs().at(1)->getName() == "1") {
						*outputFile << _states.at(i).getAssignedNodes().at(j)->getInputs().at(0)->getName();
						// *outputFile << _nodes.at(j)->getInputs().at(0)->getName();
						*outputFile << " " << _states.at(i).getAssignedNodes().at(j)->getOperation() << _states.at(i).getAssignedNodes().at(j)->getOperation();
						//*outputFile << " " << _nodes.at(j)->getOperation() << _nodes.at(j)->getOperation();
						*outputFile << ";" << endl;
					}
					/* Not */
					else {
						*outputFile << _states.at(i).getAssignedNodes().at(j)->getInputs().at(0)->getName();
						*outputFile << " " << _states.at(i).getAssignedNodes().at(j)->getOperation() << " ";
						*outputFile << _states.at(i).getAssignedNodes().at(j)->getInputs().at(1)->getName();
						// *outputFile << _nodes.at(j)->getInputs().at(0)->getName();
						// *outputFile << " " << _nodes.at(j)->getOperation() << " ";
						// *outputFile << _nodes.at(j)->getInputs().at(1)->getName();
					}
					*outputFile << ";" << endl;
				}
				/* MULTIPLICATION */
				/* DIVISION */ /* MODULO */ /* GREATER THAN */
				/* LESSER THAN */ /* EQUAL TO */ /* SHIFT LEFT */
				/* SHIFT RIGHT */
				else if (_states.at(i).getAssignedNodes().at(j)->getOperation() == "*" ||
					_states.at(i).getAssignedNodes().at(j)->getOperation() == "/" ||
					_states.at(i).getAssignedNodes().at(j)->getOperation() == "%" ||
					_states.at(i).getAssignedNodes().at(j)->getOperation() == ">" ||
					_states.at(i).getAssignedNodes().at(j)->getOperation() == "<" ||
					_states.at(i).getAssignedNodes().at(j)->getOperation() == "==" ||
					_states.at(i).getAssignedNodes().at(j)->getOperation() == ">>" ||
					_states.at(i).getAssignedNodes().at(j)->getOperation() == "<<") {
					*outputFile << "\t\t\t\t\t";
					*outputFile << _states.at(i).getAssignedNodes().at(j)->getOutputs().at(0)->getName();
					*outputFile << " <= ";
					*outputFile << _states.at(i).getAssignedNodes().at(j)->getInputs().at(0)->getName();
					*outputFile << " " << _states.at(i).getAssignedNodes().at(j)->getOperation() << " ";
					*outputFile << _states.at(i).getAssignedNodes().at(j)->getInputs().at(1)->getName();
					*outputFile << ";" << endl;
				}
				/* MULTIPLEXOR */
				else if (_states.at(i).getAssignedNodes().at(j)->getOperation() == "?") {
					*outputFile << "\t\t\t\t\t";
					*outputFile << _states.at(i).getAssignedNodes().at(j)->getOutputs().at(0)->getName();
					*outputFile << " <= ";
					*outputFile << _states.at(i).getAssignedNodes().at(j)->getInputs().at(0)->getName();
					*outputFile << " ? ";
					*outputFile << _states.at(i).getAssignedNodes().at(j)->getInputs().at(1)->getName();
					*outputFile << " : ";
					*outputFile << _states.at(i).getAssignedNodes().at(j)->getInputs().at(2)->getName();
					*outputFile << ";" << endl;
				}
				else if (_states.at(i).getAssignedNodes().at(j)->getOperation() == "if") {
					// TODO TODO TODO!
				}
				/* This is a problem... Awkward. */
				else {
					// There is no defined structure.
					return false;
				}
			// }
		}
		/* Print for the next state. */
		*outputFile << "\t\t\t\t\t" << "state <= ";
		if (i < (int)_states.size() - 1) {
			*outputFile << "s" << (i + 3) << ";" << endl;
		}
		else {

			*outputFile << "sFinal;" << endl;
		}
		*outputFile << "\t\t\t\t" << "end" << endl;
	}
	return true;

}

void HLSM::createNewInputVariable(std::string checkString, int dataWidthIndex)
{
	string checkChar = "";
	string variableName = "";
	int i = 0;

	for (i = checkString.size() - 1; i >= 0; --i) {
		if (checkString.at(i) == ' ' || checkString.at(i) == '\r') {
			checkString.erase(checkString.end() - 1);
		}
		else {
			break;
		}
	}

	std::istringstream iss(checkString);
	//checkString += '\n';

	while (!iss.eof()) {
		iss >> variableName;
		if (variableName.at(variableName.size() - 1) == ',') {
			variableName.erase(variableName.size() - 1);
		}

		if (checkIfComment(variableName)) {
			break;
		}

		switch (dataWidthIndex) {
		case 0: createNewInput(variableName, 1, DATAWIDTH_1);
			break;
		case 1: createNewInput(variableName, 1, DATAWIDTH_2);
			break;
		case 2: createNewInput(variableName, 1, DATAWIDTH_8);
			break;
		case 3: createNewInput(variableName, 1, DATAWIDTH_16);
			break;
		case 4: createNewInput(variableName, 1, DATAWIDTH_32);
			break;
		case 5: createNewInput(variableName, 1, DATAWIDTH_64);
			break;
		case 6: createNewInput(variableName, 0, DATAWIDTH_1);
			break;
		case 7: createNewInput(variableName, 0, DATAWIDTH_2);
			break;
		case 8: createNewInput(variableName, 0, DATAWIDTH_8);
			break;
		case 9: createNewInput(variableName, 0, DATAWIDTH_16);
			break;
		case 10: createNewInput(variableName, 0, DATAWIDTH_32);
			break;
		case 11: createNewInput(variableName, 0, DATAWIDTH_64);
			break;
		}

	}
}

void HLSM::createNewOutputVariable(std::string checkString, int dataWidthIndex)
{
	string checkChar = "";
	string variableName = "";
	int i = 0;

	for (i = checkString.size() - 1; i >= 0; --i) {
		if (checkString.at(i) == ' ' || checkString.at(i) == '\r') {
			checkString.erase(checkString.end() - 1);
		}
		else {
			break;
		}
	}


	std::istringstream iss(checkString);
	//checkString += '\n';

	while (!iss.eof()) {
		iss >> variableName;

		if (variableName.at(variableName.size() - 1) == ',') {
			variableName.erase(variableName.size() - 1);
		}

		if (checkIfComment(variableName)) {
			break;
		}

		switch (dataWidthIndex) {
		case 0: createNewOutput(variableName, 1, DATAWIDTH_1);
			break;
		case 1: createNewOutput(variableName, 1, DATAWIDTH_2);
			break;
		case 2: createNewOutput(variableName, 1, DATAWIDTH_8);
			break;
		case 3: createNewOutput(variableName, 1, DATAWIDTH_16);
			break;
		case 4: createNewOutput(variableName, 1, DATAWIDTH_32);
			break;
		case 5: createNewOutput(variableName, 1, DATAWIDTH_64);
			break;
		case 6: createNewOutput(variableName, 0, DATAWIDTH_1);
			break;
		case 7: createNewOutput(variableName, 0, DATAWIDTH_2);
			break;
		case 8: createNewOutput(variableName, 0, DATAWIDTH_8);
			break;
		case 9: createNewOutput(variableName, 0, DATAWIDTH_16);
			break;
		case 10: createNewOutput(variableName, 0, DATAWIDTH_32);
			break;
		case 11: createNewOutput(variableName, 0, DATAWIDTH_64);
			break;
		}
	}
}

void HLSM::createNewVariableVariable(std::string checkString, int dataWidthIndex)
{
	string checkChar = "";
	string variableName = "";
	int i = 0;

	for (i = checkString.size() - 1; i >= 0; --i) {
		if (checkString.at(i) == ' ' || checkString.at(i) == '\r') {
			checkString.erase(checkString.end() - 1);
		}
		else {
			break;
		}
	}

	std::istringstream iss(checkString);

	//checkString += '\n';
	while (!iss.eof()) {
		iss >> variableName;

		if (variableName.at(variableName.size() - 1) == ',') {
			variableName.erase(variableName.size() - 1);
		}

		if (checkIfComment(variableName)) {
			break;
		}

		switch (dataWidthIndex) {
		case 0: createNewVariable(variableName, 1, DATAWIDTH_1);
			break;
		case 1: createNewVariable(variableName, 1, DATAWIDTH_2);
			break;
		case 2: createNewVariable(variableName, 1, DATAWIDTH_8);
			break;
		case 3: createNewVariable(variableName, 1, DATAWIDTH_16);
			break;
		case 4: createNewVariable(variableName, 1, DATAWIDTH_32);
			break;
		case 5: createNewVariable(variableName, 1, DATAWIDTH_64);
			break;
		case 6: createNewVariable(variableName, 0, DATAWIDTH_1);
			break;
		case 7: createNewVariable(variableName, 0, DATAWIDTH_2);
			break;
		case 8: createNewVariable(variableName, 0, DATAWIDTH_8);
			break;
		case 9: createNewVariable(variableName, 0, DATAWIDTH_16);
			break;
		case 10: createNewVariable(variableName, 0, DATAWIDTH_32);
			break;
		case 11: createNewVariable(variableName, 0, DATAWIDTH_64);
			break;
		}
	}
}

void HLSM::createNewInput(std::string name, bool sign, int dataWidth)
{
	Input* newInput = new Input(name, sign, dataWidth);

	_inputs.push_back(newInput);
}

void HLSM::createNewOutput(std::string name, bool sign, int dataWidth)
{
	Output* newOutput = new Output(name, sign, dataWidth);

	_outputs.push_back(newOutput);
}

void HLSM::createNewVariable(std::string name, bool sign, int dataWidth)
{
	Variable* newVariable = new Variable(name, sign, dataWidth);

	_variables.push_back(newVariable);
}

bool HLSM::checkVariable(std::string checkName, int* outputIndex, int* inputIndex, int* wireIndex)
{
	bool variableFound = false;
	int i = 0;

	/*check if in inputs*/
	for (i = 0; i < (int)_inputs.size(); ++i) {
		if (!(*_inputs.at(i)).getName().compare(checkName)) {
			variableFound = true;
			*inputIndex = i;
			break;
		}
	}

	/*check if in wires*/
	for (i = 0; i < (int)_variables.size(); ++i) {
		if (!(*_variables.at(i)).getName().compare(checkName)) {
			variableFound = true;
			*wireIndex = i;
			break;
		}
	}

	/*check if in outputs*/
	for (i = 0; i < (int)_outputs.size(); ++i) {
		if (!(*_outputs.at(i)).getName().compare(checkName)) {
			variableFound = true;
			*outputIndex = i;
			break;
		}
	}
	return variableFound;
}

bool HLSM::determineOperation(std::string line, DataType* output)
{
	int i = 0;
	int equalCount = 0;
	int inputIndex = -1;
	int wireIndex = -1;
	int outputIndex = -1;
	bool result = true;
	// bool isDecInc = false;
	std::string checkString = "";
	std::string componentType = "";
	std::string tempVariableName = "";
	std::vector<DataType*> componentInputs;
	std::vector<DataType*> componentOutputs;

	for (i = line.size() - 1; i >= 0; --i) {
		if (line.at(i) == ' ' || line.at(i) == '\r') {
			line.erase(line.end() - 1);
		}
		else {
			break;
		}
	}

	std::istringstream iss(line);

	//line += '\n';
	componentOutputs.push_back(output);


	while (!iss.eof()) {
		iss >> checkString;

		if (!checkString.compare("=")) {
			++equalCount;
		}

		if (checkIfComment(checkString)) {
			break;
		}

		/*else if (!checkString.compare("1")) {
			if (!componentType.compare("ADD")) {
				componentType = "INC";
				isDecInc = true;
			}
			if (!componentType.compare("SUB")) {
				componentType = "DEC";
				isDecInc = true;
			}
		}*/

		else {
			if (!checkValidSymbol(checkString, &componentType)) {
				if (checkVariable(checkString, &outputIndex, &inputIndex, &wireIndex)) {
					if (inputIndex != -1) {
						componentInputs.push_back(_inputs.at(inputIndex));

					}
					else if (wireIndex != -1) {
						componentInputs.push_back(_variables.at(wireIndex));
					}
				}
				else {
					cout << checkString << " is not a valid variable or symbol" << endl;
					result = false;
					break;
				}
			}

		}
	}
	if (result) {
		createNewNode(componentType, _nodes.size() + 1, componentInputs, componentOutputs);
		componentType = "";
	}
	return result;

}

void HLSM::createNewNode(std::string operation, int num, std::vector<DataType*> _Cinputs, std::vector<DataType*> _Coutputs)
{
	Node* newNode = new Node(operation, num, _Cinputs, _Coutputs);

	_nodes.push_back(newNode);
}

bool HLSM::checkValidSymbol(std::string checkSymbol, std::string* operation)
{
	const std::string validSymbols[13] = { "=","+" ,"-", "*", ">", "<","==", "?", ":", ">>", "<<", "/", "%" };
	int i = 0;
	bool foundValidSymbol = false;

	for (i = 0; i < 13; i++) {
		if (!checkSymbol.compare(validSymbols[i])) {
			foundValidSymbol = true;
			if ((*operation).compare("?")) {
				*operation = validSymbols[i];
			}
			break;
		}
	}
	return foundValidSymbol;
}

bool HLSM::writeInputsToFile(ofstream *outputFile, int i, int j)
{

	///* Make sure it is still open. */
	//if (!(*outputFile).is_open()) {
	//	return false;
	//}

	///* Bit-width Matching. */
	//if ((*_nodes.at(i).getInputs().at(j)).getDataWidth() == _nodes.at(i).getDataWidth()) {
	//	(*outputFile) << (*_datapathComponents.at(i).getInputs().at(j)).getName() << ", ";
	//}
	//else if ((*_datapathComponents.at(i).getInputs().at(j)).getDataWidth() < _datapathComponents.at(i).getDataWidth()) { // Datapath width is larger than input.
	//	if ((*_datapathComponents.at(i).getInputs().at(j)).getSignUnsigned()) { // Signed
	//																			// Sign extensions for 2's complement. 
	//		(*outputFile) << "{{";
	//		(*outputFile) << _datapathComponents.at(i).getDataWidth() - (*_datapathComponents.at(i).getInputs().at(j)).getDataWidth();
	//		(*outputFile) << "{";
	//		(*outputFile) << (*_datapathComponents.at(i).getInputs().at(j)).getName();
	//		(*outputFile) << "[";
	//		(*outputFile) << (*_datapathComponents.at(i).getInputs().at(j)).getDataWidth() - 1;
	//		(*outputFile) << "]}},";
	//		(*outputFile) << (*_datapathComponents.at(i).getInputs().at(j)).getName();
	//		(*outputFile) << "}, ";

	//	}
	//	else { // Unsigned
	//		(*outputFile) << "{";
	//		(*outputFile) << _datapathComponents.at(i).getDataWidth() - (*_datapathComponents.at(i).getInputs().at(j)).getDataWidth();
	//		(*outputFile) << "'b0,";
	//		(*outputFile) << (*_datapathComponents.at(i).getInputs().at(j)).getName();
	//		(*outputFile) << "}, ";
	//	}
	//}
	//else { // Datapath width is smaller than input.

	//	   /* This way chops the last bits and uses those. */
	//	(*outputFile) << (*_datapathComponents.at(i).getInputs().at(j)).getName();
	//	(*outputFile) << "[";
	//	(*outputFile) << _datapathComponents.at(i).getDataWidth() - 1;
	//	(*outputFile) << ":0], ";
	//}

	return true;
}

bool HLSM::writeVarsToFile(std::ofstream *outputFile)
{
	int i = 0;
	int j = 0;
	int k = 0;
	bool ind1 = false;
	bool sind1 = false;
	bool ind2 = false;
	bool sind2 = false;
	bool ind8 = false;
	bool sind8 = false;
	bool ind16 = false;
	bool sind16 = false;
	bool ind32 = false;
	bool sind32 = false;
	bool ind64 = false;
	bool sind64 = false;

	/* Make sure it is still open. */
	if (!(*outputFile).is_open()) {
		return false;
	}

	/* Print Inputs. */
	ind1 = false;
	sind1 = false;
	ind2 = false;
	sind2 = false;
	ind8 = false;
	sind8 = false;
	ind16 = false;
	sind16 = false;
	ind32 = false;
	sind32 = false;
	ind64 = false;
	sind64 = false;

	/* Categorize if there are certain datawidths. */
	for (i = 0; i < (int)_inputs.size(); i++) {
		if (!(*_inputs.at(i)).getSignUnsigned()) {
			if ((*_inputs.at(i)).getDataWidth() == DATAWIDTH_1) {
				ind1 = true;
			}
			else if ((*_inputs.at(i)).getDataWidth() == DATAWIDTH_2) {
				ind2 = true;
			}
			else if ((*_inputs.at(i)).getDataWidth() == DATAWIDTH_8) {
				ind8 = true;
			}
			else if ((*_inputs.at(i)).getDataWidth() == DATAWIDTH_16) {
				ind16 = true;
			}
			else if ((*_inputs.at(i)).getDataWidth() == DATAWIDTH_32) {
				ind32 = true;
			}
			else if ((*_inputs.at(i)).getDataWidth() == DATAWIDTH_64) {
				ind64 = true;
			}
		}
		else {
			if ((*_inputs.at(i)).getDataWidth() == DATAWIDTH_1) {
				sind1 = true;
			}
			else if ((*_inputs.at(i)).getDataWidth() == DATAWIDTH_2) {
				sind2 = true;
			}
			else if ((*_inputs.at(i)).getDataWidth() == DATAWIDTH_8) {
				sind8 = true;
			}
			else if ((*_inputs.at(i)).getDataWidth() == DATAWIDTH_16) {
				sind16 = true;
			}
			else if ((*_inputs.at(i)).getDataWidth() == DATAWIDTH_32) {
				sind32 = true;
			}
			else if ((*_inputs.at(i)).getDataWidth() == DATAWIDTH_64) {
				sind64 = true;
			}
		}
	}

	/* Check for each datawidth. */
	if (ind1) {
		k = 0;
		*outputFile << "\t" << "input ";
		for (i = 0; i < (int)_inputs.size(); i++) {
			if (!(*_inputs.at(i)).getSignUnsigned()) {
				if ((*_inputs.at(i)).getDataWidth() == DATAWIDTH_1) {
					if (k) {
						*outputFile << ", ";
					}
					*outputFile << (*_inputs.at(i)).getName();
					k = 1;
				}
			}
		}
		*outputFile << ";" << endl;
	}
	if (sind1) {
		k = 0;
		*outputFile << "\t" << "input signed ";
		for (i = 0; i < (int)_inputs.size(); i++) {
			if ((*_inputs.at(i)).getSignUnsigned()) {
				if ((*_inputs.at(i)).getDataWidth() == DATAWIDTH_1) {
					if (k) {
						*outputFile << ", ";
					}
					*outputFile << (*_inputs.at(i)).getName();
					k = 1;
				}
			}
		}
		*outputFile << ";" << endl;
	}
	if (ind2) {
		k = 0;
		*outputFile << "\t" << "input ";
		for (i = 0; i < (int)_inputs.size(); i++) {
			if (!(*_inputs.at(i)).getSignUnsigned()) {
				if ((*_inputs.at(i)).getDataWidth() == DATAWIDTH_2) {
					if (k) {
						*outputFile << ", ";
					}
					else {
						*outputFile << "[1:0] ";
					}
					*outputFile << (*_inputs.at(i)).getName();
					k = 1;
				}
			}
		}
		*outputFile << ";" << endl;
	}
	if (sind2) {
		k = 0;
		*outputFile << "\t" << "input signed ";
		for (i = 0; i < (int)_inputs.size(); i++) {
			if ((*_inputs.at(i)).getSignUnsigned()) {
				if ((*_inputs.at(i)).getDataWidth() == DATAWIDTH_2) {
					if (k) {
						*outputFile << ", ";
					}
					else {
						*outputFile << "[1:0] ";
					}
					*outputFile << (*_inputs.at(i)).getName();
					k = 1;
				}
			}
		}
		*outputFile << ";" << endl;
	}
	if (ind8) {
		k = 0;
		*outputFile << "\t" << "input ";
		for (i = 0; i < (int)_inputs.size(); i++) {
			if (!(*_inputs.at(i)).getSignUnsigned()) {
				if ((*_inputs.at(i)).getDataWidth() == DATAWIDTH_8) {
					if (k) {
						*outputFile << ", ";
					}
					else {
						*outputFile << "[7:0] ";
					}
					*outputFile << (*_inputs.at(i)).getName();
					k = 1;
				}
			}
		}
		*outputFile << ";" << endl;
	}
	if (sind8) {
		k = 0;
		*outputFile << "\t" << "input signed ";
		for (i = 0; i < (int)_inputs.size(); i++) {
			if ((*_inputs.at(i)).getSignUnsigned()) {
				if ((*_inputs.at(i)).getDataWidth() == DATAWIDTH_8) {
					if (k) {
						*outputFile << ", ";
					}
					else {
						*outputFile << "[7:0] ";
					}
					*outputFile << (*_inputs.at(i)).getName();
					k = 1;
				}
			}
		}
		*outputFile << ";" << endl;
	}
	if (ind16) {
		k = 0;
		*outputFile << "\t" << "input ";
		for (i = 0; i < (int)_inputs.size(); i++) {
			if (!(*_inputs.at(i)).getSignUnsigned()) {
				if ((*_inputs.at(i)).getDataWidth() == DATAWIDTH_16) {
					if (k) {
						*outputFile << ", ";
					}
					else {
						*outputFile << "[15:0] ";
					}
					*outputFile << (*_inputs.at(i)).getName();
					k = 1;
				}
			}
		}
		*outputFile << ";" << endl;
	}
	if (sind16) {
		k = 0;
		*outputFile << "\t" << "input signed ";
		for (i = 0; i < (int)_inputs.size(); i++) {
			if ((*_inputs.at(i)).getSignUnsigned()) {
				if ((*_inputs.at(i)).getDataWidth() == DATAWIDTH_16) {
					if (k) {
						*outputFile << ", ";
					}
					else {
						*outputFile << "[15:0] ";
					}
					*outputFile << (*_inputs.at(i)).getName();
					k = 1;
				}
			}
		}
		*outputFile << ";" << endl;
	}
	if (ind32) {
		k = 0;
		*outputFile << "\t" << "input ";
		for (i = 0; i < (int)_inputs.size(); i++) {
			if (!(*_inputs.at(i)).getSignUnsigned()) {
				if ((*_inputs.at(i)).getDataWidth() == DATAWIDTH_32) {
					if (k) {
						*outputFile << ", ";
					}
					else {
						*outputFile << "[31:0] ";
					}
					*outputFile << (*_inputs.at(i)).getName();
					k = 1;
				}
			}
		}
		*outputFile << ";" << endl;
	}
	if (sind32) {
		k = 0;
		*outputFile << "\t" << "input signed ";
		for (i = 0; i < (int)_inputs.size(); i++) {
			if ((*_inputs.at(i)).getSignUnsigned()) {
				if ((*_inputs.at(i)).getDataWidth() == DATAWIDTH_32) {
					if (k) {
						*outputFile << ", ";
					}
					else {
						*outputFile << "[31:0] ";
					}
					*outputFile << (*_inputs.at(i)).getName();
					k = 1;
				}
			}
		}
		*outputFile << ";" << endl;
	}
	if (ind64) {
		k = 0;
		*outputFile << "\t" << "input ";
		for (i = 0; i < (int)_inputs.size(); i++) {
			if (!(*_inputs.at(i)).getSignUnsigned()) {
				if ((*_inputs.at(i)).getDataWidth() == DATAWIDTH_64) {
					if (k) {
						*outputFile << ", ";
					}
					else {
						*outputFile << "[63:0] ";
					}
					*outputFile << (*_inputs.at(i)).getName();
					k = 1;
				}
			}
		}
		*outputFile << ";" << endl;
	}
	if (sind64) {
		k = 0;
		*outputFile << "\t" << "input signed ";
		for (i = 0; i < (int)_inputs.size(); i++) {
			if ((*_inputs.at(i)).getSignUnsigned()) {
				if ((*_inputs.at(i)).getDataWidth() == DATAWIDTH_64) {
					if (k) {
						*outputFile << ", ";
					}
					else {
						*outputFile << "[63:0] ";
					}
					*outputFile << (*_inputs.at(i)).getName();
					k = 1;
				}
			}
		}
		*outputFile << ";" << endl;
	}
	*outputFile << endl;

	/* Print Outputs. */
	ind1 = false;
	sind1 = false;
	ind2 = false;
	sind2 = false;
	ind8 = false;
	sind8 = false;
	ind16 = false;
	sind16 = false;
	ind32 = false;
	sind32 = false;
	ind64 = false;
	sind64 = false;

	/* Categorize if there are certain datawidths. */
	for (i = 0; i < (int)_outputs.size(); i++) {
		if (!(*_outputs.at(i)).getSignUnsigned()) {
			if ((*_outputs.at(i)).getDataWidth() == DATAWIDTH_1) {
				ind1 = true;
			}
			else if ((*_outputs.at(i)).getDataWidth() == DATAWIDTH_2) {
				ind2 = true;
			}
			else if ((*_outputs.at(i)).getDataWidth() == DATAWIDTH_8) {
				ind8 = true;
			}
			else if ((*_outputs.at(i)).getDataWidth() == DATAWIDTH_16) {
				ind16 = true;
			}
			else if ((*_outputs.at(i)).getDataWidth() == DATAWIDTH_32) {
				ind32 = true;
			}
			else if ((*_outputs.at(i)).getDataWidth() == DATAWIDTH_64) {
				ind64 = true;
			}
		}
		else {
			if ((*_outputs.at(i)).getDataWidth() == DATAWIDTH_1) {
				sind1 = true;
			}
			else if ((*_outputs.at(i)).getDataWidth() == DATAWIDTH_2) {
				sind2 = true;
			}
			else if ((*_outputs.at(i)).getDataWidth() == DATAWIDTH_8) {
				sind8 = true;
			}
			else if ((*_outputs.at(i)).getDataWidth() == DATAWIDTH_16) {
				sind16 = true;
			}
			else if ((*_outputs.at(i)).getDataWidth() == DATAWIDTH_32) {
				sind32 = true;
			}
			else if ((*_outputs.at(i)).getDataWidth() == DATAWIDTH_64) {
				sind64 = true;
			}
		}
	}

	/* Check for each datawidth. */
	if (ind1) {
		k = 0;
		*outputFile << "\t" << "output reg ";
		for (i = 0; i < (int)_outputs.size(); i++) {
			if (!(*_outputs.at(i)).getSignUnsigned()) {
				if ((*_outputs.at(i)).getDataWidth() == DATAWIDTH_1) {
					if (k) {
						*outputFile << ", ";
					}
					*outputFile << (*_outputs.at(i)).getName();
					k = 1;
				}
			}
		}
		*outputFile << ";" << endl;
	}
	if (sind1) {
		k = 0;
		*outputFile << "\t" << "output reg signed ";
		for (i = 0; i < (int)_outputs.size(); i++) {
			if ((*_outputs.at(i)).getSignUnsigned()) {
				if ((*_outputs.at(i)).getDataWidth() == DATAWIDTH_1) {
					if (k) {
						*outputFile << ", ";
					}
					*outputFile << (*_outputs.at(i)).getName();
					k = 1;
				}
			}
		}
		*outputFile << ";" << endl;
	}
	if (ind2) {
		k = 0;
		*outputFile << "\t" << "output reg ";
		for (i = 0; i < (int)_outputs.size(); i++) {
			if (!(*_outputs.at(i)).getSignUnsigned()) {
				if ((*_outputs.at(i)).getDataWidth() == DATAWIDTH_2) {
					if (k) {
						*outputFile << ", ";
					}
					else {
						*outputFile << "[1:0] ";
					}
					*outputFile << (*_outputs.at(i)).getName();
					k = 1;
				}
			}
		}
		*outputFile << ";" << endl;
	}
	if (sind2) {
		k = 0;
		*outputFile << "\t" << "output reg signed ";
		for (i = 0; i < (int)_outputs.size(); i++) {
			if ((*_outputs.at(i)).getSignUnsigned()) {
				if ((*_outputs.at(i)).getDataWidth() == DATAWIDTH_2) {
					if (k) {
						*outputFile << ", ";
					}
					else {
						*outputFile << "[1:0] ";
					}
					*outputFile << (*_outputs.at(i)).getName();
					k = 1;
				}
			}
		}
		*outputFile << ";" << endl;
	}
	if (ind8) {
		k = 0;
		*outputFile << "\t" << "output reg ";
		for (i = 0; i < (int)_outputs.size(); i++) {
			if (!(*_outputs.at(i)).getSignUnsigned()) {
				if ((*_outputs.at(i)).getDataWidth() == DATAWIDTH_8) {
					if (k) {
						*outputFile << ", ";
					}
					else {
						*outputFile << "[7:0] ";
					}
					*outputFile << (*_outputs.at(i)).getName();
					k = 1;
				}
			}
		}
		*outputFile << ";" << endl;
	}
	if (sind8) {
		k = 0;
		*outputFile << "\t" << "output reg signed ";
		for (i = 0; i < (int)_outputs.size(); i++) {
			if ((*_outputs.at(i)).getSignUnsigned()) {
				if ((*_outputs.at(i)).getDataWidth() == DATAWIDTH_8) {
					if (k) {
						*outputFile << ", ";
					}
					else {
						*outputFile << "[7:0] ";
					}
					*outputFile << (*_outputs.at(i)).getName();
					k = 1;
				}
			}
		}
		*outputFile << ";" << endl;
	}
	if (ind16) {
		k = 0;
		*outputFile << "\t" << "output reg ";
		for (i = 0; i < (int)_outputs.size(); i++) {
			if (!(*_outputs.at(i)).getSignUnsigned()) {
				if ((*_outputs.at(i)).getDataWidth() == DATAWIDTH_16) {
					if (k) {
						*outputFile << ", ";
					}
					else {
						*outputFile << "[15:0] ";
					}
					*outputFile << (*_outputs.at(i)).getName();
					k = 1;
				}
			}
		}
		*outputFile << ";" << endl;
	}
	if (sind16) {
		k = 0;
		*outputFile << "\t" << "output reg signed ";
		for (i = 0; i < (int)_outputs.size(); i++) {
			if ((*_outputs.at(i)).getSignUnsigned()) {
				if ((*_outputs.at(i)).getDataWidth() == DATAWIDTH_16) {
					if (k) {
						*outputFile << ", ";
					}
					else {
						*outputFile << "[15:0] ";
					}
					*outputFile << (*_outputs.at(i)).getName();
					k = 1;
				}
			}
		}
		*outputFile << ";" << endl;
	}
	if (ind32) {
		k = 0;
		*outputFile << "\t" << "output reg ";
		for (i = 0; i < (int)_outputs.size(); i++) {
			if (!(*_outputs.at(i)).getSignUnsigned()) {
				if ((*_outputs.at(i)).getDataWidth() == DATAWIDTH_32) {
					if (k) {
						*outputFile << ", ";
					}
					else {
						*outputFile << "[31:0] ";
					}
					*outputFile << (*_outputs.at(i)).getName();
					k = 1;
				}
			}
		}
		*outputFile << ";" << endl;
	}
	if (sind32) {
		k = 0;
		*outputFile << "\t" << "output reg signed ";
		for (i = 0; i < (int)_outputs.size(); i++) {
			if ((*_outputs.at(i)).getSignUnsigned()) {
				if ((*_outputs.at(i)).getDataWidth() == DATAWIDTH_32) {
					if (k) {
						*outputFile << ", ";
					}
					else {
						*outputFile << "[31:0] ";
					}
					*outputFile << (*_outputs.at(i)).getName();
					k = 1;
				}
			}
		}
		*outputFile << ";" << endl;
	}
	if (ind64) {
		k = 0;
		*outputFile << "\t" << "output reg ";
		for (i = 0; i < (int)_outputs.size(); i++) {
			if (!(*_outputs.at(i)).getSignUnsigned()) {
				if ((*_outputs.at(i)).getDataWidth() == DATAWIDTH_64) {
					if (k) {
						*outputFile << ", ";
					}
					else {
						*outputFile << "[63:0] ";
					}
					*outputFile << (*_outputs.at(i)).getName();
					k = 1;
				}
			}
		}
		*outputFile << ";" << endl;
	}
	if (sind64) {
		k = 0;
		*outputFile << "\t" << "output reg signed ";
		for (i = 0; i < (int)_outputs.size(); i++) {
			if ((*_outputs.at(i)).getSignUnsigned()) {
				if ((*_outputs.at(i)).getDataWidth() == DATAWIDTH_64) {
					if (k) {
						*outputFile << ", ";
					}
					else {
						*outputFile << "[63:0] ";
					}
					*outputFile << (*_outputs.at(i)).getName();
					k = 1;
				}
			}
		}
		*outputFile << ";" << endl;
	}
	*outputFile << endl;

	/* Print Wires. */
	ind1 = false;
	sind1 = false;
	ind2 = false;
	sind2 = false;
	ind8 = false;
	sind8 = false;
	ind16 = false;
	sind16 = false;
	ind32 = false;
	sind32 = false;
	ind64 = false;
	sind64 = false;

	/* Categorize if there are certain datawidths. */
	for (i = 0; i < (int)_variables.size(); i++) {
		if (!(*_variables.at(i)).getSignUnsigned()) {
			if ((*_variables.at(i)).getDataWidth() == DATAWIDTH_1) {
				ind1 = true;
			}
			else if ((*_variables.at(i)).getDataWidth() == DATAWIDTH_2) {
				ind2 = true;
			}
			else if ((*_variables.at(i)).getDataWidth() == DATAWIDTH_8) {
				ind8 = true;
			}
			else if ((*_variables.at(i)).getDataWidth() == DATAWIDTH_16) {
				ind16 = true;
			}
			else if ((*_variables.at(i)).getDataWidth() == DATAWIDTH_32) {
				ind32 = true;
			}
			else if ((*_variables.at(i)).getDataWidth() == DATAWIDTH_64) {
				ind64 = true;
			}
		}
		else {
			if ((*_variables.at(i)).getDataWidth() == DATAWIDTH_1) {
				sind1 = true;
			}
			else if ((*_variables.at(i)).getDataWidth() == DATAWIDTH_2) {
				sind2 = true;
			}
			else if ((*_variables.at(i)).getDataWidth() == DATAWIDTH_8) {
				sind8 = true;
			}
			else if ((*_variables.at(i)).getDataWidth() == DATAWIDTH_16) {
				sind16 = true;
			}
			else if ((*_variables.at(i)).getDataWidth() == DATAWIDTH_32) {
				sind32 = true;
			}
			else if ((*_variables.at(i)).getDataWidth() == DATAWIDTH_64) {
				sind64 = true;
			}
		}
	}

	/* Check for each datawidth. */
	if (ind1) {
		k = 0;
		*outputFile << "\t" << "reg ";
		for (i = 0; i < (int)_variables.size(); i++) {
			if (!(*_variables.at(i)).getSignUnsigned()) {
				if ((*_variables.at(i)).getDataWidth() == DATAWIDTH_1) {
					if (k) {
						*outputFile << ", ";
					}
					*outputFile << (*_variables.at(i)).getName();
					k = 1;
				}
			}
		}
		*outputFile << ";" << endl;
	}
	if (sind1) {
		k = 0;
		*outputFile << "\t" << "reg signed ";
		for (i = 0; i < (int)_variables.size(); i++) {
			if ((*_variables.at(i)).getSignUnsigned()) {
				if ((*_variables.at(i)).getDataWidth() == DATAWIDTH_1) {
					if (k) {
						*outputFile << ", ";
					}
					*outputFile << (*_variables.at(i)).getName();
					k = 1;
				}
			}
		}
		*outputFile << ";" << endl;
	}
	if (ind2) {
		k = 0;
		*outputFile << "\t" << "reg ";
		for (i = 0; i < (int)_variables.size(); i++) {
			if (!(*_variables.at(i)).getSignUnsigned()) {
				if ((*_variables.at(i)).getDataWidth() == DATAWIDTH_2) {
					if (k) {
						*outputFile << ", ";
					}
					else {
						*outputFile << "[1:0] ";
					}
					*outputFile << (*_variables.at(i)).getName();
					k = 1;
				}
			}
		}
		*outputFile << ";" << endl;
	}
	if (sind2) {
		k = 0;
		*outputFile << "\t" << "reg signed ";
		for (i = 0; i < (int)_variables.size(); i++) {
			if ((*_variables.at(i)).getSignUnsigned()) {
				if ((*_variables.at(i)).getDataWidth() == DATAWIDTH_2) {
					if (k) {
						*outputFile << ", ";
					}
					else {
						*outputFile << "[1:0] ";
					}
					*outputFile << (*_variables.at(i)).getName();
					k = 1;
				}
			}
		}
		*outputFile << ";" << endl;
	}
	if (ind8) {
		k = 0;
		*outputFile << "\t" << "reg ";
		for (i = 0; i < (int)_variables.size(); i++) {
			if (!(*_variables.at(i)).getSignUnsigned()) {
				if ((*_variables.at(i)).getDataWidth() == DATAWIDTH_8) {
					if (k) {
						*outputFile << ", ";
					}
					else {
						*outputFile << "[7:0] ";
					}
					*outputFile << (*_variables.at(i)).getName();
					k = 1;
				}
			}
		}
		*outputFile << ";" << endl;
	}
	if (sind8) {
		k = 0;
		*outputFile << "\t" << "reg signed ";
		for (i = 0; i < (int)_variables.size(); i++) {
			if ((*_variables.at(i)).getSignUnsigned()) {
				if ((*_variables.at(i)).getDataWidth() == DATAWIDTH_8) {
					if (k) {
						*outputFile << ", ";
					}
					else {
						*outputFile << "[7:0] ";
					}
					*outputFile << (*_variables.at(i)).getName();
					k = 1;
				}
			}
		}
		*outputFile << ";" << endl;
	}
	if (ind16) {
		k = 0;
		*outputFile << "\t" << "reg ";
		for (i = 0; i < (int)_variables.size(); i++) {
			if (!(*_variables.at(i)).getSignUnsigned()) {
				if ((*_variables.at(i)).getDataWidth() == DATAWIDTH_16) {
					if (k) {
						*outputFile << ", ";
					}
					else {
						*outputFile << "[15:0] ";
					}
					*outputFile << (*_variables.at(i)).getName();
					k = 1;
				}
			}
		}
		*outputFile << ";" << endl;
	}
	if (sind16) {
		k = 0;
		*outputFile << "\t" << "reg signed ";
		for (i = 0; i < (int)_variables.size(); i++) {
			if ((*_variables.at(i)).getSignUnsigned()) {
				if ((*_variables.at(i)).getDataWidth() == DATAWIDTH_16) {
					if (k) {
						*outputFile << ", ";
					}
					else {
						*outputFile << "[15:0] ";
					}
					*outputFile << (*_variables.at(i)).getName();
					k = 1;
				}
			}
		}
		*outputFile << ";" << endl;
	}
	if (ind32) {
		k = 0;
		*outputFile << "\t" << "reg ";
		for (i = 0; i < (int)_variables.size(); i++) {
			if (!(*_variables.at(i)).getSignUnsigned()) {
				if ((*_variables.at(i)).getDataWidth() == DATAWIDTH_32) {
					if (k) {
						*outputFile << ", ";
					}
					else {
						*outputFile << "[31:0] ";
					}
					*outputFile << (*_variables.at(i)).getName();
					k = 1;
				}
			}
		}
		*outputFile << ";" << endl;
	}
	if (sind32) {
		k = 0;
		*outputFile << "\t" << "reg signed ";
		for (i = 0; i < (int)_variables.size(); i++) {
			if ((*_variables.at(i)).getSignUnsigned()) {
				if ((*_variables.at(i)).getDataWidth() == DATAWIDTH_32) {
					if (k) {
						*outputFile << ", ";
					}
					else {
						*outputFile << "[31:0] ";
					}
					*outputFile << (*_variables.at(i)).getName();
					k = 1;
				}
			}
		}
		*outputFile << ";" << endl;
	}
	if (ind64) {
		k = 0;
		*outputFile << "\t" << "reg ";
		for (i = 0; i < (int)_variables.size(); i++) {
			if (!(*_variables.at(i)).getSignUnsigned()) {
				if ((*_variables.at(i)).getDataWidth() == DATAWIDTH_64) {
					if (k) {
						*outputFile << ", ";
					}
					else {
						*outputFile << "[63:0] ";
					}
					*outputFile << (*_variables.at(i)).getName();
					k = 1;
				}
			}
		}
		*outputFile << ";" << endl;
	}
	if (sind64) {
		k = 0;
		*outputFile << "\t" << "reg signed ";
		for (i = 0; i < (int)_variables.size(); i++) {
			if ((*_variables.at(i)).getSignUnsigned()) {
				if ((*_variables.at(i)).getDataWidth() == DATAWIDTH_64) {
					if (k) {
						*outputFile << ", ";
					}
					else {
						*outputFile << "[63:0] ";
					}
					*outputFile << (*_variables.at(i)).getName();
					k = 1;
				}
			}
		}
		*outputFile << ";" << endl;
	}
	*outputFile << endl;

	/* Check for all N/A wires are necessary. */
	k = 0;
	j = 0;
	// NO N/A WIRES NECESSARY IN THIS HLSYN.
	/*for (i = 0; i < (int)_datapathComponents.size(); i++) {
		if ((!_datapathComponents.at(i).getName().compare("COMP_lt"))
			|| (!_datapathComponents.at(i).getName().compare("SCOMP_lt"))
			|| (!_datapathComponents.at(i).getName().compare("COMP_gt"))
			|| (!_datapathComponents.at(i).getName().compare("SCOMP_gt"))
			|| (!_datapathComponents.at(i).getName().compare("COMP_eq"))
			|| (!_datapathComponents.at(i).getName().compare("SCOMP_eq"))) {
			j = 1;
			*outputFile << "\t" << "wire ";
			*outputFile << "na" << k << ", ";
			k++;
			*outputFile << "na" << k << ";" << endl;
			k++;
		}
	}*/
	/* Spacing */
	if (j == 1) {
		*outputFile << endl;
	}

	return true;

}

bool HLSM::checkIfComment(std::string checkString)
{
	bool result = false;

	if (checkString.size() >= 2) {
		if (checkString.at(0) == '/' && checkString.at(1) == '/') {
			result = true;
		}
	}

	return result;
}

void HLSM::clearAlgothrimVectors()
{
	_multDistribution.clear();
	_addSubDistribution.clear();
	_modDivDistribution.clear();
	_logicDistribution.clear();
	//_asapSchedule.clear();
	//_alapShcedule.clear();
}

/*	Algorithm which creates the states to print from the FDS. 
	This Algorithm is necessary because of IF/ELSE statements. */
void HLSM::createStates()
{
	int i = 0;
	int j = 0;

	/* Note: Each IF node will create another 2 states necessitated as
	as they can execute at the same time on an FDS graph but cannot
	but executed at the same case statement on a verilog code segment. */

	/* Staging for state.
	This begins with setting the state as the FDS time. */
	for (i = 0; i < (int)_forceDirectedSchedule.size(); i++) {
		createNewState();
		for (j = 0; j < (int)_nodes.size(); j++) {
			if (_nodes.at(j)->getFDSTime() == i) {
				_states.at(i).addAssignedNode(_nodes.at(j));
			}
		}
	}

	/* Remove unnecessary states. */
	for (i = 0; i < (int)_states.size(); i++) {
		if (_states.at(i).getAssignedNodes().size() == 0) {
			_states.erase(_states.begin() + i);
			i--;
		}
	}

	/* Find and adjust for presence of if statements. */
	for (i = 0; i < (int)_states.size(); i++) {
		for (j = 0; j < (int)_states.at(i).getAssignedNodes().size() - 1; j++) {
			/* Check for an conditional statement within the state. */
			if (_states.at(i).getAssignedNodes().at(j)->getOperation().compare("if")) {
				// TODO!
			}
		}
	}

}

void HLSM::createNewState()
{
	State* newState = new State();

	_states.push_back(*newState);
}

//TODO: Add else nodes to the nextElseNodes vector
int HLSM::ifCheckStringIsIf(std::ifstream * inputFile, std::string checkString)
{
	Node* currNode = NULL;
	int nextIfIndex = -1;
	int currNodeIndex = -1;
	int inputIndex = -1;
	int outputIndex = -1;
	int regIndex = -1;

	createNewNode(checkString, _nodes.size() + 1, vector<DataType*>(), vector<DataType*>());
	currNode = _nodes.at(_nodes.size() - 1);
	currNodeIndex = _nodes.size() - 1;
	currNode->setConditional(true);

	//find what the condition is for the if statment
	*inputFile >> checkString;
	*inputFile >> checkString;

	if (!checkVariable(checkString, &outputIndex, &inputIndex, &regIndex)) {
		return -1;
	}
	else {
		if (inputIndex != -1) {
			currNode->addInput(_inputs.at(inputIndex));
			*inputFile >> checkString;
			*inputFile >> checkString;
			*inputFile >> checkString;
		}
		else if (regIndex != -1) {
			currNode->addInput(_variables.at(regIndex));
			*inputFile >> checkString;
			*inputFile >> checkString;
			*inputFile >> checkString;
		}
	}

	while (checkString.compare("}")) {
		if (!checkString.compare("if")) {
			nextIfIndex = createNestedIf(inputFile, checkString);
			currNode->addNextIfNode(_nodes.at(nextIfIndex));
			_nodes[nextIfIndex]->addPreviousNode(currNode);
			*inputFile >> checkString;
		}
		else if (!checkString.compare("else")) {
			/* To always have a fresh set of eyes... */
			outputIndex = -1;
			inputIndex = -1;
			regIndex = -1;
			/* Check where the inputs/outputs/wires are for this datapath component. */
			if (!checkVariable(checkString, &outputIndex, &inputIndex, &regIndex)) {
				cout << "Variable '" << checkString << "' not found, please correct Netlist Behavior File." << endl;
				return -1;
			}
			else {
				/* Create some datapath components. */
				getline(*inputFile, checkString);
				if (outputIndex != -1) {
					if (!determineOperation(checkString, _outputs.at(outputIndex))) {
						return -1;
					}
				}
				else if (regIndex != -1) {
					if (!determineOperation(checkString, _variables.at(regIndex))) {
						return -1;
					}
				}
				_nodes.at(_nodes.size() - 1)->addPreviousNode(_nodes.at(nextIfIndex));
				_nodes[nextIfIndex]->addNextElseNode(_nodes.at(_nodes.size() - 1));
				*inputFile >> checkString;
			}

		}
		else {
			/* To always have a fresh set of eyes... */
			outputIndex = -1;
			inputIndex = -1;
			regIndex = -1;
			/* Check where the inputs/outputs/wires are for this datapath component. */
			if (!checkVariable(checkString, &outputIndex, &inputIndex, &regIndex)) {
				cout << "Variable '" << checkString << "' not found, please correct Netlist Behavior File." << endl;
				return -1;
			}
			else {
				/* Create some datapath components. */
				getline(*inputFile, checkString);
				if (outputIndex != -1) {
					if (!determineOperation(checkString, _outputs.at(outputIndex))) {
						return -1;
					}
				}
				else if (regIndex != -1) {
					if (!determineOperation(checkString, _variables.at(regIndex))) {
						return -1;
					}
				}
				_nodes.at(_nodes.size() - 1)->addPreviousNode(_nodes.at(currNodeIndex));
				currNode->addNextIfNode(_nodes.at(_nodes.size() - 1));
				*inputFile >> checkString;
			}
		}
	}
	return currNodeIndex;
}

int HLSM::createNestedIf(std::ifstream * inputFile, std::string checkString)
{
	Node* currNode = NULL;
	int nextIfIndex = -1;
	int currNodeIndex = -1;
	int inputIndex = -1;
	int outputIndex = -1;
	int regIndex = -1;

	createNewNode(checkString, _nodes.size() + 1, vector<DataType*>(), vector<DataType*>());
	currNode = _nodes.at(_nodes.size() - 1);
	currNodeIndex = _nodes.size() - 1;
	currNode->setConditional(true);

	//find what the condition is for the if statment
	*inputFile >> checkString;
	*inputFile >> checkString;

	if (!checkVariable(checkString, &outputIndex, &inputIndex, &regIndex)) {
		return -1;
	}
	else {
		if (inputIndex != -1) {
			currNode->addInput(_inputs.at(inputIndex));
			*inputFile >> checkString;
			*inputFile >> checkString;
			*inputFile >> checkString;
		}
		else if (regIndex != -1) {
			currNode->addInput(_variables.at(regIndex));
			*inputFile >> checkString;
			*inputFile >> checkString;
			*inputFile >> checkString;
		}
	}

	while (checkString.compare("}")) {
		if (!checkString.compare("if")) {
			nextIfIndex = createNestedIf(inputFile, checkString);
			currNode->addNextIfNode(_nodes.at(nextIfIndex));
			*inputFile >> checkString;
		}
		else if (!checkString.compare("else")) {
			*inputFile >> checkString;
			*inputFile >> checkString;

			/* To always have a fresh set of eyes... */
			outputIndex = -1;
			inputIndex = -1;
			regIndex = -1;
			/* Check where the inputs/outputs/wires are for this datapath component. */
			if (!checkVariable(checkString, &outputIndex, &inputIndex, &regIndex)) {
				cout << "Variable '" << checkString << "' not found, please correct Netlist Behavior File." << endl;
				return -1;
			}
			else {
				/* Create some datapath components. */
				getline(*inputFile, checkString);
				if (outputIndex != -1) {
					if (!determineOperation(checkString, _outputs.at(outputIndex))) {
						return -1;
					}
				}
				else if (regIndex != -1) {
					if (!determineOperation(checkString, _variables.at(regIndex))) {
						return -1;
					}
				}
				_nodes.at(_nodes.size() - 1)->addPreviousNode(_nodes.at(nextIfIndex));
				_nodes[nextIfIndex]->addNextElseNode(_nodes.at(_nodes.size() - 1));
				*inputFile >> checkString;
			}

		}
		else {
			/* To always have a fresh set of eyes... */
			outputIndex = -1;
			inputIndex = -1;
			regIndex = -1;
			/* Check where the inputs/outputs/wires are for this datapath component. */
			if (!checkVariable(checkString, &outputIndex, &inputIndex, &regIndex)) {
				cout << "Variable '" << checkString << "' not found, please correct Netlist Behavior File." << endl;
				return -1;
			}
			else {
				/* Create some datapath components. */
				getline(*inputFile, checkString);
				if (outputIndex != -1) {
					if (!determineOperation(checkString, _outputs.at(outputIndex))) {
						return -1;
					}
				}
				else if (regIndex != -1) {
					if (!determineOperation(checkString, _variables.at(regIndex))) {
						return -1;
					}
				}
				_nodes.at(_nodes.size() - 1)->addPreviousNode(_nodes.at(currNodeIndex));
				currNode->addNextIfNode(_nodes.at(_nodes.size() - 1));
				*inputFile >> checkString;
			}
		}
	}
	return currNodeIndex;
}

bool HLSM::checkLatency(int latency) {
	bool mult = false;
	bool div = false;
	bool other = false;
	bool result = true;
	int i = 0;

	for (i = 0; i < (int)_nodes.size(); i++) {
		if (_nodes.at(i)->getOperation() == "*") {
			mult = true;
		}
		else if (_nodes.at(i)->getOperation() == "/") {
			div = true;
		}
		else {
			other = true;
		}
	}

	if (div) {
		if (latency < 3) {
			result = false;
		}
	}
	if (mult) {
		if (latency < 2) {
			result = false;
		}
	}
	if (other) {
		if (latency < 1) {
			result = false;
		}
	}

	return result;
}

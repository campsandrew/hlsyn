/*
Students: Brett Bushnell (Undergrad), Matt Dzurick (Grad)
Date Create: 12/5/2016
Assignment: 3
File: State.h
Description: State class for hlsyn program
*/

#ifndef STATE_H
#define STATE_H

#include<string>
#include<vector>
#include<cstring>
#include<algorithm>

#include "Node.h"

class State;

class State {
private:
	// int stateNum;
	std::vector<Node*> _assignedNodes;

public:
	/*Methods*/
	// void setStateNum(int n);
	// int getStateNum();
	State();
	void addAssignedNode(Node* addNode);
	std::vector<Node*> getAssignedNodes();
};
#endif // STATE_H
/*
Students: Brett Bushnell (Undergrad), Matt Dzurick (Grad)
Date Create: 10/17/2016
Assignment: 3
File: DataType.h
Description: DataType class for hlsyn program
*/

#ifndef DATATYPE_H
#define DATATYPE_H

#include<string>
#include<vector>
#include<cstring>
#include<algorithm>

class Node;

class DataType {
protected:
	std::string name;
	int dataWidth;
	bool signUnsigned; //used to see if variable is singed or unsigned (1 signed, 0 unsigned)
	std::vector<Node*> comingFrom;
	std::vector<Node*> goingTo;

public:
	/*Methods*/
	std::string getName();
	void setName(std::string n);
	int getDataWidth();
	void setDataWidth(int dW);
	bool getSignUnsigned();
	void setSignUnsiged(bool sU);
	std::vector<Node*> getGoingTo();
	void setGoingTo(std::vector<Node*> gT);
	std::vector<Node*> getComingFrom();
	void setComingFrom(std::vector<Node*> cF);
	void addToGoingTo(Node* newComponent);
	void addToComingFrom(Node* newComponent);

};
#endif // DATATYPE_H
/*
Students: Brett Bushnell (Undergrad), Matt Dzurick (Grad)
Date Create: 10/3/2016
Assignment: 3
File: DataType.cpp
Description: DataType class definitions for hlysn program
*/

#include "DataType.h"
//#include "DatapathComponent.h"

std::string DataType::getName()
{
	return name;
}

void DataType::setName(std::string n)
{
	name = n;
}

int DataType::getDataWidth()
{
	return dataWidth;
}

void DataType::setDataWidth(int dW)
{
	dataWidth = dW;
}

bool DataType::getSignUnsigned()
{
	return signUnsigned;
}

void DataType::setSignUnsiged(bool sU)
{
	signUnsigned = sU;
}

std::vector<Node*> DataType::getGoingTo()
{
	return goingTo;
}

void DataType::setGoingTo(std::vector<Node*> gT)
{
	goingTo = gT;
}

std::vector<Node*> DataType::getComingFrom()
{
	return comingFrom;
}

void DataType::setComingFrom(std::vector<Node*> cF)
{
	comingFrom = cF;
}

void DataType::addToGoingTo(Node* newComponent)
{
	goingTo.push_back(newComponent);
}

void DataType::addToComingFrom(Node* newComponent)
{
	comingFrom.push_back(newComponent);
}


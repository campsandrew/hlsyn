/*
Students: Brett Bushnell (Undergrad), Matt Dzurick (Grad)
Date Create: 10/17/2016
Assignment: 3
File: Variable.cpp
Description: Variable Class for hlsyn program
*/

#include "Variable.h"

Variable::Variable()
{
	name = "Unknown";
	dataWidth = 0;
}

Variable::Variable(std::string n, bool s, int dW)
{
	name = n;
	signUnsigned = s;
	dataWidth = dW;
}


//std::string Wire::getName()
//{
//	return name;
//}
//
//void Wire::setName(std::string n)
//{
//	name = n;
//}
//
//int Wire::getDataWidth()
//{
//	return dataWidth;
//}
//
//void Wire::setDataWidth(int dW)
//{
//	dataWidth = dW;
//}

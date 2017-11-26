/*
Students: Brett Bushnell (Undergrad), Matt Dzurick (Grad)
Date Create: 10/17/2016
Assignment: 3
File: Input.cpp
Description: input Class for hlsyn program
*/

#include "Input.h"

Input::Input()
{
	name = "Unknown";
	signUnsigned = false;
	dataWidth = 0;
}

Input::Input(std::string n, bool sU, int dW)
{
	name = n;
	signUnsigned = sU;
	dataWidth = dW;
}


//std::string Input::getName()
//{
//	return name;
//}
//
//void Input::setName(std::string n)
//{
//	name = n;
//}
//
//int Input::getDataWidth()
//{
//	return dataWidth;
//}
//
//void Input::setDataWidth(int dW)
//{
//	dataWidth = dW;
//}
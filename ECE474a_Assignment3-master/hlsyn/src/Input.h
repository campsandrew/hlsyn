/*
Students: Brett Bushnell (Undergrad), Matt Dzurick (Grad)
Date Create: 10/17/2016
Assignment: 3
File: Input.h
Description: input Class for hlsyn program
*/

#ifndef INPUT_H
#define INPUT_H

#include<string>
#include<vector>
#include<cstring>
#include<algorithm>

#include "DataType.h"

class Input: public DataType{

//private:
	//std::string name;
	//int dataWidth;
	//bool signUnsigned; //used to see if variable is singed or unsigned (1 signed, 0 unsigned)

public:
	/*Constructors*/
	Input();
	Input(std::string n, bool sU, int dW);

	/*Methods*/
	//std::string getName();
	//void setName(std::string n);
	//int getDataWidth();
	//void setDataWidth(int dW);
};

#endif // INPUT_H
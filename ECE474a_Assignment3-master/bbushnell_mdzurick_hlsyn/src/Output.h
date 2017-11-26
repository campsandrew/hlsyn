/*
Students: Brett Bushnell (Undergrad), Matt Dzurick (Grad)
Date Create: 10/17/2016
Assignment: 3
File: Output.h
Description: Output Class for hlsyn program
*/

#ifndef OUTPUT_H
#define OUTPUT_H

#include<string>
#include<vector>
#include<cstring>
#include<algorithm>

#include "DataType.h"

class Output: public DataType {

//private:
	//	std::string name;
	//	bool sign;
	//	int dataWidth;
public:
	/*Constructors*/
	Output();
	Output(std::string n, bool s, int dW);

	/*Methods*/
	//std::string getName();
	//void setName(std::string n);
	//int getDataWidth();
	//void setDataWidth(int dW);
};

#endif // OUTPUT_H

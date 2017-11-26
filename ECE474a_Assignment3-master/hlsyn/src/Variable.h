/*
Students: Brett Bushnell (Undergrad), Matt Dzurick (Grad)
Date Create: 10/17/2016
Assignment: 3
File: Output.h
Description: Output Class for hlsyn program
*/

#ifndef VARIABLE_H
#define VARIABLE_H

#include<string>
#include<vector>
#include<cstring>
#include<algorithm>
#include"DataType.h"


class Variable: public DataType{

//private:
	//	std::string name;
	//	bool sign;
	//	int dataWidth;

public:
	/*constructors*/
	Variable();
	Variable(std::string n, bool s, int dW);

	/*Methods*/
	/*std::string getName();
	void setName(std::string n);
	int getDataWidth();
	void setDataWidth(int dW);*/
};

#endif // VARIABLE_H

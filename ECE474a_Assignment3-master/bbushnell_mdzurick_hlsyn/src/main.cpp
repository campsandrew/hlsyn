/*
Students: Brett Bushnell (Undergrad), Matt Dzurick (Grad)
Date Create: 11/14/2016
Assignment: 3
File: main.cpp
Description: Main function for hlysn program
*/

#include <stdlib.h>
#include "HLSM.h"

using namespace std;

/* Command-line Argument as follows:
hlysn cFile latency verilogFile
*/
int main(int argc, char *argv[])
{

	HLSM newHLSM;

	/* Check for the correct number of arguments */
	if (argc != 4) {
		cout << endl;
		cout << "Usage: " << argv[0] << " cFile latency verilogFile";
		cout << endl << endl;
		return EXIT_FAILURE;
	}

	/* Read in the netlist file */
	if (!newHLSM.readFile(argv[1])) {
		cout << endl;
		cout << "Could not read from the input file " << argv[1] << ".";
		cout << endl << endl;
		return EXIT_FAILURE;
	}

	/* Create the scheduled graph. */
	if (!newHLSM.scheduleGraph(atoi(argv[2]))) {
		cout << endl;
		cout << "Can not schedule operations with latency of " << argv[2] << "." << endl;
		//cout << "Minimum latency for this circuit is " << newHLSM.asapScheduleSize(); 
		cout << endl << endl;
		return EXIT_FAILURE;
	}

	/* Write to the verilog file */
	if (!newHLSM.writeToFile(argv[3])) {
		cout << endl;
		cout << "Could not write to the output file " << argv[3] << ".";
		return EXIT_FAILURE;
	}


	return EXIT_SUCCESS;
}
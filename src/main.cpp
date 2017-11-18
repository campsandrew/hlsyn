/***********************************************************************************
 * File: main.cpp
 * Author: Jason Tran, Andrew Camps
 * NetID: ichikasuto, andrewcamps
 * Date: November 17, 2017
 *
 * Description:
 *
 **********************************************************************************/

#include "module.h"

#define ARG_FILE_IN 1
#define ARG_LATENCY 2
#define ARG_FILE_OUT 3

/**
 * Main function to start program. Reads file for high level code, outputs verilog code and calculates critical
 * path for verilog module
 */
int main(int argc, char *argv[]) {
    Module *module;
    string name = "HLSM";

    /* If arguments found is not 4, display user statement and exit */
    if (argc != 4) {
        cout << "Usage : hlsyn cFile latency verilogFile" << endl;
        return EXIT_FAILURE;
    }

    /* Initialize and build module datapath from input file */
    module = new Module(name);
    if (!module->build_module(argv[ARG_FILE_IN])) {
        cout << "Failed to build module from input file : " << argv[ARG_FILE_IN] << endl;
        return EXIT_FAILURE;
    }

    /* Output datapath module to .v file */
    if(!module->output_module(argv[ARG_FILE_OUT])){
        cout << "Failed to create output module file : " << argv[ARG_FILE_OUT] << endl;
        return EXIT_FAILURE;
    }
    
    return 0;
}

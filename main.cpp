// MAIN.CPP
// program initialization, checks, loop and shutdown
// for Goals application.
// Copyright 2018 Thanasis Karpetis
//
// Licence information following
//

#include <iostream>
#include "goals.h"

int main( int argc, char** argv) {
	// initialize program
	GoalContainer gc;
	// open files
	// read goals
	try {
		gc.loadFile("goals.xml");
	} catch (std::exception &e) {
		std::cerr<<"Exception caught while reading XML file:"<<e.what()<<"\n\n";
		return 1;
	}
	bool done=false;
	// state machine loop
	while ( !done ) {
		// display
		gc.printAll(std::cout);// dump all entries to standard output
		// input
		done=true;
	// action
	} // end of loop
	// save or not (according to user instructions)
	return 0;
}

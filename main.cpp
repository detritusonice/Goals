// MAIN.CPP
// program initialization, checks, loop and shutdown
// for Goals application.
// Copyright 2018 Thanasis Karpetis
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "goals.h"
#include "statemachine.h"

int main( int argc, char** argv) {
	try{
		int res= StateMachine::getInstance().run();
		return res;
	}catch( std::exception &e) {
		std::cerr<<"Exception caught:"<<e.what()<<"\n";
	}
	return 0;
}

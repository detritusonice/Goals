// STATEMACHINE.CPP
// State machine and implementation of main program logic of command line Goals app
// Written in separate file to facilitate testing
// Creation date: 25/Sept/2018
// Copyright 2018 Thanasis Karpetis
//
// MIT Licence:
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "statemachine.h"

GoalContainer gc;// to ease access from all states, defined here to enable testing

STATE StateMachine::stateID=STATE_EXIT;

void ExitMenu::display() { 
	if (gc.isModified())
		std::cout<<"\nsave changes (yes/no):";
	else
		std::cout<<"No changes made to the goal records.\n";
}

void ExitMenu::input() { 
	if (!gc.isModified())	// no changes to the goal data
		return;
	char c;
	std::cin>>c;		// Using cin for brevity instead of std::cin.get()
	if (c=='y' || c=='y') {
		saveChanges=true;
		done=true;
	}
	if (c=='n' || c=='n') {
		saveChanges=false;
		done=true;
	}
}

void ExitMenu::act() {
	if (gc.isModified()) {
		if (saveChanges) {
			std::cout<<"Saving changes...";
			gc.saveFile();
			std::cout<<"done.\n";
		}
		else {
			std::cout<<"Ignoring changes.\n";
		}
	}
	std::cout<<"Goodbye.\n";

	StateMachine::setNextStateID(STATE_EXIT);// signals machine to return.
}


void MainMenu::display() { 
	std::cout<<"GOALS:\n";
	std::cout<<std::setfill('=')<<std::setw(80)<<"\n";
	std::cout<<std::setfill(' ')<<std::setw(40)<<"Name";
	std::cout<<std::setw(12)<<"Priority"<<std::setw(12)<<"%Completed"<<std::setw(12)<<" Unit Cost\n";
	std::cout<<std::setfill('-')<<std::setw(80)<<"\n"<<std::setfill(' ');
	gc.printAll(std::cout);
	std::cout<<std::setfill('=')<<std::setw(80)<<"\n";
	std::cout<<"Main menu: e for exit, t for break the loop, x for exception:";
}

void MainMenu::input() { 
	std::cin>>c; 
} 

void MainMenu::act() { 
	switch(c) {
		case 'E':
		case 'e': StateMachine::setNextStateID(STATE_EXITMENU);break;
		case 't': StateMachine::setNextStateID(STATE_EXIT);break;
		case 'x': throw(std::runtime_error{"YOU TERMINATED ME!!!"});break;
		default: break;
	}
	c=char{0};
}

// set the machine to the new state, pushing previous in the stack
// this is called periodically by the run()'s loop
//
void StateMachine::setState( STATE newStateID ) { // transfer the machine to a new state
	if (state->getStateID() == newStateID) 
		return; // nochange
	if (!sv.empty() && sv.back()->getStateID()==newStateID) {
		popState();
		return;
	}

	State* newstate=nullptr;
	switch (newStateID) {
		case STATE_MAINMENU: newstate= new MainMenu{};break;
		default:break;
	}
	if (newstate!=nullptr) {
		sv.push_back(state);// the previous state
		state=newstate;
		stateID=newStateID;
	}
	else stateID=state->getStateID(); // cancel state change
}	
	
// set machine to the previous state
void StateMachine::popState() {
	delete state;
	state=nullptr;
	if (!sv.empty()) {
	//	std::cout<<"popState() size before pop():"<<sv.size()<<"\n";
		state=sv.back();
		sv.pop_back();
	}
	//std::cout<<"popState() size after pop():"<<sv.size()<<"\n";
}

// state machine's main loop
int StateMachine::run() {
	bool done=false;

	try {
		gc.loadFile("goals.xml");
	} catch( std::exception &e) {
		std::cerr<<" exception caught while reading XML file:"<<e.what()<<"\n\n";
		return 1; //should differentiate the types of exceptions. a nonexistent file should be allowed
	}

	while (!done) {
		if (stateID==STATE_EXIT || state==nullptr ) break;
		
		setState(stateID);
		if (state==nullptr) popState(); 
		state->display();
		state->input();
		state->act();
	}
	return 0;
}

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
#include <unistd.h> // for STDOUT_FILENO

GoalContainer gc;// to ease access from all states, defined here to enable testing

STATE StateMachine::stateID=STATE_EXIT;
struct winsize StateMachine::ws;

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
	if (refresh || nextToShow>0)
	{
		nextToShow=showGoals(nextToShow);
		refresh=false;
	}
	if (verbose)
		std::cout<<"Select action: e(xit), v(erbose mode off),r(efresh display),s(ort) ";
	else
		std::cout<<"e,q,v,r,s: ";
}

int MainMenu::showGoals(int firstRecord) {
	std::cout<<"GOALS:";
	std::cout<<std::setfill(' ')<<std::setw(73)<<"["+gc.getSortPrefs()+"]"<<'\n';
	std::cout<<std::setfill('=')<<std::setw(80)<<"\n";
	std::cout<<std::setfill(' ')<<std::setw(40)<<"Name";
	std::cout<<std::setw(12)<<"Priority"<<std::setw(12)<<"%Completed"<< std::setw(12)<<" Unit Cost\n";
	std::cout<<std::setfill('-')<<std::setw(80)<<"\n"<<std::setfill(' ');

	int res=gc.printAll(std::cout,firstRecord,std::max(1,StateMachine::termHeight()-7));
	if (res==0) 
		std::cout<<std::setfill('=')<<std::setw(80)<<"\n";
	return res;// return next goal record to be shown
}

void MainMenu::input() { 
	std::cin>>c;
       c=std::tolower(c);// enforcing lowercase	
} 

void MainMenu::act() { 
	switch(c) {
		case 'q':
		case 'e': StateMachine::setNextStateID(STATE_EXITMENU);break;
		case 'v': verbose=!verbose;break;
		case 'r': refresh=true;break;
		case 's': StateMachine::setNextStateID(STATE_SORTMENU);break;
		case 't': StateMachine::setNextStateID(STATE_EXIT);break;
		case 'x': throw(std::runtime_error{"YOU TERMINATED ME!!!"});break;
		default: break;
	}
	c=char{0};
}
//-----------------------------------
void SortMenu::display() {
	std::cout<<"Select sorting criterion as a string of 8 characters at maximum. Format:\n";
	std::cout<<"\t [fo]{1,4}\n\twhere f is the field ( [n]ame, [c]ompletion, [u]nitcost, [p]riority )\n";
	std::cout<<"\tand o is order ( [a]scending, [d]escending )\n";
	std::cout<<"Sort option ( '-' cancels sorting):";
}

void SortMenu::input() {
	std::string newString;
	while (newString.empty())	//get rid of newline chars left over by cin, if any
		std::getline(std::cin,newString);
	if (newString=="-")
		newString="";//set the empty sorting string

	if (gc.validateString(newString)) {
		if (sortString!=newString) {
			sortString=newString;
			changed=true;
		}
		done=true;
	}
	else {
		std::cout<<"\nEXAMPLE: napd\t\t to sort by name asc then by priority desc\n\n";
	}
}

void SortMenu::act() {
	if (changed)
		gc.setSortPrefs(sortString);
	if (done)
		StateMachine::setNextStateID(STATE_MAINMENU);
}

//-----------------------------------
// set the machine to the new state, pushing previous in the stack
// this is called periodically by the run()'s loop
//
void StateMachine::setState( STATE newStateID ) { 

	if (state->getStateID() == newStateID) 
		return; // nochange

	if (!sv.empty() && sv.back()->getStateID()==newStateID) {
		popState();
		return;
	}

	State* newstate=nullptr;
	switch (newStateID) {
		case STATE_MAINMENU: newstate= new MainMenu{};break;
		case STATE_SORTMENU: newstate= new SortMenu{gc.getSortPrefs()};break;
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
		state->onPopped();// may need to update the display
	}
	//std::cout<<"popState() size after pop():"<<sv.size()<<"\n";
}

void StateMachine::queryConsoleDimensions() {
	ioctl( STDOUT_FILENO, TIOCGWINSZ, &ws);
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

		queryConsoleDimensions();
		//std::cerr<<"Terminal Dimensions: "<<termHeight()<<" rows x "<<termWidth()<<" columns\n";

		state->display();
		state->input();
		state->act();
	}
	return 0;
}

// STATEMACHINE.H 
// definitions of the state machine and its states for the Goals app
// creation date: 26/Sept/2018
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

#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include "goals.h"
#include <sys/ioctl.h> // for struct_winsize and call to ioctl() in statemachine.cpp


enum STATE {
	STATE_EXIT=0,
	STATE_EXITMENU,
	STATE_MAINMENU,
	STATE_SORTMENU,
	STATE_OPTIONSMENU,
	STATE_DELETE,
	STATE_INVALID
};

//=============================================================================
// Base class for states of the machine
class State {
protected:
	STATE stateID;
	State( STATE id ):stateID{id}{}
				// accessible from derived classes only
public:
	virtual ~State(){}
	virtual void display()=0;
	virtual void input()=0;
	virtual void act() {}
	virtual void onPopped() {};
	STATE getStateID() {return stateID;}

#ifdef TESTING_ACTIVE
	friend class StateTester;
#endif //TESTING_ACTIVE
};

//=============================================================================

class ExitMenu : public State {
	bool saveChanges;
	bool done;
 public:
	ExitMenu():State{STATE_EXITMENU},saveChanges{false},done{false}{}
	void display();
	void input();
	void act();
};

//=============================================================================

class MainMenu : public State {
	bool changed;
	char c;
	bool refresh;
	int nextToShow;
	int showGoals(int firstRecord=0);
	int prevShown; //the first record of the previous screen. used to re-show numbers of records
 public:
	MainMenu():State{STATE_MAINMENU},changed{false},c{0},refresh{true},nextToShow{0},prevShown{0} {}
	void display();
	void input();
	void act();
	void onPopped() {refresh=true;nextToShow=0;}
};
//=============================================================================

class SortMenu : public State {
	bool done;
	bool changed;
	std::string sortString; // "NaPd" means Name ascending, Priority descending, up to 8 characters for now
 public:
	SortMenu(const std::string& oldString):State{STATE_SORTMENU},done{false},
							changed{false},sortString{oldString} {}
	void display();
	void input();
	void act();
};
	
//=============================================================================

class OptionsMenu : public State {
	enum {
		OPTION_BACK,
		OPTION_PAGING,
		OPTION_VERBOSE,
		OPTION_NUMBERS,
		OPTION_HELP,
		NUM_OPTIONS
	};
	bool toggled[NUM_OPTIONS];
 public:
	OptionsMenu():State{STATE_OPTIONSMENU},toggled{false}{}
	void display();
	void input();
	void act();
	void displayOption( int optionID );
	void showFeedback();
};
	
//=============================================================================

class DeleteState: public State {
	int recordID;
	bool commit;
	bool done;
 public:
	DeleteState():State{STATE_DELETE},recordID{-1},commit{false},done{false} {}
	void display();
	void input();
	void act();
};

//=============================================================================
// singleton implementation of the state machine. other option was expose all to states
// and pass a pointer.
class StateMachine {
	bool changed;				// changes made to preferences
	std::vector<State*> sv; 		// acts as a state stack
	State* state; 				// the current state the machine is in
	STATE stateID;
	struct winsize ws;		// containing Linux console dimensions
	GoalContainer gc;

	void setState( STATE newStateID ); 	//push current state, activate new state
	void popState(); 			// return to previous state
	
	void reset();				// achieve proper state and sv initialization
        void wipeStates();			// release all memory used by existing states	

	void queryConsoleDimensions();		// get Terminal dimensions from system
		// default state is stack with an ExitMenu and MainMenu as current 
		
	// singleton. private construction
	StateMachine():changed{false},state{ nullptr },stateID{STATE_EXIT}{
		reset();
	}
 public:
	~StateMachine() {
		wipeStates();
	}
	static StateMachine& getInstance() {
		static StateMachine theMachine;
		return theMachine;
	}
	
	// flags the next state for the machine to transit to		
	void setNextStateID( STATE newID ) { stateID=newID; }
	int termWidth() { return ws.ws_col; }
	int termHeight() { return ws.ws_row; }

	GoalContainer& getGC() {return gc;}

	int queryPrevStateID() { return (!sv.empty()?sv.back()->getStateID():STATE_EXIT);}
	
	int run(); // main loop.

#ifdef TESTING_ACTIVE	
	friend class StateMachineTester;
#endif //TESTING ACTIVE
};

#endif

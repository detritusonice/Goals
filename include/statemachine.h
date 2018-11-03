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

extern GoalContainer gc;// defined in statemachine.cpp

enum STATE {
	STATE_EXIT=0,
	STATE_EXITMENU,
	STATE_MAINMENU,
	STATE_SORTMENU,
	STATE_INVALID
};

//=============================================================================
// Base class for states of the machine
class State {
protected:
	STATE stateID;
	State( STATE id ):stateID{id}{}// accessible from derived classes only
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
	bool verbose;
	bool refresh;
	int nextToShow;
	int showGoals(int firstRecord=0);
 public:
	MainMenu():State{STATE_MAINMENU},changed{false},c{0},verbose{true},refresh{true},nextToShow{0} {}
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

class StateMachine {
	bool changed;				// changes made to preferences
	std::vector<State*> sv; 		// acts as a state stack
	State* state; 				// the current state the machine is in
	static STATE stateID;
	static struct winsize ws;		// containing Linux console dimensions

	void setState( STATE newStateID ); 	//push current state, activate new state
	void popState(); 			// return to previous state
	void queryConsoleDimensions();

 public:
		// default state is stack with an ExitMenu and MainMenu as current 
		
	StateMachine():changed{false},state{ nullptr }{
		state=new ExitMenu{} ; 		// to be pushed by setState 
		setState(STATE_MAINMENU);	// enter the main menu
	}

	~StateMachine() {
		if (state!=nullptr) 
			delete state;
		while (!(sv.empty())) {
			auto p=sv.back();
			sv.pop_back();
			delete p; 
			}
		}
	// flags the next state for the machine to transit to		
	static void setNextStateID( STATE newID ) {
		stateID=newID;
		}

	static int termWidth() {
		return ws.ws_col;
	}

	static int termHeight() {
		return ws.ws_row;
	}
	
	int run(); // main loop.

	friend class State; // in doubt: who should own a goal container? how is access to it given?
#ifdef TESTING_ACTIVE	
	friend class StateMachineTester;
#endif //TESTING ACTIVE
};

#endif

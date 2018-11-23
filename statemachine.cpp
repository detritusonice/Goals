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


void ExitMenu::display() { 
	if (StateMachine::getInstance().getGC().isModified())
		std::cout<<"\nsave changes (yes/no):";
	else
		std::cout<<"No changes made to the goal records.\n";
}

void ExitMenu::input() { 
	if (!StateMachine::getInstance().getGC().isModified())	// no changes to the goal data
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
	if (StateMachine::getInstance().getGC().isModified()) {
		if (saveChanges) {
			std::cout<<"Saving changes...";
			StateMachine::getInstance().getGC().saveFile();
			std::cout<<"done.\n";
		}
		else {
			std::cout<<"Ignoring changes.\n";
		}
	}
	std::cout<<"Goodbye.\n";

	StateMachine::getInstance().setNextStateID(STATE_EXIT);// signals machine to return.
}


void MainMenu::display() { 
	if (refresh || nextToShow>0)
	{
		nextToShow=showGoals(nextToShow);
		refresh=false;
	}
	if ( UserOptions::getInstance().getVerbosity() ) {
		std::cout<<"e(xit), o(ptions), r(efresh), s(ort), d(elete)";
		if (nextToShow>0)
			std::cout<<",n(ext)";
	}
	else {
		std::cout<<"e,q,o,r,s,d";
		if (nextToShow>0)
			std::cout<<",n";
	}
	std::cout<<" :";
}

int MainMenu::showGoals(int firstRecord) {
	prevShown = firstRecord;
	std::cout<<"GOALS:";
	std::cout<<std::setfill(' ')<<std::setw(73)<<"["+UserOptions::getInstance().getSortPrefs()+"]"<<'\n';
	std::cout<<std::setfill('=')<<std::setw(80)<<"\n";
	if (UserOptions::getInstance().getShowNum())
		std::cout<<std::setfill(' ')<<std::setw(5)<<"# ";
	std::cout<<std::setfill(' ')<<std::setw(40)<<"Name";
	std::cout<<std::setw(9)<<"Priority"<<std::setw(12)<<"%Completed"<< std::setw(12)<<" Unit Cost\n";
	std::cout<<std::setfill('-')<<std::setw(80)<<"\n"<<std::setfill(' ');

	int res=StateMachine::getInstance().getGC().printAll(std::cout,firstRecord,
			( UserOptions::getInstance().getPaging()? std::max(1,StateMachine::getInstance().termHeight()-7): 1<<30) );
	if (res==0) 
		std::cout<<std::setfill('=')<<std::setw(80)<<"\n";
	return res;// return next goal record to be shown
}

void MainMenu::input() { 
	std::cin>>c;
       c=std::tolower(c);// enforcing lowercase	
} 

// Event handler, changing states if dictated by user choice
void MainMenu::act() { 
	switch(c) {
		case 'q':
		case 'e': StateMachine::getInstance().setNextStateID(STATE_EXITMENU);break;

		case 'n': if (nextToShow>0) 	//this refreshes only if more records exist
		case 'r': refresh=true;break;

		case 'o': StateMachine::getInstance().setNextStateID(STATE_OPTIONSMENU);break;
		case 's': StateMachine::getInstance().setNextStateID(STATE_SORTMENU);break;
		case 'd': if ( !UserOptions::getInstance().getShowNum()) {
				  UserOptions::getInstance().setShowNum(true);// user needs to choose
				  showGoals(prevShown); // show numbers for the last displayed screen 
				  UserOptions::getInstance().setShowNum(false);// reset option
				  StateMachine::getInstance().setNextStateID(STATE_DELETE);
			  }
			  break;



		default: break;
	}
	c=char{0};
}
//-----------------------------------------------------------------------------------
// Guides the user in inserting a proper sorting string
void SortMenu::display() {
	std::cout<<"Select sorting criterion as a string of 8 characters at maximum. Format:\n";
	std::cout<<"\t [fo]{1,4}\n\twhere f is the field ( [n]ame, [c]ompletion, [u]nitcost, [p]riority )\n";
	std::cout<<"\tand o is order ( [a]scending, [d]escending )\n";
	std::cout<<"Sort option ( '-' cancels sorting):";
}

// Processes the user's string and if valid sets the changed flag.
void SortMenu::input() {
	std::string newString;
	while (newString.empty())	//get rid of newline chars left over by cin, if any
		std::getline(std::cin,newString);
	if (newString=="-")
		newString="";//set the empty sorting string

	if (UserOptions::getInstance().validateString(newString)) {
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
//Sets GoalContainer sorting Preferences string and exits when all done
void SortMenu::act() {
	if (changed)
		UserOptions::getInstance().setSortPrefs(sortString);
	if (done)
		StateMachine::getInstance().setNextStateID(STATE_MAINMENU);
}

//--------------------------------------------------------------------------------
void OptionsMenu::showFeedback() {
	for (int i=0;i<NUM_OPTIONS;i++) {
		if (toggled[i]) {
			displayOption(i);
			toggled[i]=false;
		}
	}
}
// Displays configuration menu and user feedback upon option toggling.
// Side-effect: resets toggled status to false, to prepare for next user choice
void OptionsMenu::display() {
	showFeedback();
	std::cout<<"Options: b(ack), p(aging ->"<<(UserOptions::getInstance().getPaging()?"off":"on");
	std::cout<<"), v(erbose ->"<<(UserOptions::getInstance().getVerbosity()?"off":"on")<<"), ";
	std::cout<<" n(umbers) ->"<<(UserOptions::getInstance().getShowNum()?"off":"on")<<") h(elp) :";
}	

//Displays option status when user toggles some option
void OptionsMenu::displayOption( int optionID ) {
	switch (optionID) {
		case OPTION_VERBOSE:
			std::cout<<"Verbosity is now "<<(UserOptions::getInstance().getVerbosity()?"on":"off")<<"\n\n";break;
		case OPTION_PAGING:	
			std::cout<<"Paging is now "<<(UserOptions::getInstance().getPaging()?"on":"off")<<"\n\n";break;
		case OPTION_NUMBERS:	
			std::cout<<"Record Numbering is now "<<(UserOptions::getInstance().getShowNum()?"on":"off")<<"\n\n";break;
		case OPTION_HELP:	
			std::cout<<"Help:\n"
"Verbosity switches wordiness in the menu prompt, when off just lists available characters\n"
"Paging switches taking into acount the terminal size and splitting Goals list into pages\n"
"Record Numbering displays a relative record ID in the current list to facilitate editing\n"
"Back saves changes and returns to Main menu\n\n";break;
		default: break;
	}
}

//accepts input string from user and toggleds included  options on and off.
//will ignore unused characters and pair multiple occurences of used ones. Odd numbers toggled.
//assuming input is lating ascii compatible
void OptionsMenu::input() {
	std::string optionString;
	while ( optionString.empty() )
		std::getline( std::cin, optionString );

	bool status[26]={false};	
	for (int i=0;i<optionString.length();i++) {
		char c=optionString[i];
		if (!std::isalpha(c))
			continue;
		c=std::tolower(c);
		status[c-'a']=!status[c-'a'];
	}
	toggled[OPTION_BACK]	= status['b'-'a'];
	toggled[OPTION_VERBOSE] = status['v'-'a'];
	toggled[OPTION_PAGING]	= status['p'-'a'];
	toggled[OPTION_NUMBERS] = status['n'-'a'];
	toggled[OPTION_HELP]	= status['h'-'a'];
}

// responds to user input.
void OptionsMenu::act() {
	if ( toggled[OPTION_VERBOSE] ) {
		UserOptions::getInstance().setVerbosity( !UserOptions::getInstance().getVerbosity() );
	}
       	if ( toggled[OPTION_PAGING]  ) {
		UserOptions::getInstance().setPaging( !UserOptions::getInstance().getPaging() );
	}
       	if ( toggled[OPTION_NUMBERS]  ) {
		UserOptions::getInstance().setShowNum( !UserOptions::getInstance().getShowNum() );
	}
	if ( toggled[OPTION_BACK]) {
		showFeedback();// user may have also toggled some other option, show feedback
		StateMachine::getInstance().setNextStateID(STATE_MAINMENU);
	}
}
//--------------------------------------------------------------------------------
// display choices for the deletion of a record. prompt and verification info
void DeleteState::display() {
	if ( recordID == -1 ) 
		std::cout<<" Record number to delete:";
	else {
		std::cout<<" You chose to delete:\n";
		StateMachine::getInstance().getGC().printRecord( std::cout,recordID );
		std::cout<<" are you sure(y/n)?";
	}
}
//--------------------------------------------------------------------------------
// two rounds of input. first the record id in the current set of displayed records
// then the verification of deleting the goal record
void DeleteState::input() {
	if (recordID>=0) {
		char c;
		std::cin>>c;
		c=tolower(c);
		if (c=='y')
			commit=true;
		else std::cout<<" aborting...\n";
		done=true;
	}
	else {
		std::cin>>recordID;
		recordID--;// user chooses 1-based
		if ( !StateMachine::getInstance().getGC().checkRecordID(recordID)) {
			std::cout<<"No such record number in current set\n";
			recordID=-1;
			done=true; // entering a wrong number
		}
	}
}
//--------------------------------------------------------------------------------
// calling upon GoalContainer to remove record id fromt the active records set
// when the user has entered a valid id and verified the removal. Exits on error.
void DeleteState::act() {
	if (recordID>=0 && commit ) {
		std::cout<<" Deleting ..";
		if (StateMachine::getInstance().getGC().deleteRecord(recordID))
			std::cout<<"done.";
		std::cout<<'\n';
	}
	if (done) StateMachine::getInstance().setNextStateID(STATE_MAINMENU);
}
	
//--------------------------------------------------------------------------------
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
		case STATE_SORTMENU: newstate= new SortMenu{UserOptions::getInstance().getSortPrefs()};break;
		case STATE_OPTIONSMENU: newstate= new OptionsMenu{};break;
		case STATE_DELETE: newstate= new DeleteState{};break;
				   
		default:break;
	}
	if (newstate!=nullptr) {
		sv.push_back(state);// the previous state
		state=newstate;
		stateID=newStateID;
	}
	else stateID=state->getStateID(); // cancel state change
}	
//-----------------------------------------------------------------------------

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

// get Linux Console dimensions from system
void StateMachine::queryConsoleDimensions() {
	ioctl( STDOUT_FILENO, TIOCGWINSZ, &ws);
}

// state machine's main loop
int StateMachine::run() {
	bool done=false;
	UserOptions::getInstance().loadFile("options.xml");
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

		if (UserOptions::getInstance().getPaging())
			queryConsoleDimensions();
		gc.sortGoals(); // will sort only when the sorting preference string has been changed
		//std::cerr<<"Terminal Dimensions: "<<termHeight()<<" rows x "<<termWidth()<<" columns\n";

		state->display();
		state->input();
		state->act();
	}
	UserOptions::getInstance().writeFile();
	return 0;
}

// clears up existing states, releasing memory
void StateMachine::wipeStates() {
	if (state!=nullptr)
		delete state;
	while (!sv.empty()) {
		auto p=sv.back();
		sv.pop_back();
		delete p;
	}
}

//since StateMachine is singleton, tests will need to start from scratch.
void StateMachine::reset() {
	wipeStates();

	state=new ExitMenu{} ; 		// to be pushed in sv by setState 
	setState(STATE_MAINMENU);	// enter the main menu
}

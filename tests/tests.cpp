// TESTS.CPP 
// unit tests for Goals application
// using Google Test - needs pthread
// created 14/Sept/2018
// Copyright 2018 Thanasis Karpetis
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include "goals.h"
#include "statemachine.h"

TEST(goal,create) {
	try {
		Goal goal{"Sample goal",100,50,0.01};
		ASSERT_EQ(goal.name,"Sample goal");
		ASSERT_EQ(goal.priority,100);
		ASSERT_EQ(goal.completion,50);
		ASSERT_EQ(goal.unitcost,0.01);
	} catch(...) {
		ASSERT_TRUE(false);
	}
}

// test if printing works as planned. Output to an ostringstream and compare 
TEST(goal,print) {
	Goal goal{"Sample goal",100,50,0.01};
	std::string str("                             Sample goal      100          50       0.01\n");
	std::ostringstream out;
	goal.print(out);
	ASSERT_EQ(str, out.str()); //compare expected to the ostrstreams string
}

//test XMLParser function getHeader
TEST( XMLParser, getHeader ) {
	XMLParser parser{"goalsample.xml"};
	std::string header=parser.getHeader();
	ASSERT_EQ(header,"xml version=\"1.0\" encoding=\"UTF-8\"");
}
//test XMLParser function getLabel
TEST( XMLParser, getLabel ) {
	XMLParser parser{"goalsample.xml"};
	parser.getHeader();// ignoring header, already tested
	std::string root=parser.getLabel();//should ignore leading whitespace
	ASSERT_EQ(root,"goalkeeper");
}
//test XMLParser function absorbComment implicitly. a comment exists between <goalkeeper> and <goal>
TEST( XMLParser, absorbComment_implicit ) {
	XMLParser parser{"goalsample.xml"};
	parser.getHeader();// ignoring header, already tested
	parser.getLabel();// ditto.   between <goalkeeper> root element and first <goal> node
			  // there is an endline <!-- ... --> coment.
	std::string label=parser.getLabel(); // should have ignored the comment and leading w/space
	ASSERT_EQ(label,"goal");
}
// test creation of emplty container
TEST( GoalContainer, createEmpty ) {
	GoalContainer gc;
	ASSERT_EQ(gc.size(),0);
}
// test reading a single goal entry
TEST( GoalContainer, readGoal ) {
	GoalContainer gc;
	XMLParser parser{"goalsample.xml"};
	parser.getHeader();
	parser.getLabel();//root element, tested
	std::string label=parser.getLabel();//"goal"
	Goal goal=gc.readGoal(parser,label);//should fill in the fields
	std::ostringstream out;
	goal.print(out);//dump to string
	std::string str("                             Sample goal      100          50       0.01\n");
	ASSERT_EQ(out.str(),str);
}
//test trying to read from a nonexistent or unreadable file
TEST( GoalContainer, readNonOpenableFile ) {
	GoalContainer gc;
	gc.loadFile("NONEXISTENT");
	ASSERT_EQ(gc.size(),0);
}

//test importing goals from xml file
TEST( GoalContainer, openfile ) {
	GoalContainer gc;
	gc.loadFile("goalsample.xml");//read records from the sample xml file
	ASSERT_EQ(gc.size(),3); // also testing for rejetion of duplicate goals included in input file
	std::ostringstream out;	// create a stringstream to hold the pring output
	gc.printAll(out);	// invoke printall to dump output to the stream
	ASSERT_EQ(out.str(),"                             Sample goal      100          50       0.01\n"
			"                        Create Goals app      100          10       0.1\n"
			"                  Pass All tests at 100%      100         100       0.01\n"	);
}

// test acceptance of sort strings, valid or not
TEST( GoalContainer, validateString ) {
	GoalContainer gc;
	ASSERT_EQ( UserOptions::getInstance().validateString(""),true);		//valid, empty sorting options
	ASSERT_EQ( UserOptions::getInstance().validateString("na"),true);	//valid, name ascending
	ASSERT_EQ( UserOptions::getInstance().validateString("napacdud"),true); //valid, max length, all fields
	ASSERT_EQ( UserOptions::getInstance().validateString("NA"),true);	//valid, uppercase version
	ASSERT_EQ( UserOptions::getInstance().validateString("1d"),false);	//invalid, nonalpha character
	ASSERT_EQ( UserOptions::getInstance().validateString("nap"),false);	//invalid, odd length
	ASSERT_EQ( UserOptions::getInstance().validateString("nana"),false);	//invlaid, repetition of field
	ASSERT_EQ( UserOptions::getInstance().validateString("napacdudna"),false); //invalid, too long
	ASSERT_EQ( UserOptions::getInstance().validateString("nk"),false);	//invalid, inappropriate order character
	ASSERT_EQ( UserOptions::getInstance().validateString("za"),false);	//invalid, inappropriate field character
}



//=================================================================================
//goal container tester class

class GoalTester{
	GoalContainer* gc;
 public:
	GoalTester( GoalContainer* container ): gc{container} {
		if ( container == nullptr )
			throw( std::runtime_error("goal container passed to goaltester was null"));
	}
	~GoalTester() {
		gc=nullptr;
	}

	bool getModified() {return gc->modifiedGoals;}
	void setModified( bool val) { gc->modifiedGoals=val;}

	std::string getFilename() { return gc->filename;}
	void setFilename( const std::string& newname ) { gc->filename=newname;}//no error checking

	std::vector<Goal>* getGoalVector() { return &gc->v;}
	std::vector<int>* getSortedVector() { return &gc->sorted;}
	
	bool isModified() { return gc->isModified();}
	size_t getSize() {return gc->size();}

	int loadFile (const std::string &name) { return gc->loadFile(name);}
	bool saveFile() { return gc->saveFile();}
};
//----------------------------------------------------------------------------------
// ensure saving after reading a file preserves content and order
TEST( GoalContainer, saveFile ) {
	GoalContainer gc;
	GoalTester tester(&gc);

	tester.loadFile("goalsample.xml");
	ASSERT_EQ(tester.getFilename(),"goalsample.xml");
	tester.setFilename("goalSaved.xml");	
	tester.setModified(true);
	ASSERT_EQ(tester.saveFile(),true);

	GoalContainer gc2;
	GoalTester tester2(&gc2);
	tester2.loadFile("goalSaved.xml"); // both gc's use the same UserOptions

	std::vector<Goal> *v1, *v2;
	v1=tester.getGoalVector();
	v2=tester2.getGoalVector();

	ASSERT_EQ(v1->size(),v2->size());

	for (int i=0;i<v1->size();i++)
		ASSERT_EQ((*v1)[i], (*v2)[i]);
	v1=nullptr;
	v2=nullptr;

}
// helper for test sort below
void testOrder( std::vector<int> *sorted, std::vector<int> *reference) {
	ASSERT_TRUE( sorted !=nullptr );
	ASSERT_TRUE( reference !=nullptr);
	ASSERT_EQ( sorted->size(), reference->size() );
	ASSERT_TRUE( *sorted == *reference ); // use STL's comparison
}

// test different ordering options using valid ordering strings in goalsample.xml
TEST( GoalContainer, sort ) {
	GoalContainer gc;
	GoalTester tester(&gc);
	int ver=UserOptions::getInstance().getSortingVer();

	tester.loadFile("goalsample.xml");
	UserOptions::getInstance().setSortPrefs("na");
	gc.sortGoals();
	ASSERT_EQ( "na", UserOptions::getInstance().getSortPrefs() );
	ASSERT_EQ(UserOptions::getInstance().getSortingVer(),++ver);

	std::vector<int> *sorted= tester.getSortedVector();
	ASSERT_TRUE( sorted !=nullptr );

	GoalComparator comp{&gc};

	for (int i=0; i< sorted->size()-1; i++) // test all consecutive pairs
		ASSERT_TRUE( comp( (*sorted)[i], (*sorted)[i+1]));
	
	std::vector<int> reference{0,1,2};// as read from file
	UserOptions::getInstance().setSortPrefs("");		// should disable sorting, ie kepp file order	
	gc.sortGoals();
	testOrder( sorted, &reference);
	ASSERT_EQ(UserOptions::getInstance().getSortingVer(),++ver);

	reference={1,2,0};
	UserOptions::getInstance().setSortPrefs("na"); //ascending by name
	gc.sortGoals();
	testOrder( sorted, &reference);
	ASSERT_EQ(UserOptions::getInstance().getSortingVer(),++ver);

	reference={2,0,1};
	UserOptions::getInstance().setSortPrefs("cd"); //descending by completion
	gc.sortGoals();
	testOrder( sorted, &reference);
	ASSERT_EQ(UserOptions::getInstance().getSortingVer(),++ver);
	
	reference={0,2,1};
	UserOptions::getInstance().setSortPrefs("nd"); // descending by name
	gc.sortGoals();
	testOrder( sorted, &reference);
	ASSERT_EQ(UserOptions::getInstance().getSortingVer(),++ver);

	reference={2,0,1};
	UserOptions::getInstance().setSortPrefs("uacd"); //ascending by unit cost, then descending by completion
	gc.sortGoals();
	testOrder( sorted, &reference);
	ASSERT_EQ(UserOptions::getInstance().getSortingVer(),++ver);

	sorted=nullptr;
}


//=================================================================================
//state machine testing classes
//

class StateMachineTester {
	StateMachine *theMachine;
public:
	StateMachineTester( StateMachine* mac ):theMachine{mac} {
		if (mac==nullptr)
			throw(std::runtime_error("machine passed was null"));
	}
	~StateMachineTester() {
		theMachine = nullptr;// do not destroy by accident...
	}
	State* getCurrentState() {
		return theMachine->state;
	}
	std::vector<State*>* getStateVector() {
		return &theMachine->sv;
	}
	STATE getCurrentStateID() {
		return (theMachine->state==nullptr?
				STATE_INVALID:theMachine->state->getStateID());
	} 
	void setState( STATE next ) { 
		theMachine->setState(next);
	}
	void popState() { 
		theMachine->popState();
	}

	void reset() {
		theMachine->reset();
	}
};

TEST( StateMachine, StateMachineTester ) {
	StateMachineTester tester{&StateMachine::getInstance()};
	tester.reset();
	ASSERT_EQ(tester.getStateVector()->size(),1);//should provide access to private members
}

TEST( StateMachine, create ) {
	StateMachineTester tester{&StateMachine::getInstance()};
	tester.reset();
	ASSERT_EQ(tester.getStateVector()->size(),1);
	ASSERT_EQ(tester.getCurrentStateID(),STATE_MAINMENU);
}

TEST( StateMachine, pop) {
	
	StateMachineTester tester{&StateMachine::getInstance()};
	tester.reset();
	// already tested initial conditions
	tester.popState();
	ASSERT_EQ(tester.getStateVector()->size(),0);
	ASSERT_EQ(tester.getCurrentStateID(),STATE_EXITMENU);
	try{
		tester.popState();//this will even delete the current state
		ASSERT_EQ(tester.getStateVector()->size(),0); //still zero, no exception thrown
		ASSERT_EQ(tester.getCurrentStateID(), STATE_INVALID);//returned when state is nullptr
	} catch(...) {
		ASSERT_TRUE(false);	// no exception should have been thrown
	}
}

TEST( StateMachine, autoPop ) {
	StateMachineTester tester{&StateMachine::getInstance()};
	tester.reset();
	
	tester.setState( STATE_EXITMENU );// this is the previous in the stack. should pop
	ASSERT_EQ( tester.getStateVector()->size(), 0);// popped
	ASSERT_EQ( tester.getCurrentStateID(), STATE_EXITMENU );//correct current state
}

TEST( UserOptions, saveLoadOptions ) {
	UserOptions::getInstance().loadFile("sampleoptions.xml");	//to set the filename
	UserOptions::getInstance().setPaging(false);			//resetting values, file might be edited
	UserOptions::getInstance().setShowNum(false);
	UserOptions::getInstance().setSortPrefs(""); // reset static Options to default (side-effect)
	UserOptions::getInstance().writeFile();		//ready to go.

	std::string sortprefs("na"); 
	UserOptions::getInstance().setSortPrefs( sortprefs );
	int ver=UserOptions::getInstance().getSortingVer();

	UserOptions::getInstance().loadFile("sampleoptions.xml");	
	ver+=(int)(sortprefs!=UserOptions::getInstance().getSortPrefs());
	ASSERT_EQ( UserOptions::getInstance().getSortingVer(),ver);// loadFile uses setSortPrefs once

	UserOptions::getInstance().setVerbosity(false);
	UserOptions::getInstance().setPaging(true);
	UserOptions::getInstance().setShowNum(true);
	UserOptions::getInstance().setSortPrefs("uacd");
	ASSERT_EQ( UserOptions::getInstance().getSortingVer(),ver+1);// following explicit call

	UserOptions::getInstance().writeFile();

	UserOptions::getInstance().loadFile("sampleoptions.xml");
	ASSERT_EQ( UserOptions::getInstance().getVerbosity(),false);
	ASSERT_EQ( UserOptions::getInstance().getPaging(),true);
	ASSERT_EQ( UserOptions::getInstance().getShowNum(),true);
	ASSERT_EQ( UserOptions::getInstance().getSortPrefs(),"uacd");
	ASSERT_EQ( UserOptions::getInstance().getSortingVer(),ver+1); // load had no effect, same sorting string 


	UserOptions::getInstance().setPaging(false);
	UserOptions::getInstance().setShowNum(false);
	UserOptions::getInstance().setSortPrefs(""); // reset static Options to default (side-effect)
	UserOptions::getInstance().writeFile();
}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

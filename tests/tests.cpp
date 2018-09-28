// TESTS.CPP 
// unit tests for Goals application
// using Google Test - needs pthread
// created 14/Sept/2018
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

#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include "goals.h"

using namespace std;// for brevity
// template test
// TEST( category, testinstance ) {
// 	ASSERT_EQ( X, Y );
// }

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
	string str("Sample goal, \t100, \t50, \t0.01\n");
	ostringstream out;
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
	std::string str("Sample goal, \t100, \t50, \t0.01\n");
	ASSERT_EQ(out.str(),str);
}

//test importing goals from xml file
TEST( GoalContainer, openfile ) {
	GoalContainer gc;
	gc.loadFile("goalsample.xml");//read records from the sample xml file
	ASSERT_EQ(gc.size(),1);
	ostringstream out;	// create a stringstream to hold the pring output
	gc.printAll(out);	// invoke printall to dump output to the stream
	ASSERT_EQ(out.str(),"Sample goal, \t100, \t50, \t0.01\n");
}
int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

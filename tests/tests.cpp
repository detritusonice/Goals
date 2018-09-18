/////////////////////////////////////////////////////////
// TESTS.cpp 
//
// unit tests for Goals application
// using Google Test - needs pthread
//
// copyright 2018 Thanasis Karpetis
// created 14/Sept/2018
//
// Licence information follows
//
////////////////////////////////////////////////////////

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

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

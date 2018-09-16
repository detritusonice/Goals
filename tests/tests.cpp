/////////////////////////////////////////////////////////
// tests.cpp 
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
#include "goals.h"

// template test
// TEST( category, testinstance ) {
// 	ASSERT_EQ( X, Y );
// }

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

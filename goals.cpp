// GOALS.CPP
// copyright 2018, Thanasis Karpetis
// main classes used in the Goals application
// Licence
//

#include <iostream>
#include <vector>
#include <algorithm>
#include "goals.h"

using namespace std;

std::ostream& operator <<( std::ostream& out, const Goal &goal) {
	return goal.print(out);
}
//implement classes and main fuction calls


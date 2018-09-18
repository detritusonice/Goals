// GOALS.H
// function declarations and class interfaces
// for the Goals application.
//
// Copyright 2018, Thanasis Karpetis
//
// Licence information following
//

#ifndef GOALS_H
 #define GOALS_H
#include <iostream>

class Goal {
public://just temporary, until gettors and settors are in place
	std::string name;
	int priority;
	int completion;
	double unitcost;

	Goal(const std::string &gn="", int gp=0, int gc=0, double uc=0.):
		name{gn},priority{gp},completion{gc},unitcost{uc} {}
	
	std::ostream& print( std::ostream &strm ) const {
		strm<<name<<", \t"<<priority<<", \t"<<completion<<", \t"<< unitcost<<'\n';
		return strm;
	}
};

std::ostream& operator <<( std::ostream& out, const Goal& goal);

#endif

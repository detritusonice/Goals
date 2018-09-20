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
#include <vector>
#include <fstream>

class XMLParser;
//==========Goal============================================

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

	bool operator <( const Goal &other) const {
		return name<other.name;
	}

};

//========= GoalContainer ===================================

class GoalContainer {
	std::vector<Goal> v;
 public:
	GoalContainer(){}

	std::ostream& printAll( std::ostream &strm) const;

	size_t size() {
		return v.size();
	}

	int loadFile( const std::string &name );
	Goal readGoal(XMLParser &p, std::string &label);
};

//======== XMLParser =======================================

class XMLParser {
	mutable std::ifstream in;
 public:
	XMLParser( std::string name ):in{name} {}
		
	//read the XML header raw string
	std::string getHeader();
	//read the following label, thow on error
	std::string getLabel();
	//assuming reading pointer just after the starting label, read LEAF data up to next <.
	std::string getLeafData(); 
	bool moreToGo() { return in.good()&& in.peek()!=EOF; }
	void absorbComment();

};

std::ostream& operator <<( std::ostream& out, const Goal& goal);

#endif

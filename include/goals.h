// GOALS.H
// function declarations and class interfaces
// for the Goals application.
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

#ifndef GOALS_H
 #define GOALS_H
#include <iostream>
#include <vector>
#include <fstream>
#include <iomanip>

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
		strm<<std::setfill(' ')<<std::setw(40)<<name;
		strm<<std::setw(12)<<priority<<std::setw(12)<<completion<<std::setw(7)<<""<<unitcost<<'\n';
		return strm;
	}

	bool operator <( const Goal &other) const {
		return name<other.name;
	}
	
	bool operator ==( const Goal &other) const {
		return (priority==other.priority && completion==other.completion && 
				unitcost==other.unitcost && name==other.name  );
	}
};

//========= GoalContainer ===================================

class GoalContainer {
	bool modifiedGoals;
	std::string filename;
	std::vector<Goal> v;
	std::string sortPrefs;
	std::vector<int> sorted;
 public:
	GoalContainer():modifiedGoals{false},sortPrefs{"na"} {}

	std::ostream& printAll( std::ostream &strm) const;

	size_t size() {
		return v.size();
	}

	void setSortPrefs(std::string newPrefs);

	bool isModified() {
		return modifiedGoals;
	}
	std::string getSortPrefs() const {return sortPrefs;}

	bool validateString( std::string candidatePrefs );
	int loadFile( const std::string &name );
	bool saveFile();
	Goal readGoal(XMLParser &p, std::string &label);
	void sortGoals();
	friend class GoalComparator;
#ifdef TESTING_ACTIVE
	friend class GoalTester;
#endif //TESTING_ACTIVE
};

class GoalComparator {
	GoalContainer *gc;
public:
	GoalComparator( GoalContainer* cont ):gc{cont} {}

	bool operator()( const int& a, const int& b);
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

//======== XMLWriter ========================================

class XMLWriter {
	mutable std::ofstream out;
	int indentLevel;
	std::vector<std::pair<std::string,bool>> labelStack; // keeps open label hierarchy 

	//write leading tabs
	bool indent() {
		for (int i=0;i<indentLevel;i++)
			out<<'\t';
	}
 public:
	XMLWriter( const std::string& name ):out{name},indentLevel{0} {
		labelStack.clear();
	} 
	//write header string
	void writeHeader();
	//write openlabel, push label in label stack, increment indent level, optionally start new line
	void openLabel(const std::string& label, bool newline=false);
	//write closelabel, pop label from label stack,reduce indent level, start a new line
	void closeLabel();
	//write a whole goal entry
	void writeGoal( const Goal& goal);
};
	
std::ostream& operator <<( std::ostream& out, const Goal& goal);

#endif

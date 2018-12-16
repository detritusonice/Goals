// GOALS.H
// class declarations of Goals, GoalContainer, GoalComparator,
// 			 XMLParser, XMLWriter and UserOptions 
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
#include <set>
#include <map>

class XMLParser;
class XMLWriter;
class UserOptions;

//==========Goal============================================

struct Goal {
	std::string name;
	int priority;
	int completion;
	double unitcost;

	Goal(const std::string &gn="", int gp=0, int gc=0, double uc=0.):
		name{gn},priority{gp},completion{gc},unitcost{uc} {}
	
	std::ostream& print( std::ostream &strm ) const {
		strm<<std::setfill(' ')<<std::setw(40)<<name;
		strm<<std::setw(9)<<priority<<std::setw(12)<<completion<<std::setw(7)<<""<<unitcost<<'\n';
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

std::ostream& operator <<( std::ostream& out, const Goal& goal);

//========= GoalContainer ===================================

class GoalContainer {
	bool modifiedGoals;
	std::string filename;
	std::vector<Goal> v;	// Unique storage of Goal records in raw order as read from file
				// additions and modifications are stored in this vector

	std::set<int> active;	// contains indices of v which are not deleted. Used as base for search
	std::map<std::string,int> names; // goal labels must be unique, facilitating enforcement
//	std::set<int> searchRes; // active indices of v which are included in search results. Used as
				 // base for 'sorted' initialisation. 

	std::vector<int> sorted;// will contain the proper order of v's indices when sorted
	int sortver;
	bool refreshSort; //any modification will raise this flag to signify need to refresh ordering.
 public:
	GoalContainer():modifiedGoals{false},sortver{-1},refreshSort{true} {}

	void printRecord( std::ostream &strm, int id ) { v[sorted[id]].print(strm);}
	int printAll( std::ostream &strm,int first=0,int maxToPrint=1000) const;

	size_t size() { return v.size(); }

	bool isModified() { return modifiedGoals; }

	int loadFile( const std::string &name );
	bool saveFile();

	Goal readGoal(XMLParser &p, std::string &label);
	void writeGoal( XMLWriter& writer, const Goal& goal); 
	void insertGoal( const Goal& newGoal);

	bool checkRecordID( int recordID );// confirm id in current displayed set.
	bool deleteRecord( int recordID ); // remove from active goal record set.

	int findNameIndex( std::string& name );


	void sortGoals();
	friend class GoalComparator;

#ifdef TESTING_ACTIVE
	friend class GoalTester;
#endif //TESTING_ACTIVE
};

class GoalComparator {
	GoalContainer *gc;
public:
	GoalComparator( GoalContainer* cont):gc{cont}{} 
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
	void writeLeaf( const std::string &label, const std::string &data);
};
	

//==============UserOptions Singleton==============================================

class UserOptions {
	bool verbosity;
	bool paging;
	bool showNumbers;
	std::string sortPrefs;  // field-order pairs, in lowercase. used by comparator object
	std::string filename;
	int sortingver;// will increase to indicate a new sorting string set.
	
	//private constructor, singleton
	UserOptions():verbosity{true},paging{false},showNumbers{false},sortPrefs{""},sortingver{0} {} 
public:
	static UserOptions& getInstance() { 
		static UserOptions userOptions; // the first and only instance created.
		return userOptions;
	}
	int getSortingVer() { return sortingver;} // enabling goalcontainers to update sorting order 

	UserOptions( UserOptions &a) 	= delete;// copy constructor
	UserOptions( UserOptions &&a) 	= delete;// move constructor

	UserOptions& operator=( UserOptions &a) 	= delete; // copy assignment
	UserOptions&& operator=( UserOptions &&a) 	= delete; //move assignment

	void loadFile( const std::string &fname);
	void writeFile();

	bool getVerbosity() {return verbosity;}
	bool getPaging() { return paging; }
	bool getShowNum() { return showNumbers; }

	bool validateString( std::string candidatePrefs );
	std::string getSortPrefs() const {return sortPrefs;}

	void setVerbosity( bool newvalue ) { verbosity = newvalue; }
	void setPaging( bool newvalue ) { paging = newvalue; }
	void setShowNum( bool newvalue ) { showNumbers = newvalue; }
	void setSortPrefs(std::string newPrefs);
	
	friend class GoalComparator;// to avoid 3 function calls for every iteration of the comparator
};

#endif

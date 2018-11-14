// GOALS.CPP
// main classes used in the Goals application
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

#include <set>
#include <algorithm>
#include "goals.h"

int UserOptions::sortingver=0;//static, sorting string version during this run

std::ostream& operator <<( std::ostream& out, const Goal &goal) {
	return goal.print(out);
}

// dumps the goal vector's entries to a stream

int GoalContainer::printAll(std::ostream& strm,int first, int maxToPrint) const {
		if (sorted.empty()) 
			return 0;
		int idx;
		for ( idx = first; idx < sorted.size() && idx < first + maxToPrint; idx++ )
			v[sorted[idx]].print(strm);
		return idx%sorted.size(); 
	}

// loads unique named goal entries from specified file
// side effect: vector of goals is wiped clean to contain only the new entries.

int GoalContainer::loadFile( const std::string &name) {
	v.clear();
	filename= name;//store the filename of the container's records for saving
	std::set<Goal> gs;
	try {

		XMLParser parser{name};
		std::string header = parser.getHeader();
		std::string root = parser.getLabel(); // root element, <goalkeeper>
		std::string label= parser.getLabel(); // read the first <goal> label
		std::string endLabel = std::string{"/"} + root;
		while (label != endLabel && parser.moreToGo()) {
			if (label=="goal") {
				Goal goal= readGoal(parser,label);//given the label, load the struct
				
				auto res=gs.insert(goal);
				if ( res.first!=gs.end() && res.second ) // succeeded, not a duplicate
					v.push_back(goal);	//only load unique records, keep initial order

				label = parser.getLabel();//read the next label
			}
			else throw(std::runtime_error("Entries of a different type detected"));
		}	
	} catch(std::exception &e){
		std::cerr<<"exception caught: "<<e.what()<<'\n';
	}
	sortGoals();
	return v.size();
}

// saves xml file containing goal records disregarding sort order.
// sideEffect: creates a .bak file of the existing (supposedly original) file
// before overwriting

bool GoalContainer::saveFile() {
	if ( isModified() ) {
		std::cerr<<"creating backup file "<<filename<<".bak\n";
		system( ("cp "+filename+" "+filename+".bak -f").c_str() );
		std::cerr<<"saving to "<<filename<<"...";
		//create backup
		try{
			XMLWriter writer{filename};
			writer.writeHeader();
			writer.openLabel("goalkeeper",true); //root element
			for (auto& goal:v)
				writeGoal( writer,goal);
			writer.closeLabel();
		} catch (std::exception& e) {
			std::cerr<<"Exception caught while saving file:"<<e.what()<<"\n";
			return false;
		}
	}
	return true;
}

// read a goal record, consisting of leaf records, discarding comment entries
// Prerequisite: parser has read the file up to a label="goal"

Goal GoalContainer::readGoal( XMLParser &parser, std::string &label) {
	Goal goal;
	std::string leafLabel=parser.getLabel();
	std::string endLabel{"/"};
	endLabel+=label;		// forming the /goal ending label.
	
	while (leafLabel != endLabel && parser.moreToGo()) {
		std::string data=parser.getLeafData();
		if (leafLabel == "name")
			goal.name=data; // would a move be more efficient or is it done implicitly?
		else if (leafLabel == "priority")
		       goal.priority = std::stoi(data); 
		else if (leafLabel == "completion")
			goal.completion = std::stoi(data);
		else if (leafLabel == "unitcost")
			goal.unitcost = std::stod(data);
		else throw(std::runtime_error(leafLabel +": unknown label in leaf data"));	
		std::string dataEnd{"/"+leafLabel};
		if (parser.getLabel() !=dataEnd)
			throw( std::runtime_error(leafLabel +": Leaf data label does not close properly"));
		leafLabel=parser.getLabel();// if it throws, will be caught by openfile
	}
	return goal;
}

//write a whole goal entry

void GoalContainer::writeGoal( XMLWriter& writer, const Goal& goal) {
	static char buf[30];
	writer.openLabel("goal",true);
	writer.writeLeaf("name",goal.name);
	writer.writeLeaf("priority",std::to_string(goal.priority));
	writer.writeLeaf("completion",std::to_string(goal.completion));

	sprintf(buf,"%.2lf",goal.unitcost);

	writer.writeLeaf("unitcost",buf);
	writer.closeLabel();// goal
}

// Estblishes the new order of v's indices based on the new sorting string
// by utilizing a recursive comparator
// Note: may be called after an insert or deletion so re-creating the sorted vector is necessary

void GoalContainer::sortGoals() {
	if (sortver==UserOptions::getInstance().getSortingVer())
		return;
	sorted.clear();		//contains the sequence of indices of v, when properly ordered
	sorted.resize(v.size());
	for (int i=0;i<v.size();i++)
		sorted[i]=i;
	GoalComparator comp{this}; // build a comparator object to act on this container
	std::sort(sorted.begin(),sorted.end(),comp);
	sortver=UserOptions::getInstance().getSortingVer();
}
// comparator objects function operator, to be called recursively from std::sort and work based on depth and 
// comparison preferences string in GoalContainer
// Note: using a static to mark recursive use of the sorting string

bool GoalComparator::operator()( const int &a, const int &b ) {
	static int depth=0;	//progressing by pair of characters 
	
	if (depth==UserOptions::getInstance().sortPrefs.length()) 	// no remaining sort fields, 
		return a<b;			//return physical file order of records 
	char c=UserOptions::getInstance().sortPrefs[depth];
	char o=UserOptions::getInstance().sortPrefs[depth+1];
	bool lt,eq; 
	switch (c) {
		case 'n': lt=( gc->v[a].name < gc->v[b].name );
			  eq=( gc->v[a].name == gc->v[b].name ); 
			  break;
		case 'p': lt=( gc->v[a].priority < gc->v[b].priority );
			  eq=( gc->v[a].priority == gc->v[b].priority ); 
			  break;
		case 'c': lt=( gc->v[a].completion < gc->v[b].completion );
			  eq=( gc->v[a].completion == gc->v[b].completion ); 
			  break;
		case 'u': lt=( gc->v[a].unitcost < gc->v[b].unitcost );
			  eq=( gc->v[a].unitcost == gc->v[b].unitcost ); 
			  break;
	} 
	if (eq) {			//fields are equal
		depth+=2;		// mark next field, if one exists
		bool res=(*this)(a,b);	//recursive call for the next field
		depth-=2;
		return res;
	} 
	return ( (o=='a')?lt:!lt);	//can be ordered based on this field, <less than> holds.
}

//Load user display and sort options
void UserOptions::loadFile( const std::string &fname) {
	filename= fname;
	try {
		XMLParser parser{fname};
		parser.getHeader();
		std::string label=parser.getLabel();
		std::string endlabel=std::string{"/"}+label;
		label=parser.getLabel();
		while (label !=endlabel  && parser.moreToGo()) {
			std::string data = parser.getLeafData();
			if (label=="verbosity")
				verbosity=(data=="true");
			else if (label=="paging")
				paging = (data=="true");
			else if (label=="sort")
				setSortPrefs(data);
			else throw (std::runtime_error(label+" :unknown leaf label in "+fname));
			std::string dataend{ "/"+label};
			label = parser.getLabel();
			if (label!=dataend)
				throw( std::runtime_error("Error, leaf end label <"+dataend+"> missing"));

			label=parser.getLabel(); // read label for the next record, or the root end
		}
	} catch ( std::exception &e ) {
		std::cerr<<e.what();
	}
}

// Write user options to xml file after creating a backup of the original
void UserOptions::writeFile() {
	std::cerr<<"creating options backup file "<<filename<<".bak\n";
	system( ("cp "+filename+" "+filename+".bak -f").c_str() );
	std::cerr<<"saving to "<<filename<<"...\n";
	try{
		XMLWriter writer{filename};
		writer.writeHeader();
		writer.openLabel("options",true); //root element
		writer.writeLeaf("verbosity",(verbosity?"true":"false"));
		writer.writeLeaf("paging",(paging?"true":"false"));
		writer.writeLeaf("sort",sortPrefs);
		writer.closeLabel();
	} catch (std::exception& e) {
		std::cerr<<"Exception caught while saving file:"<<e.what()<<"\n";
		}
}

// called to validate the format of a candidate string of sorting preferences

bool UserOptions::validateString( std::string candidatePrefs) {
	if (candidatePrefs.length()&1) 
		return false;		// no odd length accepted. field-order pairs only
	if (candidatePrefs.length()>8)
		return false;
	if (candidatePrefs.empty()) 
		return true;		// empty string sets order as not-sorted, use file order
	enum {
		INVALID=0,
		AVAILABLE,
		USED
	};
	for (int i=0;i<candidatePrefs.length();i++) {	//ensure all alphabetic and lowercase
		if (!std::isalpha(candidatePrefs[i]))
			return false;
		candidatePrefs[i]=std::tolower(candidatePrefs[i]);
	}
	int fields[26]={INVALID}; // all possible chars, will mark valid and catch duplicates

	fields['n'-'a'] = fields['p'-'a'] =
	fields['u'-'a'] = fields['c'-'a'] = AVAILABLE;

	for (int i=0;i<candidatePrefs.length();i+=2) {
		char c=candidatePrefs[i];
		if (fields[c-'a']!=AVAILABLE) 
			return false;		//invalid char or used more than once

		char o=candidatePrefs[i+1]; //each pair contains a field and an order specifier
		if (o!='a' && o!='d')	
			return false; 		// not specifying order
		fields[c-'a']=USED;
	}
	return true;
}

// sets new sort string in lowercase chars, then sorts the goal records
// Prerequisites: sorting string must be valid.
// 		  the new string is presumably checked to be different than the previous one

void UserOptions::setSortPrefs(std::string newPrefs) {
	for(auto &c:newPrefs)
		c=std::tolower(c);// must be lowercase to avoid unnecessary complexity
	if (validateString(newPrefs)) {
		sortPrefs=newPrefs; // string must be valid
		sortingver++; // indicate that re-sorting will be necessary
	}
}


// GOALS.CPP
// main classes used in the Goals application
// Copyright 2018 Thanasis Karpetis
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <set>
#include <algorithm>
#include "goals.h"

std::ostream& operator <<( std::ostream& out, const Goal &goal) {
	return goal.print(out);
}

// dumps the goal vector's entries to a stream

int GoalContainer::printAll(std::ostream& strm,int first, int maxToPrint) const {
	if (sorted.empty()) 
		return 0;
	int idx;
	for ( idx = first; idx < sorted.size() && idx < first + maxToPrint; idx++ ) {
		if (UserOptions::getInstance().getShowNum() ) 
			std::cout<<std::setfill(' ')<<std::setw(4)<<idx+1<<".";
		v[sorted[idx]].print(strm);
	}
	return idx%sorted.size(); 
}

// insert a new goal in the goal vector, also adding to helper structures
// side effect: sets modifiedGoals and refreshSort to true
//
void GoalContainer::insertGoal( const Goal& goal ) {
	if (goal.name.empty()) return;    // name is mandatory
	auto res=names.insert(std::make_pair(goal.name,(int)v.size())); // add to names set, if not already in

	if ( res.first!=names.end() && res.second ) { // succeeded, not a duplicate
		v.push_back(goal);	//unique records, keep initial order
		active.insert( active.end(),active.size()); //hint insert at the end
	}
	modifiedGoals=true;	
	refreshSort=true; // signify re-sorting is in order
}

// loads unique named goal entries from specified file
// side effect: vector of goals is wiped clean to contain only the new entries.

int GoalContainer::loadFile( const std::string &name) {
	v.clear();
	active.clear();
	names.clear();

	filename= name;//store the filename of the container's records for saving
	try {

		XMLParser parser{name};
		std::string header = parser.getHeader();
		std::string root = parser.getLabel(); // root element, <goalkeeper>
		std::string label= parser.getLabel(); // read the first <goal> label
		std::string endLabel = std::string{"/"} + root;
		while (label != endLabel && parser.moreToGo()) {
			if (label=="goal") {
				Goal goal= readGoal(parser,label);//given the label, load the struct
				insertGoal(goal);	
				label = parser.getLabel();//read the next label
			}
			else throw(std::runtime_error("Entries of a different type detected"));
		}	
	} catch(std::exception &e){
		std::cerr<<"exception caught: "<<e.what()<<'\n';
	}
	sortGoals();
	modifiedGoals=false;
	return v.size();
}

// saves xml file containing goal records disregarding sort order.
// sideEffect: creates a .bak file of the existing (supposedly original) file
// before overwriting

bool GoalContainer::saveFile() {
	if ( isModified() ) {
		std::cerr<<"creating backup file "<<filename<<".bak\n";
		system( ("cp "+filename+" "+filename+".bak -f").c_str() );
		std::cerr<<"saving to "<<filename<<"...\n";
		//create backup
		try{
			XMLWriter writer{filename};
			writer.writeHeader();
			writer.openLabel("goalkeeper",true); //root element

			for (auto idx:active) //skip the deleted records
				writeGoal( writer,v[idx]);
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
	if ( !refreshSort  &&  sortver==UserOptions::getInstance().getSortingVer())//no reordering needed
		return;
	sorted.clear();		//will contains the sequence of indices of live goals in v, 
				//when properly ordered
	for (int idx:active) 
		sorted.push_back(idx);
	GoalComparator comp{this}; // build a comparator object to act on this container
	std::sort(sorted.begin(),sorted.end(),comp);
	sortver=UserOptions::getInstance().getSortingVer();
	refreshSort=false;
}

// confirm id in current displayed set.
bool GoalContainer::checkRecordID( int recordID ) {
	return (recordID>=0 && recordID<sorted.size());
}

// remove from active goal record set.
bool GoalContainer::deleteRecord( int recordID ) {
	int globalID=sorted[recordID]; 		// get the absolute record ID
	try {
	active.erase(globalID);			//remove from active records
	} catch ( std::exception e) {
		std::cerr<<"error while deleting goal:"<<e.what()<<'\n';
		return false;
	}
	names.erase(v[globalID].name);		// remove goal name from used name set
	//searchRes.erase(globalID);		// remove from search results
	sorted.erase(sorted.begin()+recordID);// removing the record will not necessitate a new sorting
						// dependend records should be reloaded, but is wasteful.
	modifiedGoals=true;			//changes made, should ask about saving on exit 
	return true;
}

//modifies a goal record given its record id and new values. attentds to index and sorting updating
bool GoalContainer::modifyRecord( int recordID, const Goal& newvals ) {
	int globalID = sorted[recordID];

	int idx=findNameIndex( newvals.name );
	if (idx>=0 && idx != globalID ) // another goal with the same name exists
		return false;
	if (idx<0) // a new name
		names.erase(v[globalID].name); // remove from the names map
	v[globalID]=newvals;		// change the goal record
	if (idx<0)
		names.insert(make_pair(newvals.name,globalID)); // re-insert into names map
	refreshSort=true; 
	modifiedGoals=true;
	return true;
}

//returns in copy the values of the original record, identified by record ID
int GoalContainer::getGoalByRecordID( int recordID , Goal& copy) {
	if (checkRecordID(recordID)) {
		int globalID = sorted[recordID];
		copy = v[globalID];
		return globalID;
	}
	return -1;
}
	
int GoalContainer::findNameIndex( const std::string& name ) const{
	auto res=names.find(name);
	if (res !=names.end())
		return res->second;// the index of the goal record in V
	return -1;//non-existent
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
			else if (label=="numbers")
				showNumbers = ( data == "true" );
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
		writer.writeLeaf("numbers",(showNumbers?"true":"false"));
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


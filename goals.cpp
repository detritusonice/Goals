// GOALS.CPP
// copyright 2018, Thanasis Karpetis
// main classes used in the Goals application
// Licence
//

#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include "goals.h"

using namespace std;

std::ostream& operator <<( std::ostream& out, const Goal &goal) {
	return goal.print(out);
}
// dumps the goal vector's entries to a stream
std::ostream& GoalContainer::printAll(std::ostream& strm) const {
		for (auto rec : v)
			rec.print(strm);
		return strm;
	}
// loads unique named goal entries from specified file
// side effect: vector of goals is wiped clean to contain only the new entries.
int GoalContainer::loadFile( const std::string &name) {

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
				gs.insert(goal);	//should perform some error detection.
				label = parser.getLabel();//read the next label
			}
			else throw std::runtime_error("Entries of a different type detected");
		}	
	} catch(std::exception &e){
		cerr<<"exception caught: "<<e.what()<<'\n';
	}
	v.clear();
	std::copy(gs.begin(),gs.end(),std::back_inserter(v));			
	return v.size();
}

// read a goal record, consisting of leaf records, discarding comment entries
// Prerequisite: parser has read the file up to a label="goal"
Goal GoalContainer::readGoal( XMLParser &parser, std::string &label) {
	Goal goal;
	std::string leafLabel=parser.getLabel();
	std::string endLabel{"/"};
	endLabel+=label;// forming the /goal ending label.
	
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
		else throw(runtime_error(leafLabel +": unknown label in leaf data"));	
		std::string dataEnd{"/"+leafLabel};
		if (parser.getLabel() !=dataEnd)
			throw( runtime_error(leafLabel +": Leaf data label does not close properly"));
		leafLabel=parser.getLabel();// if it throws, will be caught by openfile
	}
	
	return goal;
}


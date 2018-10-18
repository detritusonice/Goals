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
	return v.size();
}

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
				writer.writeGoal(goal);
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
		else throw(std::runtime_error(leafLabel +": unknown label in leaf data"));	
		std::string dataEnd{"/"+leafLabel};
		if (parser.getLabel() !=dataEnd)
			throw( std::runtime_error(leafLabel +": Leaf data label does not close properly"));
		leafLabel=parser.getLabel();// if it throws, will be caught by openfile
	}
	
	return goal;
}


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

// read the XML file's header and return it withouth the <? and ?> sequences
// the result might possibly be empty on error. Do not care for the moment
// Prerequisite: XMLParser has been initialized with a valid file
std::string XMLParser::getHeader() {
	std::string header;
	if (in.good()) {
		std::getline( in, header );
		if (header.length() >4 && header.substr(0,2)=="<?" && 
				header.substr(header.length()-2,2)=="?>")
			header = header.substr(2,header.length()-4);
		else	throw( std::runtime_error("header of XML file was illegal")); 
	}
	return header;//possibly empty, no error
}
// read the following xml label, discarding any whitespace and comments found.
// Prerequisite: the parser has read the header entry
std::string XMLParser::getLabel() {
	std::string label;
	bool opened=false;
	bool closed=false;
	char c;
	while( moreToGo()  && ( !opened || !closed ) ) {
		c=in.get();
		switch(c){
			case '<': if (opened) 
					  throw(std::runtime_error("Label is malformed"));
				  if (moreToGo() && in.peek()=='!') {
					  absorbComment();
				  }
				  else { 
					  opened=true;//mark the opening of the label object
				  }
				  break;
			case '>': if (!opened)
					  throw(std::runtime_error("Label ending before it starts"));
				  closed=true;
				  break;
			case '\t':
			case '\n':
			case ' ':
				  if (opened)
					  throw( std::runtime_error("whitespace in label"));
				  break;
			default:
				  if (opened)
					  label+=c;
				  break;
		}
					
	}
	if (!closed) 
		throw (std::runtime_error ("an opened label was never closed") );

	return label;
}
// absorbs a line- or multiline comment, based on delimiter digraphs <! and !>
// Prerequisite: the starting '<' of the comment has already been read and is followed by '!'
// Side effect: moves the next reading point of the parser's stream to one-past then ending '>'
void XMLParser::absorbComment() {
	//to be called, a '<' has been consumed, and a ! has been peeked.
	in.get();// consuming the starting digraph's ! - HACK. comments use <!-- and --> delimiters
	char c,prev=0;
	bool done=false;

	while (moreToGo() && !done) {
		c=in.get();
		if (prev!=0) { // invalid, used to signal the first iteration
			if (c=='>' && prev=='-') {
				done=true;
			}
		}
		prev=c;
	}
	if (!done) {
		throw ( std::runtime_error(" malformed comment"));
	}
}
	
// reads leaf entry data as a string. does not allow for newlines in data.
// Prerequisite: stream reading point at the start of a leaf line of the form <label>Data</label>
std::string XMLParser::getLeafData() {
	std::string data;
	bool done=false;
	char c;

	while ( moreToGo() && !done ) {
		c=in.peek();
		switch(c){
			case '<': done=true; break;

			case '\n':
			case '>': throw( std::runtime_error(data +"["+c+"illegal characters in data")); break;
			
			default : {
					data+=c;
					in.get();//withdraw char from stream
					break;
				  }
		}
	}
	if (!done)
		throw( std::runtime_error("malformed file"));
	return data;
}
		  			  

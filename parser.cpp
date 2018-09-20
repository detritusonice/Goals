// PARSER.CPP
// copyright 2018, Thanasis Karpetis
// implementation of rudimentary xml parser's functions related to Goals 
// Licence
//


#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include "goals.h"

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
		  			  
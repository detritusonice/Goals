// PARSER.CPP
// implementation of rudimentary xml parser's functions related to Goals 
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


#include <algorithm>
#include <string>
#include <iomanip>
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
			case ' ': if (opened)
					  throw( std::runtime_error("whitespace in label"));
				  break;
			default: if (opened)
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
			
			default : data+=c;
				  in.get();//withdraw char from stream
				  break;
		}
	}
	if (!done)
		throw( std::runtime_error("malformed file"));
	return data;
}
		  			  
//writes header string

void XMLWriter::writeHeader() {
	out<<"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
}

//writes openlabel, push label in label stack, increment indent level, optionally start new line

void XMLWriter::openLabel(const std::string& label, bool newline) {
	indent();
	out<<'<'<<label<<'>';
	labelStack.emplace_back(label,newline); // will store node elements having a newline, leafs without
	if (newline)
		out<<'\n';
	indentLevel++;
}

//write closelabel, pop label from label stack,reduce indent level, start a new line

void XMLWriter::closeLabel() {
	if (labelStack.empty())
		throw ( std::runtime_error("Request to close an unopened label"));
	indentLevel--;
	if (labelStack.back().second) // a node element
		indent();
	out<<"</"<<labelStack.back().first<<">\n";
	labelStack.pop_back();
}

//write a whole goal entry

void XMLWriter::writeGoal( const Goal& goal) {
	openLabel("goal",true);
	openLabel("name"); out<<goal.name; closeLabel();
	openLabel("priority"); out<<goal.priority; closeLabel();
	openLabel("completion"); out<<goal.completion; closeLabel();
	openLabel("unitcost"); out<<std::setprecision(2); out<<goal.unitcost; closeLabel();
	closeLabel();// goal
}


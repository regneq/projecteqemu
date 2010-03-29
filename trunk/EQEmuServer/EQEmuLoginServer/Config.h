#ifndef EQEMU_CONFIG_H
#define EQEMU_CONFIG_H

#include <stdio.h>
#include <list>
#include <map>
#include <string>

using namespace std;

/**
 * Keeps track of all the configuration for the application with a small parser.
 * Note: This is not a thread safe class, but only parse writes to variables in the class.
 * Thus making it mostly safe so long as you're careful with where you call Parse()
 */
class Config
{
public:
	Config() { }
	~Config() { }

	/**
	 * Parses the selected file for variables, will clear current variables if selected.
	 */
	virtual void Parse(const char *file_name);
	
	/**
	 * Gets a variable if it exists.
	 */
	string GetVariable(string title, string parameter);

protected:
	map<string, map<string, string> > vars;

private:
	/**
	 * Breaks our input up into tokens for Parse().
	 * This is private because it's not intended to be overloaded by a derived class which 
	 * may get it's input from other places than a C file pointer. (a http get request for example).
	 * The programmer of a derived class would be expected to make their own Tokenize function for their own Parse().
	 */
	void Tokenize(FILE* input, list<string> &tokens);
};

#endif


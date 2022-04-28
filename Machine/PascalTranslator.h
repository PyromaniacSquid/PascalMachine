#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <set>
#include <stdexcept>
#include <regex>
using namespace std;

class PascalTranslator {
private:
	// Contains each word/symbol in current Pascal script
	vector<string> WordContainer;
	// Contains Line Index of each word/symbol in current Pascal script
	vector<int> IndexContainer;
	// Contains Status of each word/symbol in script (0 - non-reserved/ 1 - reserved)
	vector<bool> ReservedState;
	// Set of all reserved words
	set<string> ReservedWords;

	// Script file stream
	ifstream file;
	
	// Extra memory wasted, no time wasted;
	int ContainerLength;
	// Index of current word
	int ContainerIndex;
	// Index of script line for exceptions
	int LineIndex;
public:
	// Initialize a Translator;
	PascalTranslator();
	// Initialize a Translator and load .pas script with path "path"
	PascalTranslator(string path);
	// Destroy the Translator and unload file
	~PascalTranslator();
	
	// Script input
	
	// Loads .pas file with path "path"
	bool Load(string path);
	// Unload .pas file and clear containers
	void Unload();

	// Load functions:
	
	// Checks if char is one of the following symbols: ':', ';', '.', ',', '='
	bool is_reserved_delimiter(char c);
	// Checks if char is arithmetic operator '*', '/', '+', '-'
	bool is_arithmetic_operator(char c);
	// Checks if char is boolean operator/part of operator '>', '<', '=', '!'
	bool is_boolean_operator(char c);
	// Checks if char is whitespace/newline symbol
	bool is_text_delimiter(char c);
	// Check if word is reserved
	bool is_reserved_word(string word);

	// Access to data
	
	// Adds a new word to WordContainer, IndexContainer, ReservedState;
	bool AddWord(string word, int l_index, bool r_state);
	
	// Returns currently observed word in container;
	string CurrentWord();
	// Returns next word in container
	string NextWord();
	// Returns previous word in container
	string PreviousWord();

	// Increments current index, returns false if impossible
	bool ToNext();
	// Decrements current index, returns false if impossible
	bool ToPrevious();

	// Returns total word count - container length
	int WordCount();
	// Returns Line where current word belongs
	int WordLine();


	// Machine logic

	// Starts translation process, returns true if script is correct
	bool Translate();

	// Checks Program word and goes deeper(+)
	bool check_Program_Keyword();

	// Checks Name Block
	bool check_Name();

	// Var block
	bool check_var();

	// Varibales desc block
	bool check_variables_desc();

	// Varibales list block
	bool check_variables_list();

	// Procedure list block
	bool check_procedure();

	// Operations block
	bool check_op_part();

	// Operations block
	bool check_op_part_layer2();

	// Operations block
	bool check_op_part_layer3();

	// Operators block
	bool check_operators();

	// Read operator
	bool operator_read();

	// Write operator
	bool operator_write();

	// Assign operator
	bool operator_assign();

	// If operator
	bool operator_if();

	// while-do operator
	bool operator_while_do();

	// expression
	bool expression();

	// assign_expr
	bool assign_expr();

	// term
	bool term();

	//factor
	bool factor();

	// number
	bool number();

	//paranthesis sequence
	bool paranthesis_sequence();

	//bool expr
	bool bool_expr();

	//boolean expression
	bool bool_expression();

	//comparison
	bool comparison();

	// bool term
	bool bool_term();

	// bool factor
	bool bool_factor();

	// call proc
	bool call_proc();

	// Bool operator check
	bool check_bool_operator_str(string str);

	// Bool paranthesis sequence check
	bool paranthesis_sequence_bool();

	// Debug Purposes
	
	// Prints all found words, lines and prioritites
	void Print();
};
#include "PascalTranslator.h"

// Initialize a Translator;
PascalTranslator::PascalTranslator() {
	WordContainer = {};
	IndexContainer = {};
	ReservedState = {};
	ContainerLength = 0;  ContainerIndex = 0; LineIndex = 1;
	ReservedWords = { "Program", "program", "Integer", "integer", "Boolean", "boolean", "var", "begin", "end", "write", "read", "if", "while", "do", "procedure",
		"True", "true", "False", "false","while", "do", "read", "write", "if" };
};

// Initialize a Translator and load .pas script with path "path"
PascalTranslator::PascalTranslator(string path) {
	WordContainer = {};
	IndexContainer = {};
	ReservedState = {};
	ContainerLength = 0;  ContainerIndex = 0; LineIndex = 1;
	ReservedWords = { "Program", "program", "Integer", "integer", "Boolean", "boolean", "var", "begin", "end", "write", "read", "if", "while", "do", "procedure",
		"True", "true", "False", "false", "while", "do", "read", "write", "if"};
	Load(path);
}


// Destroy the Translator and unload file
PascalTranslator::~PascalTranslator() {
	Unload();
}

// Checks if char is one of the following symbols: ':', ';', '.', ','
bool PascalTranslator::is_reserved_delimiter(char c) {
	return c == ':'
		|| c == ';'
		|| c == '.'
		|| c == ','
		|| c == '('
		|| c == ')'
		;
}
// Checks if char is arithmetic operator '*', '/', '+', '-'
bool PascalTranslator::is_arithmetic_operator(char c) {
	return c == '*'
		|| c == '/'
		|| c == '+'
		|| c == '-'
		;
}
// Checks if char is boolean operator/part of operator '>', '<', '='
bool PascalTranslator::is_boolean_operator(char c) {
	return c == '>'
		|| c == '<'
		|| c == '='
		;
}
// Checks if char is whitespace/newline symbol
bool PascalTranslator::is_text_delimiter(char c) {
	return c == ' '
		|| c == '\n'
		;
}

// Check if word is reserved
bool PascalTranslator::is_reserved_word(string word) {
	return ReservedWords.find(word) != ReservedWords.end();
}

// Script input

// Loads .pas file with path "path"
bool PascalTranslator::Load(string path) {
	// Open script
	file.open(path, fstream::in);
	
	// Extract words if opened successfully
	if (file.is_open()) {
		// Buffer for a new word
		string str_buffer = "";
		char ch_buffer;
		// Get one char at a time to check for operators
		while (file.get(ch_buffer)) {
			// Most common case is gonna be alphanumerical symbols:
			// We could filter words [i.e. 123abc (incorrect)/ abc123 (correct)] but we'll leave it to the machine
			if (isalpha(ch_buffer) || ch_buffer == '_' || isdigit(ch_buffer)) {
				str_buffer += string(1,ch_buffer);
			}
			// In case of delimiters we need to check for assignment operator to save work for later
			else if (is_reserved_delimiter(ch_buffer)) {
				// Save currently read word (if it's not empty) 
				if (!str_buffer.empty()) {
					AddWord(str_buffer, LineIndex, is_reserved_word(str_buffer));
					str_buffer.clear();
				}
				// Jump right into assignment scenario:
				if (ch_buffer == ':') {
					if (file.peek() != EOF) {
						char extra_symbol_buf = char(file.peek());
						// Actually let's just check symbols explicilty without calling a function
						if (extra_symbol_buf == '=') {
							// now we actually call get to get rid of symbol
							file.get(extra_symbol_buf);
							AddWord(string(1, ch_buffer) + string(1, extra_symbol_buf), LineIndex, 1);
						}
						else
							AddWord(string(1, ch_buffer), LineIndex, 1);
					}
				}
				// In other cases like '.', ',', ';' just add it to container
				else
					AddWord(string(1, ch_buffer), LineIndex, 1);
			}
			// Text delimiters reset a word and proceed
			else if (is_text_delimiter(ch_buffer)) {
				// Save currently read word (if it's not empty) 
				if (!str_buffer.empty()) {
					AddWord(str_buffer, LineIndex, is_reserved_word(str_buffer));
					str_buffer.clear();
				}
				// If faced new line, remember that
				if (ch_buffer == '\n') LineIndex++;
			}
			// Arithmetic operators can only come in one symbol
			else if (is_arithmetic_operator(ch_buffer)) {
				// Save currently read word (if it's not empty) 
				if (!str_buffer.empty()) {
					AddWord(str_buffer, LineIndex, is_reserved_word(str_buffer));
					str_buffer.clear();
				}
				// Also Save new word which is our operator
				AddWord(string(1, ch_buffer), LineIndex, 1);
			}
			// Boolean operators, however, can be 1-2 symbols, so check that immediately not to mix things up
			else if (is_boolean_operator(ch_buffer)) {
				// Save our current word (if it's not empty)
				if (!str_buffer.empty()) {
					AddWord(str_buffer, LineIndex, is_reserved_word(str_buffer));
					str_buffer.clear();
				}
				// '>', '<' can be followed by '=' or form a '<>' as in "not equal"
				// So in cases of '<' or '>' we must check explicitly
				// In case of '=' just go on, nothing to see here
				switch (ch_buffer) {
					// Can be both '=' and '>' so just recheck next char for boolean operator
				case '<':
					// Extra eof check just to be sure 
					if (file.peek() != EOF) {
						char extra_symbol_buf = char(file.peek());
						// Actually let's just check symbols explicilty without calling a function
						if (extra_symbol_buf == '>'
							|| extra_symbol_buf == '=') {
							// now we actually call get to get rid of symbol
							file.get(extra_symbol_buf);
							AddWord(string(1, ch_buffer) + string(1, extra_symbol_buf), LineIndex, 1);
						}
						else
							AddWord(string(1, ch_buffer), LineIndex, 1);
					}
					// Throw an exception if we see EOF after an operator - no doubts
					else {
						string err_msg = "Error in line " + to_string(LineIndex) + ": script unfinished";
						throw std::runtime_error(err_msg);
					}
					break;
					// This one can only be followed by '=', actions are the same
				case '>':
					// Extra eof check just to be sure 
					if (file.peek() != EOF) {
						char extra_symbol_buf = char(file.peek());
						// Check for '=' explicilty
						if (extra_symbol_buf == '=') {
							// now we actually call get to get rid of symbol
							file.get(extra_symbol_buf);
							AddWord(string(1, ch_buffer) + string(1, extra_symbol_buf), LineIndex, 1);
						}
						else
							AddWord(string(1, ch_buffer), LineIndex, 1);
					}
					// Throw an exception if we see EOF after an operator - no doubts
					else {
						string err_msg = "Error in line " + to_string(LineIndex) + ": script unfinished";
						throw std::runtime_error(err_msg);
					}
					break;
					// This one is boring
				case '=':
					AddWord(string(1, ch_buffer), LineIndex, 1);
					break;
					// Unreachable
				default:
					break;
				}
			}
			// And i guess all other symbols will be unrecognizable
			else throw std::runtime_error("Unknow symbol " + string(1,ch_buffer) + " in line " + to_string(LineIndex));
		}
		if (!str_buffer.empty()) {
			AddWord(str_buffer, LineIndex, is_reserved_word(str_buffer));
			str_buffer.clear();
		}
	}
	else throw exception("File cannot be opened");
	file.close();
	return 0;
}

// Unload .pas file and clear containers
void PascalTranslator::Unload() {
	WordContainer.clear();
	IndexContainer.clear();
	ReservedState.clear();
	if (file.is_open()) file.close();
}

// Access to data

// Adds a new word to WordContainer, IndexContainer, ReservedState;
bool PascalTranslator::AddWord(string word, int l_index, bool r_state) {
	WordContainer.push_back(word);
	IndexContainer.push_back(l_index);
	// We know for sure that this is an operator, so it is technically reserved
	ReservedState.push_back(r_state);
	ContainerLength++;
	return 0;
}

// Returns currently observed word in container;
string PascalTranslator::CurrentWord() {
	return WordContainer[ContainerIndex];
}
// Returns next word in container
string PascalTranslator::NextWord() {
	if (ContainerIndex < ContainerLength - 1) return WordContainer[ContainerIndex + 1];
	else return "";
}
// Returns previous word in container
string PascalTranslator::PreviousWord() {
	if (ContainerIndex > 0) return WordContainer[ContainerIndex - 1];
	else return "";
}

// Increments current index, returns false if impossible
bool PascalTranslator::ToNext() {
	if (ContainerIndex < ContainerLength - 1) ContainerIndex++;
	else return 0;
	return 1;
}
// Decrements current index, returns false if impossible
bool PascalTranslator::ToPrevious() {
	if (ContainerIndex > 0) ContainerIndex--;
	else return 0;
	return 1;
}

// Returns total word count - container length
int PascalTranslator::WordCount() {
	return ContainerLength;
}

// Returns Line where current word belongs
int PascalTranslator::WordLine() {
	if (!WordContainer.empty()) return IndexContainer[ContainerIndex];
	else return -1;
}

// Machine logic

// Starts translation process, returns true if script is correct
bool PascalTranslator::Translate() {
	// Check if we have anything in read script
	if (ContainerLength == 0) return false;

	// Set ContainerIndex to 0 just in case
	ContainerIndex = 0;
	
	// Check Program word:
	if (!check_Program_Keyword()) return false;

	// If we got here, we're all good
	return true;
}

// Checks Program word and goes deeper
bool PascalTranslator::check_Program_Keyword() {
	string word = CurrentWord();
	// Technically casing doesn't matter it can be PrOgRaM, but we won't do that
	if (word != "program"
		&& word != "Program") {
		cout << "Error on line " + to_string(WordLine()) + ": keyword Program expected" << endl;
		return false;
	}
	else {
		// Jump to next word, if unsuccessful, raise error message
		if (ToNext()) {
			// If everything is right, we get a program name
			if (check_Name()) {
				// Program name MUST be followed by ';'
				if (NextWord() == ";")
					ToNext();
				// In any other case, raise error
				else {
					cout << "Error on line " + to_string(WordLine()) + ": ; expected" << endl;
					return false;
				}
				// We still haven't returned in case of ';'
				// Look at next word (after ';') to see where to go
				if (ToNext()) {
					if (CurrentWord() == "begin") {
						// There can be an empty body, check that as well
						if (NextWord() == "end") {
							ToNext();
							if (ToNext()) {
								if (CurrentWord() == ".") return true;
								else {
									cout << "Error on line " + to_string(WordLine()) + ": '.' expected" << endl;
									return false;
								}
							}
							else {
								cout << "Error on line " + to_string(WordLine()) + ": '.' expected" << endl;
								return false;
							}
						}
						// Body block:
						else {
							//..
						}
					}
					else if (CurrentWord() == "var") {
						//ToNext();
						check_var();
						//...
					}
					else if (CurrentWord() == "procedure") {
						ToNext();
						return check_procedure();
					}
				}
				else {
					cout << "Error on line " + to_string(WordLine()) + ": program body/variable field/procedure field expected" << endl;
					return false;
				}
			}
		}
		else {
			cout << "Error on line " + to_string(WordLine()) + ": program name expected" << endl;
			return false;
		} 
		
	}
}

// Name block
bool PascalTranslator::check_Name() {
	// Check name with regex
	if (regex_match(CurrentWord(), regex("[a-zA-Z_][a-zA-Z_0-9]*"))
		&& !ReservedState[ContainerIndex])
		return true;
	else {
		cout << "Error on line " + to_string(WordLine()) + ": incorrect identifier";
		return false;
	}
}


// Var block
bool PascalTranslator::check_var() {
	// Check for variable description blocks until we leave var block
	if (check_variables_desc()) {
		// Keyword defines following steps:
		if (ToNext()) {
			if (CurrentWord() == "var") {
				return check_var();
			}
			else if (CurrentWord() == "begin") {
				return false;
			}
			else if (CurrentWord() == "procedure") {
				return check_procedure();
				//return false;
			}
		}
		else {
			cout << "Error on line " << to_string(LineIndex) << ": program body, variable/procedure declaration expected" << endl;
			return false;
		}
	}
}

// Varibales desc block
bool PascalTranslator::check_variables_desc() {
	// Get into var list
	if (check_variables_list()) {
		if (CurrentWord() == ":") {
			if (ToNext()) {
				if (CurrentWord() == "Integer"
					|| CurrentWord() == "integer"
					|| CurrentWord() == "Boolean"
					|| CurrentWord() == "boolean")
				{
					// Expect ";"
					if (ToNext()) {
						if (CurrentWord() == ";") {
							return true;
						}
						else {
							cout << "Error on line " << to_string(LineIndex) << ": ';' expected" << endl;
							return false;
						}
					}
					else {
						cout << "Error on line " << to_string(LineIndex) << ": ';' expected, but variable description is unfinished" << endl;
						return false;
					}
				}
				else {
					cout << "Error on line " << to_string(LineIndex) << ": unknown type " << CurrentWord() << endl;
					return false;
				}
			}
			else {
				cout << "Error on line " << to_string(LineIndex) << ": variable type expected (Integer/Boolean)" << endl;
				return false;
			}
		}
		else {
			cout << "Error on line " << to_string(LineIndex) << ": ':' or ',' expected" << endl;
			return false;
		}
	}
		
}

// Varibales list block
bool PascalTranslator::check_variables_list() {
	if (NextWord() != "") {
		ToNext();
		// identifier is required after var
		if (check_Name()) {
			// it can be followed by ',' and list
			if (ToNext()) {
				// start loop until no more ',' symbols
				while (CurrentWord() == ",") {
					// Go to next name
					if (ToNext()) {
						if (check_Name()) {
							// Go to next "," (next iteration)
							if (!ToNext()) {
								cout << "Error on line " << to_string(LineIndex) << ": expected ',' but block is unfinished" << endl;
								return false;
							}
						}
						else return false;
					}
					else {
						cout << "Error on line " << to_string(LineIndex) << ": expected identifier, but variable block unfinished" << endl;
						return false;
					}
				}
				// If loop is finished or there was no loop at all
				// consider var list finished
				return true;
			}
			// As usual, if it's followed by nothing, raise error
			else {
				cout << "Error on line " << to_string(LineIndex) << ": expected ',' or end of variable block" << endl;
				return false;
			}
		}
		else return false;
	}
	else {
		cout << "Error on line " << to_string(LineIndex) << ": expected identifier" << endl;
		return false;
	}
}

// Procedure list block
bool PascalTranslator::check_procedure() {
	if (NextWord() != "") {
		ToNext();
		// First word after procedure is identifier
		if (check_Name()) {
			// Followed by '('
			if (NextWord() == "(") {
				ToNext();
				// Either ')' or variable list
				// First check then enter loop
				if (NextWord() == "") {
					cout << "Error on line " << to_string(LineIndex) << ": ')' or variable list expected" << endl;
					return false;
				}
				while (NextWord() != ")") {
					if (check_variables_list()) {
						// has to be followed by ':'
						if (CurrentWord() == ":") {
							if (ToNext()) {
								if (CurrentWord() == "integer"
									|| CurrentWord() == "Integer"
									|| CurrentWord() == "Boolean"
									|| CurrentWord() == "boolean") {
									// If it's ';' - skip it, else while condition checks it
									if (NextWord() == ";") ToNext();
								}
								else {
									cout << "Error on line " << to_string(LineIndex) << ": variable type expected (Integer/Boolean)" << endl;
									return false;
								}
							}
							else {
								cout << "Error on line " << to_string(LineIndex) << ": unfinished script" << endl;
								return false;
							}
						}
						else {
							cout << "Error on line " << to_string(LineIndex) << ": ':' expected" << endl;
							return false;
						}
					}
					else return false;
				}
				//Skip ')'
				ToNext();
				// ')' is followed by ';'
				if (ToNext()) {
					if (CurrentWord() == ";") {
						ToNext();
						while (CurrentWord() == "var") {
							check_variables_desc();
							ToNext();
						}
						if (CurrentWord() == "begin") {
							// op part;
						}
						else {
							cout << "Error on line " << to_string(LineIndex) << ": var/begin expected" << endl;
							return false;
						}
					}
					else {
						cout << "Error on line " << to_string(LineIndex) << ": ';' expected" << endl;
						return false;
					}
				}
				else {
					cout << "Error on line " << to_string(LineIndex) << ": ';' expected, but script ended" << endl;
					return false;
				} 
			}
			else {
				cout << "Error on line " << to_string(LineIndex) << ": expected '('" << endl;
				return false;
			}
		}
		else {
			cout << "Error on line " << to_string(LineIndex) << ": procedure identifier expected" << endl;
			return false;
		}
	}
	else {
		cout << "Error on line " << to_string(LineIndex) << ": procedure identifier expected" << endl;
		return false;
	}
}


// Operations block
bool PascalTranslator::check_op_part() {
	// Current word is begin
	ToNext();
	while (CurrentWord() != "end") {
		if (CurrentWord() == "begin") {
			// Get in next layer and check if it succeeded
			if (!check_op_part_layer2()) {
				return false;
			}
			// Current word is end
			ToNext(); // ';'
			if (CurrentWord() != ";") {
				cout << "Error on line " << to_string(LineIndex) << ": ';' expected" << endl;
				return false;
			}
			ToNext(); // whatever else, may be end
		}
		// Normal operations
		if (!check_operators()) return false;
	}
	return true;
}

// Operations block
bool PascalTranslator::check_op_part_layer2() {
	// Current word is begin
	ToNext();
	while (CurrentWord() != "end") {
		if (CurrentWord() == "begin") {
			// Get in next layer and check if it succeeded
			if (!check_op_part_layer3()) {
				return false;
			}
			// Current word is end
			ToNext(); // ';'
			if (CurrentWord() != ";") {
				cout << "Error on line " << to_string(LineIndex) << ": ';' expected" << endl;
				return false;
			}
			ToNext(); // whatever else, may be end
		}
		// Normal operations
		if (!check_operators()) return false;
	}
	return true;
}

// Operations block
bool PascalTranslator::check_op_part_layer3() {
	// Current word is begin
	ToNext();
	while (CurrentWord() != "end") {
		// Normal operations
		if (!check_operators()) return false;
	}
	return true;
}

// Operators block
bool PascalTranslator::check_operators() {
	// Current word is one of operators
	if (CurrentWord() == "read") {
		return operator_read();
	}
	else if (CurrentWord() == "write") {
		return operator_write();
	}
	else if (CurrentWord() == "if") {
		return operator_if();
	}
	else if (CurrentWord() == "while") {
		return operator_while_do();
	}
	else if (check_Name()) {
		return operator_assign();
	}
}

// Read operator
bool PascalTranslator::operator_read() {
	// Current word is read
	ToNext();
	if (CurrentWord() == "(") {
		ToNext();
		if (CurrentWord() == ")") return true;
		// Parameter is single identifier
		if (!check_Name()) {
			cout << "Error on line " << to_string(LineIndex) << ": identifier expected" << endl;
			return false;
		}
		else {
			ToNext();
			if (CurrentWord() == ")") return true;
			else {
				cout << "Error on line " << to_string(LineIndex) << ": ')' expected" << endl;
				return false;
			}
		}
	}
	else {
		cout << "Error on line " << to_string(LineIndex) << ": '(' expected" << endl;
		return false;
	}
}

// Write operator
bool PascalTranslator::operator_write() {
	// Current word is write
	return false;
}

// Assign operator
bool PascalTranslator::operator_assign() {
	// Current word is <name>
	return false;
}

// If operator
bool PascalTranslator::operator_if() {
	// Current word is if
	return false;
}

// while-do operator
bool PascalTranslator::operator_while_do(){
	// Current word is while
	return false;
}



// Debug Purposes

// Prints all found words, lines and prioritites
void PascalTranslator::Print() {
	cout << "-------------------------------------------------------------" << endl;
	for (int i = 0; i < ContainerLength; i++) {
		cout << "Word: " << WordContainer[i] << " Line: " << IndexContainer[i] << " Reserved? " << ReservedState[i] << endl;
	}
	cout << "-------------------------------------------------------------" << endl;
}
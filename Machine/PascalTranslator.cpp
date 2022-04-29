#include "PascalTranslator.h"

// Initialize a Translator;
PascalTranslator::PascalTranslator() {
	WordContainer = {};
	IndexContainer = {};
	ReservedState = {};
	ContainerLength = 0;  ContainerIndex = 0; LineIndex = 1;
	ReservedWords = { "Program", "program", "Integer", "integer", "Boolean", "boolean", "var", "begin", "end", "write", "read", "if", "while", "do", "procedure",
		"True", "true", "False", "false","while", "do", "read", "write", "if", "not", "or", "and" };
};

// Initialize a Translator and load .pas script with path "path"
PascalTranslator::PascalTranslator(string path) {
	WordContainer = {};
	IndexContainer = {};
	ReservedState = {};
	ContainerLength = 0;  ContainerIndex = 0; LineIndex = 1;
	ReservedWords = { "Program", "program", "Integer", "integer", "Boolean", "boolean", "var", "begin", "end", "write", "read", "if", "while", "do", "procedure",
		"True", "true", "False", "false", "while", "do", "read", "write", "if", "not", "or", "and" };
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
		|| c == '\t'
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
				str_buffer += string(1,tolower(ch_buffer));
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
						string err_msg = "Error in line " + to_string(WordLine()) + ": script unfinished";
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
						string err_msg = "Error in line " + to_string(WordLine()) + ": script unfinished";
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
			else throw std::runtime_error("Unknow symbol " + string(1,ch_buffer) + " in line " + to_string(WordLine()));
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
	// Technically casing doesn't matter it can be PrOgRaM, but we won't do that
	if (CurrentWord() != "program") {
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
								if (CurrentWord() == ".") {
									if (ContainerIndex == ContainerLength - 1) {
										return true;
									}
									else {
										cout << "Error on line " << to_string(WordLine()) << ": nothing should follow end of program" << endl;
										return false;
									}
								}
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
							if (!check_op_part()) {
								return false;
							}
							// end -> .
							ToNext();
							if (CurrentWord() == ".") {
								if (ContainerIndex == ContainerLength - 1) return true;
								else {
									cout << "Error on line " + to_string(WordLine()) + ": nothing should follow \'.\'" << endl;
									return false;
								}
							}
							else {
								cout << "Error on line " + to_string(WordLine()) + ": \'.\' expected" << endl;
								return false;
							}
						}
					}
					else if (CurrentWord() == "var") {
						ToNext();
						return check_var();
					}
					else if (CurrentWord() == "procedure") {
						//ToNext();
						// We'll check all even body
						return check_procedure();
					}
				}
				else {
					cout << "Error on line " + to_string(WordLine()) + ": program body/variable field/procedure field expected" << endl;
					return false;
				}
			}
			else {
				cout << "Error on line " + to_string(WordLine()) + ": invalid program name" << endl;
				return false;
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
	else return false;
}


// Var block
bool PascalTranslator::check_var() {
	// Check for variable description blocks until we leave var block
	if (check_variables_desc()) {
	// Keyword defines following steps:
		if (CurrentWord() == "var") {
			ToNext();
			return check_var();
		}
		else if (CurrentWord() == "begin") {
			// There can be an empty body, check that as well
			if (NextWord() == "end") {
				ToNext();
				if (ToNext()) {
					if (CurrentWord() == ".") {
						if (ContainerIndex == ContainerLength - 1) {
							return true;
						}
						else {
							cout << "Error on line " << to_string(WordLine()) << ": nothing should follow end of program" << endl;
							return false;
						}
					}
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
				check_op_part();
				// end -> .
				ToNext();
				if (CurrentWord() == ".") {
					if (ContainerIndex == ContainerLength - 1) return true;
					else {
						cout << "Error on line " + to_string(WordLine()) + ": nothing should follow \'.\'" << endl;
						return false;
					}
				}
				else {
					cout << "Error on line " + to_string(WordLine()) + ": \'.\' expected" << endl;
					return false;
				}
			}
		}
		else if (CurrentWord() == "procedure") {
			return check_procedure();
			//return false;
		}
		
	}
}

// Varibales desc block
bool PascalTranslator::check_variables_desc() {
	// Get into var list
	while (check_Name()) {
		if (check_variables_list()) {
			if (CurrentWord() == ":") {
				if (ToNext()) {
					if ( CurrentWord() == "integer"
						|| CurrentWord() == "boolean")
					{
						
						if (ToNext()) {
							if (CurrentWord() == ";") {
								ToNext();
							}
							else {
								cout << "Error on line " << to_string(WordLine()) << ": ';' expected" << endl;
								return false;
							}
						}
						else {
							cout << "Error on line " << to_string(WordLine()) << ": ';' expected, but variable description is unfinished" << endl;
							return false;
						}
					}
					else {
						cout << "Error on line " << to_string(WordLine()) << ": unknown type " << CurrentWord() << endl;
						return false;
					}
				}
				else {
					cout << "Error on line " << to_string(WordLine()) << ": variable type expected (Integer/Boolean)" << endl;
					return false;
				}
			}
			else {
				cout << "Error on line " << to_string(WordLine()) << ": ':' or ',' expected" << endl;
				return false;
			}
		}
	}
	//Check if we left because a reserved word was used
	if (ReservedState[ContainerIndex] && 
		CurrentWord()!="procedure" &&
		CurrentWord()!="var" &&
		CurrentWord()!="begin") {
		cout << "Error on line " << to_string(WordLine()) << ": \"" << CurrentWord() << "\" cannot be used as identifier"<< endl;
		return false;
	}
	return true;
}

// Varibales list block
bool PascalTranslator::check_variables_list() {
	//if (NextWord() != "") {
	//	ToNext();
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
								cout << "Error on line " << to_string(WordLine()) << ": expected ',' but block is unfinished" << endl;
								return false;
							}
						}
						else {
							cout << "Error on line " << to_string(WordLine()) << ": correct identifier expected" << endl;
							return false;
						}
					}
					else {
						cout << "Error on line " << to_string(WordLine()) << ": expected identifier, but variable block unfinished" << endl;
						return false;
					}
				}
				// If loop is finished or there was no loop at all
				// consider var list finished
				return true;
			}
			// As usual, if it's followed by nothing, raise error
			else {
				cout << "Error on line " << to_string(WordLine()) << ": expected ',' or end of variable block" << endl;
				return false;
			}
		}
		else {
			cout << "Error on line " << to_string(WordLine()) << ": correct identifier expected" << endl;
			return false;
		}
	//}
	/*else {
		cout << "Error on line " << to_string(WordLine()) << ": expected identifier" << endl;
		return false;
	}*/
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
					cout << "Error on line " << to_string(WordLine()) << ": ')' or variable list expected" << endl;
					return false;
				}
				while (NextWord() != ")") {
					ToNext();
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
									cout << "Error on line " << to_string(WordLine()) << ": variable type expected (Integer/Boolean)" << endl;
									return false;
								}
							}
							else {
								cout << "Error on line " << to_string(WordLine()) << ": unfinished script" << endl;
								return false;
							}
						}
						else {
							cout << "Error on line " << to_string(WordLine()) << ": ':' expected" << endl;
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
							ToNext();
							check_variables_desc();
							//ToNext();
						}
						if (CurrentWord() == "begin") {
							check_op_part();
							// end -> ;
							ToNext();
							if (CurrentWord() == ";") {
								ToNext();
								// S18->S4
								if (CurrentWord() == "var") {
									ToNext();
									return check_var();
								}
								if (CurrentWord() == "procedure") {
									return check_procedure();
								}
								// S18->S19
								if (CurrentWord() == "begin") {
									check_op_part();
									// end -> .
									ToNext();
									if (CurrentWord() == ".") {
										// If it's the last word, then it's ok
										if (ContainerIndex == ContainerLength - 1) {
											return true;
										}
										else {
											cout << "Error on line " << to_string(WordLine()) << ": nothing should follow end of program" << endl;
											return false;
										}
									}
								}
								else {
									cout << "Error on line " << to_string(WordLine()) << ": code block expected" << endl;
									return false;
								}
							}
							else {
								cout << "Error on line " << to_string(WordLine()) << ": ';' expected" << endl;
								return false;
							}
						}
						else {
							cout << "Error on line " << to_string(WordLine()) << ": var/begin expected" << endl;
							return false;
						}
					}
					else {
						cout << "Error on line " << to_string(WordLine()) << ": ';' expected" << endl;
						return false;
					}
				}
				else {
					cout << "Error on line " << to_string(WordLine()) << ": ';' expected, but script ended" << endl;
					return false;
				} 
			}
			else {
				cout << "Error on line " << to_string(WordLine()) << ": expected '('" << endl;
				return false;
			}
		}
		else {
			cout << "Error on line " << to_string(WordLine()) <<  ": \"" << CurrentWord() << "\" can't be used as procedure identifier" << endl;
			return false;
		}
	}
	else {
		cout << "Error on line " << to_string(WordLine()) << ": procedure identifier expected" << endl;
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
			if (!check_op_part()) {
				return false;
			}
			// Current word is end
			ToNext(); // ';'
			if (CurrentWord() != ";") {
				cout << "Error on line " << to_string(WordLine()) << ": ';' expected" << endl;
				return false;
			}
			ToNext(); // whatever else, may be end
		}
		// Normal operations
		else if (!check_operators()) return false;
		//else ToNext();
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
				cout << "Error on line " << to_string(WordLine()) << ": ';' expected" << endl;
				return false;
			}
			ToNext(); // whatever else, may be end
		}
		// Normal operations
		if (!check_operators()) return false;
		//else ToNext();
	}
	return true;
}

// Operations block
bool PascalTranslator::check_op_part_layer3() {
	// Current word is begin
	ToNext();
	while (CurrentWord() != "end") {
		// Normal operations
		if (CurrentWord() == "begin") {
			cout << "Error on line " + to_string(WordLine()) + ": no more begin-end blocks allowed" << endl;
			return false;
		}
		if (!check_operators()) return false;
		//else ToNext();
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
		if (!call_proc())
			return operator_assign();
		else return true;
	}
	else {
		cout << "Error on line " << to_string(WordLine()) << ": unknown operation or incorrect identifier" << endl;
		return false;
	}
}

// Read operator
bool PascalTranslator::operator_read() {
	// Current word is read
	ToNext();
	if (CurrentWord() == "(") {
		ToNext();
		if (CurrentWord() == ")") {
			ToNext();
			if (CurrentWord() == ";") return true;
			else {
				cout << "Error on line " << to_string(WordLine()) << ": ';' expected" << endl;
				return false;
			}
		}
		// Parameter is single identifier
		if (!check_Name()) {
			cout << "Error on line " << to_string(WordLine()) << ": correct identifier expected" << endl;
			return false;
		}
		else {
			ToNext();
			if (CurrentWord() == ")") {
				ToNext();
				if (CurrentWord() == ";") ToNext();
				else {
					cout << "Error on line " << to_string(WordLine()) << ": ';' expected" << endl;
					return false;
				}
				return true;
			}
			else {
				cout << "Error on line " << to_string(WordLine()) << ": ')' expected" << endl;
				return false;
			}
		}
	}
	else {
		cout << "Error on line " << to_string(WordLine()) << ": '(' expected" << endl;
		return false;
	}
}

// Write operator
bool PascalTranslator::operator_write() {
	// Current word is write
	ToNext();
	if (CurrentWord() == "(") {
		ToNext();
		if (CurrentWord() == ")") {
			ToNext();
			if (CurrentWord() == ";") return true;
			else {
				cout << "Error on line " << to_string(WordLine()) << ": ';' expected" << endl;
				return false;
			}
		}
		// Parameter is single identifier
		if (!expression()) {
			cout << "Error on line " << to_string(WordLine()) << ": expression expected" << endl;
			return false;
		}
		else {
			ToNext();
			if (CurrentWord() == ")") {
				ToNext();
				if (CurrentWord() == ";") ToNext();
				else {
					cout << "Error on line " << to_string(WordLine()) << ": ';' expected" << endl;
					return false;
				}
				return true;
			}
			else {
				cout << "Error on line " << to_string(WordLine()) << ": ')' expected" << endl;
				return false;
			}
		}
	}
	else {
		cout << "Error on line " << to_string(WordLine()) << ": '(' expected" << endl;
		return false;
	}
}

// Assign operator
bool PascalTranslator::operator_assign() {
	if (NextWord() != ":=") return false;
	else {
		// Current word is <name>
		ToNext();
		// Expected :=
		if (CurrentWord() == ":=") {
			ToNext();
			// Temp
			if (assign_expr()) {
				// last word -> ;
				if (ToNext()) {
					if (CurrentWord() == ";") ToNext();
					else {
						cout << "Error on line " << to_string(WordLine()) << ": \';\' expected" << endl;
						return false;
					}
					return true;
				}
				else {
					cout << "Error on line " << to_string(WordLine()) << ": \';\' expected" << endl;
					return false;
				}
			}
			else {
				cout << "Error on line " << to_string(WordLine()) << ": identifier or expression expected" << endl;
				return false;
			}
		}
		else {
			cout << "Error on line " << to_string(WordLine()) << ": ':=' expected" << endl;
			return false;
		}
		return false;
	}
}

// If operator
bool PascalTranslator::operator_if() {
	// Current word is if
	if (!ToNext()) {
		cout << "Error on line " << to_string(WordLine()) << ": boolean expression expected" << endl;
		return false;
	}
	if (!bool_expression()) {
		cout << "Error on line " << to_string(WordLine()) << ": incorrect boolean expression" << endl;
		return false;
	}
	else {
		ToNext();
		if (CurrentWord() == "then") {
			if (ToNext()) {
				if (CurrentWord() == "begin") {
					if (check_op_part()) {
						if (ToNext()) {
							if (CurrentWord() == ";") ToNext();
							else {
								cout << "Error on line " << to_string(WordLine()) << ": \';\' expected" << endl;
								return false;
							}
							return true;
						}
						else {
							cout << "Error on line " << to_string(WordLine()) << ": \';\' expected" << endl;
							return false;
						}
					}
					else {
						cout << "Error on line " << to_string(WordLine()) << ": incorrect if body" << endl;
						return false;
					}
				}
				else {
					if (!check_operators()) {
						cout << "Error on line " << to_string(WordLine()) << ": invalid operation" << endl;
						return false;
					}
					else return true;
				}
			}
			else{
				cout << "Error on line " << to_string(WordLine()) << ": 'then' expected" << endl;
				return false;
			}
		}
		else {
			cout << "Error on line " << to_string(WordLine()) << ": 'then' expected" << endl;
			return false;
		}
	}
	return false;
}

// while-do operator
bool PascalTranslator::operator_while_do(){
	// Current word is while
	ToNext();
	if (bool_expression()) {
		if (NextWord()=="do") {
			ToNext();
			if (ToNext()) {
				if (CurrentWord() == "begin") {
					if (check_op_part()) {
						if (ToNext()) {
							if (CurrentWord() == ";") ToNext();
							else {
								cout << "Error on line " << to_string(WordLine()) << ": \';\' expected" << endl;
								return false;
							}
							return true;
						}
						else {
							cout << "Error on line " << to_string(WordLine()) << ": \';\' expected" << endl;
							return false;
						}
					}
					else {
						cout << "Error on line " << to_string(WordLine()) << ": incorrect if body" << endl;
						return false;
					}
				}
				else {
					if (!check_operators()) {
						cout << "Error on line " << to_string(WordLine()) << ": invalid operation" << endl;
						return false;
					}
					else return true;
				}
			}
			else {
				cout << "Error on line " << to_string(WordLine()) << ": 'then' expected" << endl;
				return false;
			}
		}
		else {
			cout << "Error on line " << to_string(WordLine()) << ": 'do' expected" << endl;
			return false;
		}
	}
	else {
		cout << "Error on line " << to_string(WordLine()) << ": boolean expression expected" << endl;
		return false;
	}
	return false;
}

// expression
bool PascalTranslator::expression() {
	if (term()) {
		while (NextWord() == "+" || NextWord() == "-") {
			// term -> operator
			ToNext();
			// operator -> possible term
			ToNext();
			if (!term()) return false;
		}
		return true;
	}
	return false;
}

// assign_expr
bool PascalTranslator::assign_expr() {
	// name/number
	// If facing a name - either procedure or expression
	// checking call_proc doesn't move us untill it is a call
	// on "name(..." structure it must be procedure
	//if (call_proc()) return true;
	//else 
	{
		// Extra Case: boolean starting with 'not'
		if (CurrentWord() == "not" 
			|| CurrentWord()=="true"
			|| CurrentWord()=="false") 
			return bool_expression();
		
		// In another case we can meet either expression or bool_expression
		// expression () returns after it hasn't found another arithmetic opeator
		if (expression()) {
			// Current word after expression() is its last operator
			// That lets us check whether the rest is boolean operator + smth, to detect boolean expression
			// But for that we need to make sure that next word is a boolean operator
			// In trivial case we have a variable/number which is expression and a part of bool_expr
			if (check_bool_operator_str(NextWord())||NextWord() == "or" || NextWord() == "and") {
				if (!bool_expression()) {
					cout << "Error on line " << to_string(WordLine()) << ": incorrect boolean expression" << endl;
					return false;
				}
			}
			// Else it was just an arithmetic expression
			return true;
		}
		else {
			if (bool_expression()) return true;
			cout << "Error on line " << to_string(WordLine()) << ": expression/boolean expression/procedure call expected" << endl;
			return false;
		}
		// else we have name or unknown symbol after name, which is incorrect
		return false;
	}
	// returns before ';'
	return false;
}


// term
bool PascalTranslator::term() {
	if (factor()) {
		while (NextWord() == "*" || NextWord() == "/") {
			// factor->operator
			ToNext();
			// operator->possible factor
			ToNext();
			if (!factor()) return false;
		}
		return true;
	}
	else return false;
}

//factor
bool PascalTranslator::factor() {
	if (CurrentWord() == "(") return paranthesis_sequence();
	return (check_Name() || number()) && !call_proc();
	//else return false;
}

// number
bool PascalTranslator::number() {
	if (CurrentWord() == "-") ToNext();
	return (CurrentWord() == "0") ||(regex_match(CurrentWord(), regex("[1-9][0-9]*")));
}

//paranthesis sequence
bool PascalTranslator::paranthesis_sequence() {
	if (CurrentWord() == "(") {
		ToNext();
		if (expression()) {
			if (NextWord() == ")") {
				ToNext();
				return true;
			}
			else {
				cout << "Error on line " << to_string(WordLine()) << ": incorrect paranthesis sequence" << endl;
				exit(1);
				return false;
			}
		}
		else return false;
	}
	else return false;
}


//comparison
bool PascalTranslator::comparison() {
	if (expression() || (CurrentWord() == ")")) {
		if (NextWord() == "") {
			cout << "Error on line " << to_string(WordLine()) << ": boolean operator expected" << endl;
			return false;
		}
		if (check_bool_operator_str(NextWord())) {
			// skip operator
			ToNext();
			// Now it HAS to be an expression
			if (!ToNext()) {
				cout << "Error on line " << to_string(WordLine()) << ": boolean expression expected" << endl;
				return false;
			}
			if (expression()) {
				return true;
			}
			else {
				cout << "Error on line " << to_string(WordLine()) << ": incorrect boolean expression expected" << endl;
				return false;
			}
		}
		// returns last operand of first expression, allowing to check bool_expression
		else return false;
	}
	// -||-
	return false;
}

//boolean expression
bool PascalTranslator::bool_expression() {
	// enters with an <expression>
	if (bool_term()) {
		while (NextWord() == "or") {
			// factor->operator
			ToNext();
			// operator->possible factor
			ToNext();
			if (!bool_term()) return false;
		}
		return true;
	}
	else return false;
}

// bool term
bool PascalTranslator::bool_term() {
	if (bool_factor()) {
		while (NextWord() == "and") {
			// factor->operator
			ToNext();
			// operator->possible factor
			ToNext();
			if (!bool_factor()) return false;
		}
		return true;
	}
	else return false;
}
// bool factor
bool PascalTranslator::bool_factor() {
	if (CurrentWord() == "not") {
		if (!ToNext()) {
			cout << "Error on line " << to_string(WordLine()) << ": boolean expression expected" << endl;
			return false;
		}
		else {
			return bool_factor();
		}
	}
	if (CurrentWord() == "(") {
		ToNext();
		if (!bool_expression()) return false;
		ToNext();
		// Leave paranthesis sequence
		if (CurrentWord() == ")") return true;
		else {
			return false;
		}
	}
	// Else Check for name, comparison or True/False
	return (CurrentWord() == "true"
		|| CurrentWord() == "false"
		// potential bug
		|| comparison() 
		//
		|| check_Name());
}
// call proc
bool PascalTranslator::call_proc() {
	// check CurrentWord for name, then check next word for '('
	if (check_Name() && (NextWord() == "(")) {
		// either arguments or ')'
		// name -> (
		ToNext();
		// Trivial case
		if (NextWord() == ")")
			ToNext();
		while (CurrentWord() != ")") {
			// on first iter '(' -> parameter/)
			if (ToNext()) {
				// parameters can be either of assign_expr
				if (assign_expr()) {
					if (!ToNext()) return false;
				}
				else return false;
			}
			else return false;
		}
		if (ToNext()) {
			if (CurrentWord() == ";") ToNext();
			else {
				cout << "Error on line " << to_string(WordLine()) << ": \';\' expected" << endl;
				return false;
			}
			return true;
		}
		else {
			cout << "Error on line " << to_string(WordLine()) << ": \';\' expected" << endl;
			return false;
		}
	}
	else return false;
}

// Bool operator check
bool PascalTranslator::check_bool_operator_str(string str) {
	return (set<string>({"<", ">", "<=", ">=", "<>", "=" }).count(str) > 0);
}
// Bool paranthesis sequence check
bool PascalTranslator::paranthesis_sequence_bool() {
	// for now it has already got '('
	if (bool_expression()) {
		// Finishes on )
		return ToNext();
	}
	else return false;
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
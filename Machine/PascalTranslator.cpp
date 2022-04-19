#include "PascalTranslator.h"

// Initialize a Translator;
PascalTranslator::PascalTranslator() {
	WordContainer = {};
	IndexContainer = {};
	ReservedState = {};
	ContainerLength = 0;  ContainerIndex = 0; LineIndex = 1;
	ReservedWords = { "Program", "program", "Integer", "integer", "Boolean", "boolean", "var", "begin", "end", "write", "read", "if", "while", "do", "procedure",
		"True", "true", "False", "false" };
};

// Initialize a Translator and load .pas script with path "path"
PascalTranslator::PascalTranslator(string path) {
	WordContainer = {};
	IndexContainer = {};
	ReservedState = {};
	ContainerLength = 0;  ContainerIndex = 0; LineIndex = 1;
	ReservedWords = { "Program", "program", "Integer", "integer", "Boolean", "boolean", "var", "begin", "end", "write", "read", "if", "while", "do", "procedure",
		"True", "true", "False", "false"};
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
	else return 1;
	return 0;
}
// Decrements current index, returns false if impossible
bool PascalTranslator::ToPrevious() {
	if (ContainerIndex > 0) ContainerIndex--;
	else return 1;
	return 0;
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
		|| word != "Program") {
		cout << "Error on line " + to_string(WordLine()) + ": keyword Program expected" << endl;
		return false;
	}
	else {
		// Jump to next word, if unsuccessful, raise error message
		if (ToNext()) return check_Program_Name();
		else {
			cout << "Error on line " + to_string(WordLine()) + ": program name expected" << endl;
			return false;
		} 
	}
}

// Checks Program name word+
bool PascalTranslator::check_Program_Name() {
	if (!ReservedState[ContainerIndex]) {
		// That means Name is OK, gotta check ; symbol
		if (NextWord() == ";")
			ToNext();
		// In any other case, raise error
		else {
			cout << "Error on line " + to_string(WordLine()) + ": ; expected" << endl;
			return false;
		}
		// We still haven't returned in case of ';'
		// Look at next word (after ';') to see where to go
		string next_word = NextWord();
		// Trivial case - no more words
		if (next_word == "") {
			cout << "Error on line " + to_string(WordLine()) + ": program body expected" << endl;
			return false;
		}
		//else if (next_word = "")
	}
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
#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>


namespace {


class preprocess {
public:

	static std::string string(std::string s) {
		std::string out;
		preprocess(s, out);
		return out;
	}

	static std::string file(std::string fileName) {
		return preprocess::string(readFile(fileName));
	}

private:

	static std::string readFile(std::string fileName) {
  	std::ifstream in(fileName, std::ios::in | std::ios::binary);
  	if (in) {
    	std::string contents;
    	in.seekg(0, std::ios::end);
    	contents.resize((size_t)in.tellg());
    	in.seekg(0, std::ios::beg);
    	in.read(&contents[0], (std::streamsize)contents.size());
    	in.close();
			return contents;
  	} else {
			std::cerr << "reading file fail: " << (int)errno << std::endl;
			return "";
		}
	}


	enum class State {
		TEXT,                      // skipping to $
		DOLLAR_COMMAND_NAME,       // skipping to ( or non-alphanumeric
		DOLLAR_COMMAND_PARAMETERS  // skipping to matching )
	};

	const std::string & in;
	std::string & out;
	std::string quotesString;

	State state = State::TEXT;
	int bracketsDepth = 1;
	std::string constantText;
	std::string varOrCommandName;
	std::string varOrCommandParameters;


	preprocess(const std::string & in_, std::string & out_) : in(in_), out(out_) {
		static const char alpha[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
		quotesString.resize(16);
		do {
			for (size_t i=0; i<16; i++) quotesString[i] = alpha[(unsigned)rand() % (sizeof(alpha)-1)];
		} while (in.find(quotesString) != std::string::npos);

		out += fileStart();
		for (auto it = std::begin(in); it != std::end(in); it++) {
			preprocessChar(*it);
		}
		out += text(constantText);
		out += command(varOrCommandName, varOrCommandParameters);
		out += fileEnd();
	}

	void preprocessChar(char c) {
		switch (state) {
			case State::TEXT: preprocessCharInText(c); break;
			case State::DOLLAR_COMMAND_NAME: preprocessCharInDollarCommandName(c); break;
			case State::DOLLAR_COMMAND_PARAMETERS: preprocessCharInDollarCommandParameters(c); break;
		}
	}

	void preprocessCharInText(char c) {
		if (c == '$') {
			setState(State::DOLLAR_COMMAND_NAME);
		} else {
			constantText += c;
		}
	}

	void preprocessCharInDollarCommandName(char c) {
		if (isalnum(c) || (c == '_')) {
			varOrCommandName += c;
		} else {
			if (c == '(') {
				setState(State::DOLLAR_COMMAND_PARAMETERS);
			} else {
				setState(State::TEXT);
				preprocessCharInText(c);
			}
		}
	}

	void preprocessCharInDollarCommandParameters(char c) {
		if (c == '(') {
			bracketsDepth++;
			varOrCommandParameters += c;
		} else {
			if (c == ')')  {
				bracketsDepth--;
				if (bracketsDepth == 0) {
					setState(State::TEXT);
				} else {
					varOrCommandParameters += c;
				}
			} else {
				varOrCommandParameters += c;
			}
		}
	}

	void setState(State newState) {
		State oldState = state;
		if ((oldState == State::TEXT) && ((newState == State::DOLLAR_COMMAND_NAME) || (newState == State::DOLLAR_COMMAND_PARAMETERS))) {
			out += text(constantText);
			constantText.clear();
		}
		if ((newState == State::TEXT) && ((oldState == State::DOLLAR_COMMAND_NAME) || (oldState == State::DOLLAR_COMMAND_PARAMETERS))) {
			out += command(varOrCommandName, varOrCommandParameters);
			bracketsDepth = 1;
			varOrCommandName.clear();
			varOrCommandParameters.clear();
		}
		state = newState;
	}

	std::string fileStart() const {
		return "[&]() {\n"
			"#ifndef __TEMPLATER_HPP__INCLUDED__\n"
			"#error \"You must include common/templater.hpp to use templates\"\n"
			"#endif\n"
			"std::stringstream __res;";
	}
	std::string fileEnd() const { return "return __res.str(); }()"; }

	std::string openQuotes() const { return "R\"" + quotesString + "("; }
	std::string closeQuotes() const { return ")" + quotesString + "\""; }

	std::string appendBegin() const { return "__res << "; }
	std::string appendEnd() const { return ";"; }

	std::string quoted(std::string s) const { return openQuotes() + s + closeQuotes(); }
	std::string text(std::string text) const { return appendBegin() + quoted(text) + appendEnd(); }
	std::string expression(std::string expr) const { return appendBegin() + expr + appendEnd(); }

	std::string end() const { return "}"; }
	std::string include(std::string fileName) const { return readFile(fileName); }
	std::string for_(std::string parameters) const { return "for (" + parameters + ") {"; }
	std::string foreach(std::string parameters) const { return for_("const auto &" + parameters); }
	std::string if_(std::string condition) const { return "if (" + condition + ") {"; }
	std::string else_() const { return "} else {"; }

	std::string command(std::string command, std::string parameters) const {
		if ((command == "") && (parameters == "")) return "";            // так надо

		// simple expression
		if (command == "")        return expression(parameters);         // $(var)
		// commands without parameters
		if (command == "$")       return "$";                            // $$
		if (command == "else")    return else_();                        // $else
		if (command == "end")     return end();                          // $end
		// commands with parameters
		if (parameters == "")     return expression(command);            // $var
		if (command == "import")  return preprocess::file(parameters);   // $import(file.htmlt)
		if (command == "include") return include(parameters);            // $include(file.css)
		if (command == "for")     return for_(parameters);               // $for (int i=0; i<n; i++)
		if (command == "foreach") return foreach(parameters);            // $foreach(item : collection)
		if (command == "if")      return if_(parameters);                // $if (cond)

		throw std::runtime_error("unknown command: $" + command + "(" + parameters + ")");
	}
};


}



int main(int argc, char ** argv) {
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <file.htmlt> <file.htmltc>" << std::endl;
		return 1;
	}

	std::ofstream file(argv[2], std::ios_base::binary);
 	file << preprocess::file(argv[1]);

	return 0;
}

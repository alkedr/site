#include <common/templater.hpp>

class preprocess {
public:

	static std::string string(std::string s) {
		std::string out;
		preprocess(s, out);
		return out;
	}

	static std::string file(std::string fileName) {
  	std::ifstream in(fileName, std::ios::in | std::ios::binary);
  	if (in) {
    	std::string contents;
    	in.seekg(0, std::ios::end);
    	contents.resize((size_t)in.tellg());
    	in.seekg(0, std::ios::beg);
    	in.read(&contents[0], (std::streamsize)contents.size());
    	in.close();
			return preprocess::string(contents);
  	} else {
			std::cerr << "reading file fail: " << (int)errno << std::endl;
			return "";
		}
	}

private:

	enum State {
		TEXT,  // skipping to $ or <
		TAG,   // between < and >, skipping to >
		DOLLAR_COMMAND_NAME,   // $a, $(a), $foreach(...), $end, $include(...)    skipping to (
		DOLLAR_COMMAND_PARAMETERS   // skipping to matching )
	};

	const std::string & in;
	std::string & out;
	std::string quotesString;

	State state = TEXT;
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
			case TEXT: preprocessCharInText(c); break;
			case TAG: preprocessCharInTag(c); break;
			case DOLLAR_COMMAND_NAME: preprocessCharInDollarCommandName(c); break;
			case DOLLAR_COMMAND_PARAMETERS: preprocessCharInDollarCommandParameters(c); break;
		}
	}

	void preprocessCharInText(char c) {
		if (c == '$') {
			setState(DOLLAR_COMMAND_NAME);
		} else {
			if (c == '<') setState(TAG);
			constantText += c;
		}
	}

	void preprocessCharInTag(char c) {
		constantText += c;
		if (c == '>') setState(TEXT);
	}

	void preprocessCharInDollarCommandName(char c) {
		if (isalnum(c) || (c == '_')) {
			varOrCommandName += c;
		} else {
			if (c == '(') {
				setState(DOLLAR_COMMAND_PARAMETERS);
			} else {
				setState(TEXT);
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
					setState(TEXT);
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
		if (((oldState == TEXT               ) || (oldState == TAG                      )) 
		 && ((newState == DOLLAR_COMMAND_NAME) || (newState == DOLLAR_COMMAND_PARAMETERS))) {
			out += text(constantText);
			constantText.clear();
		}
		if (((newState == TEXT               ) || (newState == TAG                      )) 
		 && ((oldState == DOLLAR_COMMAND_NAME) || (oldState == DOLLAR_COMMAND_PARAMETERS))) {
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
			"std::stringstream res;";
	}
	std::string fileEnd() const { return "return res.str(); }()"; }

	std::string openQuotes() const { return "R\"" + quotesString + "("; }
	std::string closeQuotes() const { return ")" + quotesString + "\""; }

	std::string appendBegin() const { return "res << "; }
	std::string appendEnd() const { return ";"; }

	std::string quoted(std::string s) const { return openQuotes() + s + closeQuotes(); }
	std::string text(std::string text) const { return appendBegin() + quoted(text) + appendEnd(); }
	std::string expression(std::string expr) const { return appendBegin() + expr + appendEnd(); }
	std::string foreach(std::string parameters) const { return "for (const auto & " + parameters + ") {"; }
	std::string end() const { return "}"; }

	std::string command(std::string command, std::string parameters) const {
		if (command.empty() && parameters.empty()) return "";
		if (command == "include") return preprocess::file(parameters);   // TODO: don't preprocess if not .htmlt
		if (command == "foreach") return foreach(parameters);
		if (command == "end") return end();
		if (command == "") return expression(parameters);    // $(...)
		if (parameters == "") return expression(command);    // $...
		throw std::runtime_error("unknown command: $" + command + "(" + parameters + ")");
	}
};

int main(int argc, char ** argv) {
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <file.htmlt> <file.htmltc>" << std::endl;
		return 1;
	}

	std::ofstream file(argv[2], std::ios_base::binary);
 	file << preprocess::file(argv[1]);
	file.close();

	return 0;
}

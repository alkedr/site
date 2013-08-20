#include <common/templater.hpp>

#include <common/catch.hpp>


const char * correctAnswer = R"(<html>
<body>
<h1>Hello!</h1>
<h2>This is some text</h2>
<h3>Numbers:</h3>
<ul>
<li>1</li>
<li>2</li>
<li>3</li>
</ul>
<h3>MORE NUMBERS</h3>
<ul>
<li>1.125</li>
<li>2.567</li>
<li>3.874</li>
</ul>
</body>
</html>
)";



TEST_CASE( "preprocess empty" ) {
	std::string res =
#	include <common/tests/templates/empty.htmltc>
	;

	CHECK( res == "\n" );
}


TEST_CASE( "preprocess static" ) {
	std::string res =
#	include <common/tests/templates/static.htmltc>
	;

	CHECK( res == correctAnswer );
}


TEST_CASE( "preprocess simple string variable" ) {
	std::string title = "Hello!";

	std::string res =
#	include <common/tests/templates/one_var.htmltc>
	;

	CHECK( res == correctAnswer );
}


TEST_CASE( "preprocess two string variables without space between them" ) {
	std::string titlePartOne = "Hell";
	std::string titlePartTwo = "o!";

	std::string res =
#	include <common/tests/templates/two_vars.htmltc>
	;

	CHECK( res == correctAnswer );
}


TEST_CASE( "preprocess char as int variable" ) {
	char number1 = 1;

	std::string res =
#	include <common/tests/templates/char_as_int_var.htmltc>
	;

	CHECK( res == correctAnswer );
}


TEST_CASE( "preprocess int variable" ) {
	int number1 = 1;

	std::string res =
#	include <common/tests/templates/int_var.htmltc>
	;

	CHECK( res == correctAnswer );
}


TEST_CASE( "preprocess float variable" ) {
	float number1 = 1.125;

	std::string res =
#	include <common/tests/templates/float_var.htmltc>
	;

	CHECK( res == correctAnswer );
}


TEST_CASE( "preprocess std::array and std::vector" ) {
	std::array<unsigned short, 3> ints = { 1, 2, 3 };
	std::vector<double> floats = { 1.125, 2.567, 3.874 };

	std::string res =
#	include <common/tests/templates/array_vector.htmltc>
	;

	CHECK( res == correctAnswer );
}


TEST_CASE( "preprocess formatted floats" ) {
	std::array<unsigned short, 3> ints = { 1, 2, 3 };
	std::vector<double> floats = { 1.1254444, 2.5673333, 3.8742222 };

	std::string res =
#	include <common/tests/templates/formatted_floats.htmltc>
	;

	CHECK( res == correctAnswer );
}

#include <common/serialize.hpp>
#include <common/intospection.hpp>

#include <common/catch.hpp>



DEFINE_STRUCT(
	TestPod,
	(int8_t) a,
	(uint8_t) b,
	(int16_t) c,
	(uint16_t) d,
	(int32_t) e,
	(uint32_t) f,
	(int64_t) g,
	(uint64_t) h,
	(float) i,
	(double) j
);

static_assert(std::is_pod<TestPod>::value, "NOT POD");

TEST_CASE( "serialize pod" ) {
	TestPod in{
		.a = 'a',
		.b = 'b',
		.c = -1234,
		.d = 1234,
		.e = -12345678,
		.f = 12345678,
		.g = -123456789000,
		.h = 123456789000,
		.i = 123.456,
		.j = 234.456,
	};
	TestPod out;

	serialize::Output output;
	output << in;

	serialize::Input input;
	input.data() = output.data();
	input >> out;

	//std::cerr << in << std::endl << out << std::endl;
	CHECK(in == out);
}



DEFINE_STRUCT(
	TestCollections,
	(std::string) a,
	(std::string) b,
	(std::vector<uint32_t>) c,
	(std::array<uint32_t, 4>) d,
	(std::deque<uint8_t>) e,
	(std::list<uint16_t>) f,
//	(std::forward_list<uint64_t>) g,
	(std::set<float>) h,
	(std::multiset<float>) i,
	(std::unordered_set<float>) j,
	(std::unordered_multiset<float>) k,
	(std::map<float, int16_t>) l,
	(std::multimap<float, int16_t>) m,
	(std::unordered_map<float, int16_t>) n,
	(std::unordered_multimap<float, int16_t>) o
)

TEST_CASE( "serialize collections" ) {
	TestCollections in{
		.a = {""},
		.b = {"qwerrtyuiop"},
		.c = {1, 2, 4, 7},
		.d = {{1, 2, 4, 7}},
		.e = {1, 2, 4, 7},
		.f = {1, 2, 4, 7},
//		{1, 2, 4, 7},
		.h = {1.1, 2.2, 4.4, 7.7},
		.i = {1.1, 2.2, 4.4, 7.7},
		.j = {1.1, 2.2, 4.4, 7.7},
		.k = {1.1, 2.2, 4.4, 7.7},
		.l = {{1.1, -1000}, {2.2, 1000}, {4.4, -30000}, {7.7, 30001}},
		.m = {{1.1, -1000}, {2.2, 1000}, {4.4, -30000}, {7.7, 30001}},
		.n = {{1.1, -1000}, {2.2, 1000}, {4.4, -30000}, {7.7, 30001}},
		.o = {{1.1, -1000}, {2.2, 1000}, {4.4, -30000}, {7.7, 30001}}
	};

	TestCollections out;

	serialize::Output output;
	output << in;

	serialize::Input input;
	input.data() = output.data();
	input >> out;

	//std::cerr << in << std::endl << out << std::endl;
	CHECK(in == out);
}



DEFINE_STRUCT(
	TestCollections2,
	(std::vector<std::list<std::set<uint8_t>>>) a
)


TEST_CASE( "serialize collections of collections" ) {
	TestCollections2 in{
		.a = {
			{ { 1, 2, 3 }, { 4, 5, 6}, { 7, 8, 9 } },
			{ { 1, 2, 3 }, { 4, 5, 6}, { 7, 8, 9 } },
			{ { 1, 2, 3 }, { 4, 5, 6}, { 7, 8, 9 } },
			{ { 9, 8, 7 }, { 6, 5, 4}, { 3, 2, 1 } },
			{ { 9, 8, 7 }, { 6, 5, 4}, { 3, 2, 1 } },
			{ { 9, 8, 7 }, { 6, 5, 4}, { 3, 2, 1 } }
		}
	};

	TestCollections2 out;

	serialize::Output output;
	output << in;

	serialize::Input input;
	input.data() = output.data();
	input >> out;

	//std::cerr << in << std::endl << out << std::endl;
	CHECK(in == out);
}



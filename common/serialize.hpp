#pragma once

#include <cstdint>

#include <vector>
#include <string>
#include <array>
#include <bitset>
#include <deque>
#include <queue>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <list>
#include <forward_list>



namespace serialize {

class Buffer {
public:
	std::vector<uint8_t> & data() { return data_; }
	const std::vector<uint8_t> & data() const { return data_; }

private:
	std::vector<uint8_t> data_;
};


class Input : public Buffer {

	struct Read {
		Input & input;

		template<typename T> void operator()(T & t) const {
			input.read(t);
		}
	};

public:

	template<typename Type> Input & operator>>(Type & x) {
		x.forEachField(Read{*this});
		return *this;
	}

private:

	// reads count items
	template<typename T> void read(T * p, size_t count = 1) {
		if (std::is_pod<T>::value) {
			auto size = sizeof(T) * count;
			if (pos + size > data().size()) throw "no data";
			std::copy(data().begin()+pos, data().begin()+pos+size, (uint8_t*)p);
			pos += size;
		} else {
			for (size_t i = 0; i < count; i++) {
				read(p[i]);
			}
		}
	}

	template<typename T> void readArray(T & x) {
		read(x.data(), x.size());
	}

	template<typename T> void readVector(T & x) {
		x.resize(read<uint32_t>());
		readArray(x);
	}

	template<typename T> void readList(T & x) {
		auto size = read<uint32_t>();
		for (auto i = 0; i < size; i++) {
			x.insert(x.end(), read<typename T::value_type>());
		}
	}

	template<typename T> void readSet(T & x) {
		auto size = read<uint32_t>();
		for (auto i = 0; i < size; i++) {
			x.insert(read<typename T::value_type>());
		}
	}

	template<typename ... Args> void read(std::array<Args...> & x) { readArray(x); }

	template<typename ... Args> void read(std::vector      <Args...> & x) { readVector(x); }
	template<typename ... Args> void read(std::basic_string<Args...> & x) { readVector(x); }

	template<typename ... Args> void read(std::deque       <Args...> & x) { readList(x); }
	template<typename ... Args> void read(std::forward_list<Args...> & x) { readList(x); }
	template<typename ... Args> void read(std::list        <Args...> & x) { readList(x); }

	template<typename ... Args> void read(std::set<Args...> & x) { readSet(x); }
	template<typename ... Args> void read(std::map<Args...> & x) { readSet(x); }
	template<typename ... Args> void read(std::multiset<Args...> & x) { readSet(x); }
	template<typename ... Args> void read(std::multimap<Args...> & x) { readSet(x); }
	template<typename ... Args> void read(std::unordered_set<Args...> & x) { readSet(x); }
	template<typename ... Args> void read(std::unordered_map<Args...> & x) { readSet(x); }
	template<typename ... Args> void read(std::unordered_multiset<Args...> & x) { readSet(x); }
	template<typename ... Args> void read(std::unordered_multimap<Args...> & x) { readSet(x); }

	template<typename ... Args> void read(std::pair<Args...> & x) { read(x.first); read(x.second); }

	template<size_t S> void read(std::bitset<S> & x) { x = std::bitset<S>(read<unsigned long>()); }


	template<typename T> void read(T & x) {
		static_assert(std::is_pod<T>::value, "don't know how to read this class");
		read(&x);
	}

	template<typename T> T read() {
		T x;
		read(x);
		return x;
	}


	size_t pos = 0;

};

class Output : public Buffer {

	struct Write {
		Output & output;

		template<typename T> void operator()(T& t) const {
			output.write(t);
		}
	};

public:

	template<typename Type> Output & operator<<(const Type & x) {
		x.forEachField(Write{*this});
		return *this;
	}

private:

	// writes count items
	template<typename T> void write(const T * d, size_t count = 1) {
		if (std::is_pod<T>::value) {
			auto size = sizeof(T) * count;
			auto p = (uint8_t*)d;
			data().insert(data().end(), p, p + size);
		} else {
			for (size_t i = 0; i < count; i++) {
				write(d[i]);
			}
		}
	}

	template<typename T> void writeArray(const T & x) {
		write(x.data(), x.size());
	}

	template<typename T> void writeResizableCollection(const T & x) {
		write(uint32_t(x.size()));
		for (const typename T::value_type & item : x) write(item);
	}

	template<typename ... Args> void write(const std::array<Args...> & x) { writeArray(x); }

	template<typename ... Args> void write(const std::vector      <Args...> & x) { writeResizableCollection(x); }
	template<typename ... Args> void write(const std::basic_string<Args...> & x) { writeResizableCollection(x); }

	template<typename ... Args> void write(const std::deque       <Args...> & x) { writeResizableCollection(x); }
	template<typename ... Args> void write(const std::forward_list<Args...> & x) { writeResizableCollection(x); }
	template<typename ... Args> void write(const std::list        <Args...> & x) { writeResizableCollection(x); }

	template<typename ... Args> void write(const std::set<Args...> & x) { writeResizableCollection(x); }
	template<typename ... Args> void write(const std::map<Args...> & x) { writeResizableCollection(x); }
	template<typename ... Args> void write(const std::multiset<Args...> & x) { writeResizableCollection(x); }
	template<typename ... Args> void write(const std::multimap<Args...> & x) { writeResizableCollection(x); }
	template<typename ... Args> void write(const std::unordered_set<Args...> & x) { writeResizableCollection(x); }
	template<typename ... Args> void write(const std::unordered_map<Args...> & x) { writeResizableCollection(x); }
	template<typename ... Args> void write(const std::unordered_multiset<Args...> & x) { writeResizableCollection(x); }
	template<typename ... Args> void write(const std::unordered_multimap<Args...> & x) { writeResizableCollection(x); }

	template<typename ... Args> void write(const std::pair<Args...> & x) { write(x.first); write(x.second); }

	template<size_t S> void write(const std::bitset<S> & x) { write(x.to_ulong()); }


	template<typename T> void write(const T & x) {
		static_assert(std::is_pod<T>::value, "don't know how to write this class");
		write(&x);
	}
};

}


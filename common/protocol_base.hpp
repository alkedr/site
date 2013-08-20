#pragma once

#include <vector>
#include <cstdint>



template<class _Header> class Message {
public:

	using Header = _Header;
	
	Message() : data_(sizeof(Header)) {
		new (headerPtr()) Header();
	}

	const void * headerPtr() const { return data(); }
	void * headerPtr() { return data(); }
	const Header & header() const { return *static_cast<const Header*>(headerPtr()); }
	Header & header() { return *static_cast<Header*>(headerPtr()); }
	std::size_t headerSize() const { return sizeof(Header); }

	const void * bodyPtr() const { return (const char*)data() + headerSize(); }
	void * bodyPtr() { return (char*)data() + headerSize(); }
	std::size_t bodySize() const { return dataSize() - headerSize(); }
	void setBodySize(std::size_t value) { data_.resize(headerSize() + value); }

	const void * data() const { return data_.data(); }
	void * data() { return data_.data(); }
	std::size_t dataSize() const { return data_.size(); }

protected:

	std::vector<uint8_t> data_;
};


#pragma once


#include <cstdint>


namespace lfdb {

using ItemId = uint32_t;

namespace _ {

class ItemBase {
public:

	using RefCount = uint32_t;

	RefCount atomicIncrementRefCount() {
		return __atomic_add_fetch(&refCount, 1, __ATOMIC_SEQ_CST);
	}

	RefCount atomicDecrementRefCount() {
		return __atomic_sub_fetch(&refCount, 1, __ATOMIC_SEQ_CST);
	}

	ItemId id() const { return id_; }
	void setId(ItemId value) { id_ = value; }

private:
	RefCount refCount_ = 0;
	ItemId id_;
};

}

template<class Data> class Item : public _::ItemBase {
public:
private:
	Data data_;
};

}

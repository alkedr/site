#pragma once


#include <vector>
#include <memory>
#include <atomic>
#include <boost/thread.hpp>


class ItemBase {
public:
	uint32_t refCount = 0;
};

class HelperBase {
protected:
	union {
		struct {
			ItemBase * pItem_;
			uint16_t threadsCount_;
		};
		__uint128_t asUint128_;
	};

	HelperBase() {
		__atomic_store_n(&asUint128_, 0, __ATOMIC_SEQ_CST);
	}

	ItemBase * _ref() {
		__atomic_add_fetch(&threadsCount_, 1, __ATOMIC_SEQ_CST);
		ItemBase * item = __atomic_load_n(&pItem_, __ATOMIC_SEQ_CST);
		__atomic_add_fetch(&pItem_->refCount, 1, __ATOMIC_SEQ_CST);
		__atomic_sub_fetch(&threadsCount_, 1, __ATOMIC_SEQ_CST);
		return item;
	}

	void _replace(ItemBase * newPItem) {
		HelperBase oldHelper = *this;
		HelperBase newHelper = { .pItem_ = newPItem, threadsCount_ = 0 };
		do {
			oldHelper.threadsCount_ = 0;
		} while (__atomic_compare_exchange_n(&asUint128_, &oldHelper.asUint128_, newHelper.asUint128_, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST));
	}

	static void _deref(ItemBase * pItem_) {
		auto newRefCount = __atomic_sub_fetch(&pItem_->refCount, 1, __ATOMIC_SEQ_CST);
		if (newRefCount == 0) {
			delete pItem_;
		}
	}

};

template<class T> class Helper : HelperBase {
public:
	using HelperBase::HelperBase;

	T * ref() {
		static_cast<T*>(_ref());
	}

	void replace(T * newPItem) {
		_replace(static_cast<ItemBase*>(pItem_));
	}

	static void deref(T * pItem_) {
		_deref(static_cast<ItemBase*>(pItem_));
	}

};


template<class T> class ROPtr {
public:
	ROPtr(Helper<T> & helper) {
		pItem_ = helper.ref();
	}

	~ROPtr() {
		Helper<T>::deref(pItem_);
	}

	T * operator->() const {
		return pItem_;
	}

private:
	T * pItem_;
};


template<class T> class RWPtr {
public:
private:
};




template<class T> class AtomicIntrusivePtr {
};


template<class ItemPtr, class HookExtractor, class KeyExtractor, class Comparator> class IndexRbtree {
public:

	IndexRbtree(const IndexRbtree&) = delete;
	IndexRbtree(IndexRbtree&&) = delete;

	IndexRbtree & operator=(const IndexRbtree&) = delete;
	IndexRbtree & operator=(IndexRbtree&&) = delete;


	void insert(ItemPtr item) {
	}

	void erase(ItemPtr item) {
	}



private:

	struct Node {
		AtomicIntrusivePtr<Node> left;
		AtomicIntrusivePtr<Node> right;
		ItemPtr itemPtr;
/*
		void ref() { refCount++; }
		void deref() { refCount--; }
		uint32_t refCount() const { return refCount_; }*/
	};

	Node root;
};









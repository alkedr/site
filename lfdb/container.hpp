#pragma once

/*
 * Everything is thread-safe
 *
 * Usage:
 *
 * LFDB_CONTAINER(
 *   MyCoolStore,
 *   MyCoolItem,
 *   INDEXES(
 *     ID(byId),
 *     RBTREE(byA, a),
 *     SUFFIX_TREE(byC, c)
 *   )
 * )
 *
 * MyCoolStore myCoolItems;
 *
 *
 */


//#include <lfdb/item.hpp>
//#include <lfdb/indexes.hpp>

#include <utility>
#include <memory>
#include <atomic>
#include <thread>




#define __LFDB_INDEXES_LIST_TO_TYPE(INDEXES_LIST)



#define LFDB_CONTAINER(NAME, ITEM_TYPE, INDEXES_LIST)                    \
class NAME {                                                             \
public:                                                                  \
private:                                                                 \
};


/*
namespace lfdb {

using ItemId = uint32_t;

template<class Data, class Indexes> class Container : public Indexes {
public:

	template<typename... Args> ItemId insert(Args&&... args) {
		Indexes::insert(allocateItem(args));
	}

	template<typename F> void modify(ItemId itemId, F f) {
		std::unique_lock<std::mutex> _(writerMutex);
		std::unique_ptr<Item<Data>> newItem(copyItem(itemId));
		f(*newItem);
		Indexes::replace(itemId, newItem.release());
	}

	void erase(ItemId itemId) {
		Indexes::erase(itemId);
	}

private:

	template<typename... Args> Item<Data>* allocateItem(Args&&... args) {
		return new Item<Data>(std::forward<Args>(args)...);
	}

	Item<Data>* copyItem(ItemId itemId) {
		return allocateItem(*indexes.byId.getReadPtr(itemId));
	}



	Item * itemPtr(ItemId itemId) const {
		union {
			uint32_t u32;
			struct {
				uint16_t u16l;
				uint16_t u16h;
			};
		} u;
		u.u32 = itemId;

		if (!v[u.u16h]) return nullptr;
		return v[u.u16h][u.u16l];
	}



	std::atomic<ItemId> nextId;

	std::array< std::unique_ptr<std::array<std::shared_ptr<IndexedItem>, 65536>>, 65536> v;  // index by id,  indexes - loword and hiword of id,  v[hiword][loword]

	Indexes indexes;

};


}*/


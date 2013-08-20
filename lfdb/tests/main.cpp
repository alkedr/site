#define CATCH_CONFIG_MAIN
#include <common/catch.hpp>

#include <lfdb/container.hpp>


LFDB_CONTAINER(
	TestItems,
	TestItem,
	INDEXES(
		ID(byId),
		RBTREE(byField1, field1)
	)
)


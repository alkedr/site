#pragma once

#include <common/catch.hpp>

#include <common/intospection.hpp>
#include <common/rpc.hpp>


namespace rpc {
	namespace myrpc {
		namespace myMethod {
			DEFINE_STRUCT(Request,
				(int) a,
				(std::string) b,
				(std::map<int, int>) c
			);
			DEFINE_STRUCT(Response,
				(int) a
			);
		}
	}
}


DEFINE_RPC(
	rpc::myrpc,
	(1) myMethod
)

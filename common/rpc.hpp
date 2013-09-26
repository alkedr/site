#pragma once

/*
 * RPC - macro that defines simple RPC interface
 *
 * Example:
 *
 * namespace rpc {
 *   namespace myrpc {
 *     namespace myMethod {
 *       DEFINE_STRUCT(Request,
 *         (int) a,
 *         (std::string) b,
 *         (std::map<int, int>) c
 *       );
 *       DEFINE_STRUCT(Response,
 *         ...
 *       );
 *     }
 *     ...
 *     DEFINE_RPC(
 *       (1) myMethod,
 *       (2) myOtherMethod
 *     )
 *   }
 * }
 * 
 * will generate:
 *
 * namespace rpc {
 *   namespace myrpc {
 *     namespace myMethod {
 *       struct Request {...};
 *       struct Response {...};
 *     }
 *     class Client {
 *     public:
 *       myMethod::Response myMethod(myMethod::Request&) {...}
 *     };
 *     class Server {
 *     public:
 *       virtual myMethod::Response myMethod(myMethod::Request&) = 0;
 *     }
 *   }
 * }
 *
 */



#include <common/data.hpp>

#define BOOST_PP_VARIADICS
#include <boost/preprocessor.hpp>


#define __DEFINE_RPC_REMOVE_PARANTHESES(...) __VA_ARGS__
#define __DEFINE_RPC_REMOVE_ID(...)

#define TYPEOF(x) DETAIL_TYPEOF(DETAIL_TYPEOF_PROBE x,)
#define DETAIL_TYPEOF(...) DETAIL_TYPEOF_HEAD(__VA_ARGS__)
#define DETAIL_TYPEOF_HEAD(x, ...) __DEFINE_RPC_REMOVE_PARANTHESES x
#define DETAIL_TYPEOF_PROBE(...) (__VA_ARGS__),


#define __DEFINE_RPC_CLIENT_METHOD_IMPL(NAME)                         \
NAME::Response NAME(NAME::Request & request) {                        \
	NAME::Response response;                                            \
	__makeRequest(request, response);                                   \
	return response;                                                    \
}

#define __DEFINE_RPC_CLIENT_METHOD(R, DATA, ITEM) __DEFINE_RPC_CLIENT_METHOD_IMPL(__DEFINE_RPC_REMOVE_ID ITEM)

#define __DEFINE_RPC_SERVER_METHOD_IMPL(NAME) virtual NAME::Response NAME(NAME::Request & request) = 0;

#define __DEFINE_RPC_SERVER_METHOD(R, DATA, ITEM) __DEFINE_RPC_SERVER_METHOD_IMPL(__DEFINE_RPC_REMOVE_ID ITEM)

#define __DEFINE_RPC_SERVER_DISPATCH_METHOD_IMPL(ID, NAME) case (ID): response << NAME(request); break;

#define __DEFINE_RPC_SERVER_DISPATCH_METHOD(R, DATA, ITEM)                                                      \
	__DEFINE_RPC_SERVER_DISPATCH_METHOD_IMPL(TYPEOF(ITEM), __DEFINE_RPC_REMOVE_ID ITEM)




#define DEFINE_RPC(...)                                                                                                 \
class Client : rpc::_::Client {                                                                                         \
public:                                                                                                                 \
	BOOST_PP_SEQ_FOR_EACH(__DEFINE_RPC_CLIENT_METHOD, ~, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))                           \
};                                                                                                                      \
class Server : rpc::_::Server {                                                                                         \
protected:                                                                                                              \
	BOOST_PP_SEQ_FOR_EACH(__DEFINE_RPC_SERVER_METHOD, ~, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))                           \
private:                                                                                                                \
	virtual void __dispatch(data::Request & request, data::Response & response) override {                                \
		switch (request.method) {                                                                                           \
		BOOST_PP_SEQ_FOR_EACH(__DEFINE_RPC_SERVER_DISPATCH_METHOD, ~, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))                \
		default:                                                                                                            \
		}                                                                                                                   \
	}                                                                                                                     \
};


#include <common/daemon.hpp>
#include <common/data.hpp>


class RequestHandler {
public:
	void operator()(data::protocol::Request & request, data::protocol::Response & response) {
	}
};


DAEMON("filmd", "1.0.0", data::Server<RequestHandler>)


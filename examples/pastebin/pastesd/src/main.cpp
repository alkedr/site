#include <common/daemon.hpp>
#include <common/data.hpp>


class RequestHandler {
public:
	void operator()(data::protocol::Request & request, data::protocol::Response & response) {
	}
};


DAEMON("pastesd", "1.0.0", data::Server<RequestHandler>)


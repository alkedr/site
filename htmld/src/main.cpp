#include <common/daemon.hpp>
#include <common/fcgi.hpp>
#include <common/templater.hpp>


class RequestHandler {
public:
	template<class ResponseWriter> void operator()(fcgi::protocol::Request & request, ResponseWriter responseWriter) {
		std::cerr << __PRETTY_FUNCTION__ << std::endl;
		auto response = std::make_shared<fcgi::protocol::Response>();
		response->stdout = "Content-type: text/html\r\n\r\n" +
#	include <htmld/templates/index.htmltc>
		;
		responseWriter(response);
	}
};


DAEMON("htmld", "1.0.0", fcgi::Server<RequestHandler>)


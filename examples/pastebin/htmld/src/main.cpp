#include <common/daemon.hpp>
#include <common/fcgi.hpp>
#include <common/templater.hpp>


class RequestHandler {
public:
	template<class ResponseWriter> void operator()(fcgi::protocol::Request & request, ResponseWriter responseWriter) {
		LOG_DEBUG(__PRETTY_FUNCTION__);

		auto response = std::make_shared<fcgi::protocol::Response>();
		if (request.uri() == "/") {
			response->stdout = "Content-type: text/html\r\n\r\n" + index();
		}
		responseWriter(response);
	}

private:

	std::string index() {
		return
#	include <examples/pastebin/htmld/templates/index.htmltc>
		;
	}

	std::string paste() {
		std::string text = "some data";

		return
#	include <examples/pastebin/htmld/templates/paste.htmltc>
		;
	}
};


DAEMON("htmld", "1.0.0", fcgi::Server<RequestHandler>)


#include <common/daemon.hpp>
#include <common/scgi.hpp>
#include <common/templater.hpp>

namespace {


class RequestHandler {
public:
	template<class ResponseWriter> void operator()(scgi::protocol::Request & request, ResponseWriter responseWriter) {
		LOG_DEBUG(__PRETTY_FUNCTION__);

		auto response = std::make_shared<scgi::protocol::Response>();
		if (request.uri() == "/") {
			response->stdout = "Content-type: text/html\r\n\r\n" + index();
		}

/*
		// one request
		daemon1.method1(parameters)
			([](Response response) {
				// completion handler
			});

		// many requests
		int count = 0;
		daemon1.method1(parameters)
			([&count](Response1 response) {
				count++;
				if (count == 2) RealHandler(response);
			});
		daemon2.method1(parameters)
			([&count](Response2 response) {
				count++;
				if (count == 2) RealHandler(response);
			});



		data::async(
			daemon1.method1(parameters),
			daemon2.method1(parameters)
		)(
			[](Response1, Response2) {
				... some processing ...
				responseWriter(response);
			}
		)


		data::request(
			daemon1.method1(parameters),   // returns future
			daemon2.method1(parameters)
		).request(
			daemon3.method1(parameters)
		).than(
			[](Response1, Response2, Response3) {
				... some processing ...
				responseWriter(response);
			}
		)
*/

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


}


DAEMON("htmld", "1.0.0", scgi::Server<RequestHandler>)


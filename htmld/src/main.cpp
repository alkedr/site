#include <common/fcgi.hpp>
#include <common/daemon.hpp>
#include <common/templater.hpp>


class PageServer : public fcgi::Server {
public:
	using Server::Server;

private:
	virtual void handleRequest(fcgi::protocol::Request & request, fcgi::protocol::Response & response) {
		std::cerr << __PRETTY_FUNCTION__ << std::endl;
		response.stdout = "Content-type: text/html\r\n\r\n" +
#	include <htmld/templates/index.htmltc>
		;
	}
};

class Htmld : public Daemon<PageServer> {
public:
	using Daemon::Daemon;
	virtual std::string name() const override { return "htmld"; }
	virtual std::string version() const override { return "1.0.0"; }
};

int main(int argc, char ** argv) {
	return Htmld(argc, argv).run();
}


#pragma once

#include <string>
#include <boost/asio.hpp>



template<class Server> class Daemon {

protected:
	virtual std::string name() const = 0;
	virtual std::string version() const = 0;

public:
	Daemon(int argc, char ** argv) {}

	int run() {
		std::cerr << name() << " " << version() << std::endl;
		// TODO: cmd options
		boost::asio::io_service io_service;
		Server server(io_service);
		server.start(1);
		server.join();  // TODO: unix signals
		return 0;
	}
};



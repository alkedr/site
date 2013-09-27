#pragma once

#include <string>
#include <boost/asio/io_service.hpp>



namespace {
namespace daemon {

template<class Server> int main(const char * name, const char * version, int argc, char ** argv) {
	std::cerr << name << " " << version << std::endl;
	// TODO: cmd options
	boost::asio::io_service io_service;
	Server server(io_service);
	server.start(1);
	server.join();  // TODO: unix signals
	return 0;
}

}
}


// Usage:
//   DAEMON("myDaemon", "1.0.0", MyServer)
#define DAEMON(NAME, VERSION, SERVER) int main(int argc, char ** argv) { return daemon::main<SERVER>(NAME, VERSION, argc, argv); }

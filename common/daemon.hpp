#pragma once

#include <boost/asio/io_service.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>



namespace {

template<class Server> class Daemon {
public:
	Daemon(const char * name, const char * version, int argc, char ** argv)
	: name_(name)
	, version_(version)
	, io_service_()
	, server(io_service_)
	{
		boost::program_options::options_description desc("Allowed options");
		desc.add_options()
			("help", "produce help message")
			("compression", boost::program_options::value<int>(), "set compression level")
			;

		boost::program_options::variables_map vm;
		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
		boost::program_options::notify(vm);    
	}

	int run() {
		std::cerr << name_ << " " << version_ << std::endl;
		server.start(1);
		server.join();
		return 0;
	}

private:
	const char * name_;
	const char * version_;

	boost::asio::io_service io_service_;
	Server server;
};

}


// Usage:
//   DAEMON("myDaemon", "1.0.0", MyServer)
#define DAEMON(NAME, VERSION, ...) int main(int argc, char ** argv) { return Daemon<__VA_ARGS__>(NAME, VERSION, argc, argv).run(); }


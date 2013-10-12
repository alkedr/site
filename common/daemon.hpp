#pragma once

#include <common/log.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <systemd/sd-daemon.h>



namespace {

template<class Server> class Daemon {
public:

	Daemon(const char * name, const char * version, int argc, char ** argv)
	: name_(name)
	, version_(version)
	, description("Allowed options")
	{
		description.add_options()
			("version", "print name and version")
			("help", "print this help")
			("host", boost::program_options::value<std::string>(), "set host (for non-systemd testing)")
			("port", boost::program_options::value<uint32_t>(), "set port (for non-systemd testing)")
			;

		boost::program_options::variables_map vm;
		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, description), vm);
		boost::program_options::notify(vm);    

		if (vm.count("version")) {
			needToPrintNameAndVersion = true;
			needToStartServer = false;
		}

		if (vm.count("help")) {
			needToPrintHelp = true;
			needToStartServer = false;
		}

		if (vm.count("host")) {
			takeSocketsFromSystemd = false;
			host = boost::asio::ip::address::from_string(vm["host"].as<std::string>());
		}

		if (vm.count("port")) {
			takeSocketsFromSystemd = false;
			port = vm["port"].as<unsigned>();
		}
	}


	int run() {
		if (needToPrintNameAndVersion) {
			std::cout << name_ << " " << version_ << std::endl;
		}

		if (needToPrintHelp) {
			std::cout << description;
		}

		if (needToStartServer) {
			boost::asio::io_service io_service_;
			Server server(createAcceptor(io_service_));
			server.start(1);
			server.join();
		}

		return 0;
	}

private:

	boost::asio::ip::tcp::acceptor createAcceptor(boost::asio::io_service & io_service) const {
		if (takeSocketsFromSystemd) {
			LOG_INFO("taking socket from systemd");
			return boost::asio::ip::tcp::acceptor(io_service, boost::asio::ip::tcp::v4(), SD_LISTEN_FDS_START);
		} else {
			LOG_INFO("listening on " << host << ':' << port);
			return boost::asio::ip::tcp::acceptor(io_service, boost::asio::ip::tcp::endpoint(host, port));
		}
	}

	const char * name_;
	const char * version_;

	boost::program_options::options_description description;

	bool needToPrintNameAndVersion = true;
	bool needToPrintHelp = false;
	bool needToStartServer = true;
	bool takeSocketsFromSystemd = true;

	boost::asio::ip::address host;
	unsigned short port = 0;
};

}


// Usage:
//   DAEMON("myDaemon", "1.0.0", MyServer)
#define DAEMON(NAME, VERSION, ...) int main(int argc, char ** argv) { return Daemon<__VA_ARGS__>(NAME, VERSION, argc, argv).run(); }


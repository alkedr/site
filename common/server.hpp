#pragma once

#include <common/log.hpp>
#include <common/connection.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/thread/thread.hpp>



template<class ConnectionHandler> class Server {
public:
	Server(boost::asio::ip::tcp::acceptor && acceptor) : acceptor_(std::move(acceptor)) {
	}

	void start(int threadsCount) {
		for (auto i = 0; i != threadsCount; ++i) {
			workers_.create_thread(
				[this]() {
					boost::asio::ip::tcp::socket clientSocket(io_service());
					startAccepting(clientSocket);
					io_service().run();
				}
			);
		}
	}

	void join() {
		workers_.join_all();
	}

private:

	boost::asio::io_service & io_service() { return acceptor_.get_io_service(); }
	const boost::asio::io_service & io_service() const { return acceptor_.get_io_service(); }

	void startAccepting(boost::asio::ip::tcp::socket & socket) {
		LOG_DEBUG("server " << __FUNCTION__);
		acceptor_.async_accept(
			socket,
			[&socket, this](boost::system::error_code error) {
				if (!error) {
					connectionHandler_(std::make_shared<Connection>(std::move(socket)));
				} else {
					LOG_ERROR(error);
					sleep(1);
				}
				startAccepting(socket);
			}
		);
	}

	boost::asio::ip::tcp::acceptor acceptor_;
	boost::thread_group workers_;
	ConnectionHandler connectionHandler_;
};

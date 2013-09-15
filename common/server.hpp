#pragma once

#include <common/log.hpp>

#include <boost/asio.hpp>
#include <boost/thread.hpp>



template<class ConnectionHandler> class Server {
public:
	Server(boost::asio::io_service & io_service) : io_service_(io_service), acceptor_(io_service, boost::asio::ip::tcp::v4(), SD_LISTEN_FDS_START) {
		LOG_DEBUG(__PRETTY_FUNCTION__);
	}

	void start(int threadsCount) {
		LOG_DEBUG(__PRETTY_FUNCTION__);
		for (auto i = 0; i != threadsCount; ++i) {
			workers_.create_thread(
				[this]() {
					boost::asio::ip::tcp::socket clientSocket(io_service_);
					startAccepting(clientSocket);
					io_service_.run();
				}
			);
		}
	}

	void join() {
		workers_.join_all();
	}

private:
	void startAccepting(boost::asio::ip::tcp::socket & socket) {
		LOG_DEBUG(__PRETTY_FUNCTION__);
		acceptor_.async_accept(
			socket,
			[&socket, this](boost::system::error_code error) {
				if (!error) {
					connectionHandler_(std::move(socket));
				} else {
					LOG_ERROR(error);
					sleep(1);
				}
				startAccepting(socket);
			}
		);
	}

	boost::asio::io_service & io_service_;
	boost::asio::ip::tcp::acceptor acceptor_;
	boost::thread_group workers_;
	ConnectionHandler connectionHandler_;
};

#pragma once

#include <common/log.hpp>

#include <boost/asio.hpp>
#include <boost/thread.hpp>




class Server {
public:
	Server(boost::asio::io_service & io_service) : io_service_(io_service), acceptor(io_service, boost::asio::ip::tcp::v4(), SD_LISTEN_FDS_START) {
		LOG_DEBUG(__PRETTY_FUNCTION__);
	}

	void start(int threadsCount) {
		LOG_DEBUG(__PRETTY_FUNCTION__);
		for (std::size_t i = 0; i != threadsCount; ++i) {
			workers_.create_thread(
				[&]() {
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

protected:

	virtual void handleConnection(boost::asio::ip::tcp::socket socket) = 0;

private:
	void startAccepting(boost::asio::ip::tcp::socket & socket) {
		LOG_DEBUG(__PRETTY_FUNCTION__);
		acceptor.async_accept(
			socket,
			[&, this](boost::system::error_code error) {
				if (error) {
					LOG_ERROR(error);
					sleep(1);
				} else {
					handleConnection(std::move(socket));
				}
				startAccepting(socket);
			}
		);
	}

	boost::asio::ip::tcp::acceptor acceptor;
	boost::asio::io_service & io_service_;
	boost::thread_group workers_;
};

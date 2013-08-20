#pragma once

#include <boost/asio.hpp>



class Connection {
public:
	Connection(boost::asio::ip::tcp::socket && socket) : socket_(std::move(socket)) {
	}

	boost::asio::ip::tcp::socket & socket() { return socket_; }
	const boost::asio::ip::tcp::socket & socket() const { return socket_; }

private:
	boost::asio::ip::tcp::socket socket_;
};


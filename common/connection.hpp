#pragma once

#include <common/log.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>



class Connection {
public:
	Connection(boost::asio::ip::tcp::socket && socket) : socket_(std::move(socket)) {
	}

	template<typename CompletionHandler>
	void connect(const boost::asio::ip::tcp::socket::endpoint_type & peer_endpoint, CompletionHandler handler) {
		socket_.async_connect(
			peer_endpoint,
			[](const boost::system::error_code & error) {
				if (!error) {
					handler();
				} else {
					LOG_ERROR(error);
				}
			}
		);
	}

	template<typename MutableBufferSequence, typename CompletionHandler>
	void read(const MutableBufferSequence & buffers, CompletionHandler handler) {
		std::size_t expectedLength = std::distance(buffers.begin(), buffers.end());
		boost::asio::async_read(
			socket_,
			buffers,
			[expectedLength, &handler](boost::system::error_code error, std::size_t length) {
				if ((!error) && (length != expectedLength)) {
					handler();
				} else {
					LOG_ERROR(error);
				}
			}
		);
	}

	template<typename MutableBufferSequence, typename CompletionHandler>
	void read_until(const MutableBufferSequence & buffers, char c, CompletionHandler handler) {
		std::size_t expectedLength = std::distance(buffers.begin(), buffers.end());
		boost::asio::async_read_until(
			socket_,
			buffers,
			c,
			[expectedLength, &handler](boost::system::error_code error, std::size_t length) {
				if (!error) {
					handler(length);
				} else {
					LOG_ERROR(error);
				}
			}
		);
	}

	template<typename ConstBufferSequence, typename CompletionHandler>
	void write(const ConstBufferSequence & buffers, CompletionHandler handler) {
		std::size_t expectedLength = std::distance(buffers.begin(), buffers.end());
		boost::asio::async_write(
			socket_,
			buffers,
			[expectedLength, &handler](boost::system::error_code error, std::size_t length) {
				if ((!error) && (length != expectedLength)) {
					handler();
				} else {
					LOG_ERROR(error);
				}
			}
		);
	}


	boost::asio::ip::tcp::socket & socket() { return socket_; }
	const boost::asio::ip::tcp::socket & socket() const { return socket_; }

private:
	boost::asio::ip::tcp::socket socket_;
};


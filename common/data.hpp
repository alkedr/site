#pragma once

#include <common/protocol_base.hpp>
#include <common/server.hpp>
#include <common/connection.hpp>

#include <memory>


namespace data {
namespace protocol {
namespace _ {

	struct RequestHeader {
		uint16_t method;
		uint16_t requestId;
		uint32_t dataLength;
	};

	struct ResponseHeader {
		uint16_t errorCode;
		uint16_t requestId;
		uint32_t dataLength;
	};

}

using Request  = Message<_::RequestHeader >;
using Response = Message<_::ResponseHeader>;

}
}


namespace data {

template<class RequestHandler> class ConnectionHandler {
public:
	void operator()(boost::asio::ip::tcp::socket socket) {
		auto connection = std::make_shared<Connection>(std::move(socket));
		startReadingHeader(connection);
	}

private:
	void startReadingHeader(std::shared_ptr<Connection> connection) {
		auto request = std::make_shared<protocol::Request>();

		boost::asio::async_read(
			connection->socket(),
			boost::asio::buffer(request->headerPtr(), request->headerSize()),
			[this, connection, request](boost::system::error_code error, std::size_t length) {
				if ((!error) || (length != request->headerSize())) {
					startReadingBody(connection, request);
				} else {
					LOG_ERROR(error);
				}
			}
		);
	}

	void startReadingBody(std::shared_ptr<Connection> connection, std::shared_ptr<protocol::Request> request) {
		boost::asio::async_read(
			connection->socket(),
			boost::asio::buffer(request->bodyPtr(), request->bodySize()),
			[this, connection, request](boost::system::error_code error, std::size_t length) {
				if ((!error) || (length != request->bodySize())) {
					startReadingHeader(connection);
					auto response = std::make_shared<protocol::Response>();
					response->header().requestId = request->header().requestId;
					requestHandler(*request, *response);
					startWritingResponse(connection, response);
				} else {
					LOG_ERROR(error);
				}
			}
		);
	}

	void startWritingResponse(std::shared_ptr<Connection> connection, std::shared_ptr<protocol::Response> response) {
		boost::asio::async_write(
			connection->socket(),
			boost::asio::buffer(response->data(), response->dataSize()),
			[this, connection, response](boost::system::error_code error, std::size_t length) {
				if ((!error) || (length != response->dataSize())) {
				} else {
					LOG_ERROR(error);
				}
			}
		);
	}


	RequestHandler requestHandler;
};


template<class RequestHandler> using Server = ::Server<ConnectionHandler<RequestHandler>>;

}



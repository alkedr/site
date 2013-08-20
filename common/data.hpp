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

class Server : ::Server {
public:
	using ::Server::Server;

protected:

	virtual void handleRequest(protocol::Request & request, protocol::Response & response) = 0;

private:

	virtual void handleConnection(boost::asio::ip::tcp::socket socket) override {
		auto connection = std::make_shared<Connection>(std::move(socket));
		startReadingHeader(connection);
	}


	void startReadingHeader(std::shared_ptr<Connection> connection) {
		auto request = std::make_shared<protocol::Request>();

		boost::asio::async_read(
			connection->socket(),
			boost::asio::buffer(request->headerPtr(), request->headerSize()),
			[this, connection, request](boost::system::error_code ec, std::size_t length) {
				startReadingBody(connection, request);
			}
		);
	}

	void startReadingBody(std::shared_ptr<Connection> connection, std::shared_ptr<protocol::Request> request) {
		boost::asio::async_read(
			connection->socket(),
			boost::asio::buffer(request->bodyPtr(), request->bodySize()),
			[this, connection, request](boost::system::error_code ec, std::size_t length) {
				startReadingHeader(connection);
				auto response = std::make_shared<protocol::Response>();
				response->header().requestId = request->header().requestId;
				handleRequest(*request, *response);
				startWritingResponse(connection, response);
			}
		);
	}

	void startWritingResponse(std::shared_ptr<Connection> connection, std::shared_ptr<protocol::Response> response) {
		boost::asio::async_write(
			connection->socket(),
			boost::asio::buffer(response->data(), response->dataSize()),
			[this, connection, response](boost::system::error_code ec, std::size_t length) {
			}
		);
	}

};

}



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
	void operator()(std::shared_ptr<Connection> connection) {
		startReadingHeader(connection);
	}

private:
	void startReadingHeader(std::shared_ptr<Connection> connection) {
		auto request = std::make_shared<protocol::Request>();

		connection->read(
			boost::asio::buffer(request->headerPtr(), request->headerSize()),
			[this, connection, request]() {
				startReadingBody(connection, request);
			}
		);
	}

	void startReadingBody(std::shared_ptr<Connection> connection, std::shared_ptr<protocol::Request> request) {
		connection->read(
			boost::asio::buffer(request->bodyPtr(), request->bodySize()),
			[this, connection, request]() {
				startReadingHeader(connection);
				auto response = std::make_shared<protocol::Response>();
				response->header().requestId = request->header().requestId;
				requestHandler(*request, *response);
				startWritingResponse(connection, response);
			}
		);
	}

	void startWritingResponse(std::shared_ptr<Connection> connection, std::shared_ptr<protocol::Response> response) {
		connection->write(
			boost::asio::buffer(response->data(), response->dataSize()),
			[this, connection, response]() {}
		);
	}


	RequestHandler requestHandler;
};


template<class RequestHandler> using Server = ::Server<ConnectionHandler<RequestHandler>>;

}



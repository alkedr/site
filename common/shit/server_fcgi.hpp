#pragma once

#include <common/server.hpp>

#include <common/connection.hpp>
#include <common/protocol_fcgi.hpp>


namespace fcgi {

class Server : public ::Server {
public:
	using ::Server::Server;

protected:

	virtual void handleRequest(protocol::Request & request, protocol::Response & response) = 0;

private:
	virtual void handleConnection(boost::asio::ip::tcp::socket socket) override {
		std::cerr << __FUNCTION__ << std::endl;
		auto connection = std::make_shared<Connection>(std::move(socket));
		auto request = std::make_shared<protocol::Request>();
		startReadingHeader(connection, request);
	}

	void startReadingHeader(std::shared_ptr<Connection> connection, std::shared_ptr<protocol::Request> request) {
		auto message = std::make_shared<protocol::Message>();

		std::cerr << __FUNCTION__ << "  " << message->headerSize() << std::endl;

		boost::asio::async_read(
			connection->socket(),
			boost::asio::buffer(message->headerPtr(), message->headerSize()),
			[this, connection, request, message](boost::system::error_code ec, std::size_t length) {
				message->header().dprint();
				startReadingBody(connection, request, message);
			}
		);
	}

	void startReadingBody(std::shared_ptr<Connection> connection, std::shared_ptr<protocol::Request> request,
			std::shared_ptr<protocol::Message> message) {
		std::cerr << __FUNCTION__ << "  " << message->bodySize() << std::endl;
		message->setBodySize(message->header().contentLength() + message->header().paddingLength);
		boost::asio::async_read(
			connection->socket(),
			boost::asio::buffer(message->bodyPtr(), message->bodySize()),
			[this, connection, request, message](boost::system::error_code ec, std::size_t length) {
				request->addMessage(*message);
				//sleep(1);
				if (request->isComplete()) {
					auto response = std::make_shared<protocol::Response>();
					//response->header().requestId = request->header().requestId;
					handleRequest(*request, *response);
					startWritingResponse(connection, response);
				} else {
					startReadingHeader(connection, request);
				}
			}
		);
	}

	void startWritingResponse(std::shared_ptr<Connection> connection, std::shared_ptr<protocol::Response> response) {
		std::cerr << __FUNCTION__ << std::endl;
		

		{
			protocol::Header header = {
				.version = 1,
				.type = 6,
				.requestIdB0 = 1,
				.requestIdB1 = 0,
				.contentLengthB0 = static_cast<uint8_t>(response->stdout.size()),
				.contentLengthB1 = 0,
				.paddingLength = 0
			};
			connection->socket().send(boost::asio::buffer(&header, sizeof(header)));
			connection->socket().send(boost::asio::buffer(response->stdout.data(), response->stdout.size()));
		}

		{
			protocol::Header header = {
				.version = 1,
				.type = 3,
				.requestIdB0 = 1,
				.requestIdB1 = 0,
				.contentLengthB0 = 0,
				.contentLengthB1 = 0,
				.paddingLength = 0
			};
			connection->socket().send(boost::asio::buffer(&header, sizeof(header)));
		}

		{
			protocol::EndRequestBody body = {};
			protocol::Header header = {
				.version = 1,
				.type = 6,
				.requestIdB0 = 1,
				.requestIdB1 = 0,
				.contentLengthB0 = sizeof(body),
				.contentLengthB1 = 0,
				.paddingLength = 0
			};
			connection->socket().send(boost::asio::buffer(&header, sizeof(header)));
			connection->socket().send(boost::asio::buffer(&body, sizeof(body)));
		}



		/*boost::asio::async_write(
			connection->socket(),
			boost::asio::buffer(response->data(), response->dataSize()),
			[this, connection, response](boost::system::error_code ec, std::size_t length) {
			}
		);*/
	}

};

}

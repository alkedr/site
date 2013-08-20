#pragma once


#include <common/protocol_base.hpp>
#include <common/server.hpp>
#include <common/connection.hpp>

#include <map>
#include <string>
#include <iostream>



namespace fcgi {

namespace protocol {

struct Header {
	uint8_t version;
	uint8_t type;
	uint8_t requestIdB1;
	uint8_t requestIdB0;
	uint8_t contentLengthB1;
	uint8_t contentLengthB0;
	uint8_t paddingLength;
	uint8_t reserved;

	uint16_t requestId() const { return static_cast<uint16_t>((requestIdB1 << 8) | requestIdB0); }
	uint16_t contentLength() const { return static_cast<uint16_t>((contentLengthB1 << 8) | contentLengthB0); }

	void dprint() {
		std::cout
 			<< "Header:" << std::endl
			<< "  version: " << (int)version << std::endl
			<< "  type: " << (int)type << std::endl
			<< "  requestId: " << (int)requestId() << std::endl
			<< "  contentLength: " << (int)contentLength() << std::endl
			<< "  paddingLength: " << (int)paddingLength << std::endl;
	}
};


struct EndRequestBody {
	unsigned char appStatusB3;
	unsigned char appStatusB2;
	unsigned char appStatusB1;
	unsigned char appStatusB0;
	unsigned char protocolStatus;
	unsigned char reserved[3];
};




enum RequestType {
	REQUEST_TYPE_BEGIN = 1,
	REQUEST_TYPE_ABORT = 2,
	REQUEST_TYPE_END = 3,
	REQUEST_TYPE_PARAMS = 4,
	REQUEST_TYPE_STDIN = 5,
	REQUEST_TYPE_STDOUT = 6,
	REQUEST_TYPE_STDERR = 7,
	REQUEST_TYPE_DATA = 8,
	REQUEST_TYPE_GET_VALUES = 9,
	REQUEST_TYPE_GET_VALUES_RESULT = 10
};


using Message = ::Message<Header>;


class Request {
public:

	void addMessage(const Message & message) {
		switch (message.header().type) {
			case (REQUEST_TYPE_BEGIN): handleBeginMessage(message); break;
			case (REQUEST_TYPE_ABORT): handleAbortMessage(message); break;
			case (REQUEST_TYPE_PARAMS): handleParamsMessage(message); break;
			case (REQUEST_TYPE_STDIN): handleStdinMessage(message); break;
			case (REQUEST_TYPE_DATA): handleDataMessage(message); break;
			case (REQUEST_TYPE_GET_VALUES): handleGetValuesMessage(message); break;
			default: handleUnknownMessage(message); break;
		}
	}

	bool isComplete() const { return parametersRead_ && stdinRead_; }
	const std::map<std::string, std::string> & parameters() const { return parameters_; }
	std::map<std::string, std::string> & parameters() { return parameters_; }
	const std::string & stdin() const { return stdin_; }
	std::string & stdin() { return stdin_; }

private:
	void handleBeginMessage(const Message & message) {
		std::clog << __FUNCTION__ << " (requestId=" << message.header().requestId() << ")" << std::endl;
	}
	void handleAbortMessage(const Message & message) {
		std::clog << __FUNCTION__ << " (requestId=" << message.header().requestId() << ")" << std::endl;
	}
	void handleParamsMessage(const Message & message) {
		std::clog << __FUNCTION__ << " (requestId=" << message.header().requestId() << ")" << std::endl;
		if (message.header().contentLength() == 0) {
			parametersRead_ = true;
		} else {
			const unsigned char * p = (unsigned char*)message.bodyPtr();
			while (p < (unsigned char*)message.bodyPtr() + message.header().contentLength()) {
				parameters_.insert(readPair(p));
			}
			
			std::cout << "parameters: " << std::endl;
			for (const auto & pair : parameters_) {
				std::cout << "  " << pair.first << " = " << pair.second << std::endl;
			}
		}
	}
	void handleStdinMessage(const Message & message) {
		std::clog << __FUNCTION__ << " (requestId=" << message.header().requestId() << ")" << std::endl;
		if (message.header().contentLength() == 0) {
			stdinRead_ = true;
		} else {
		}
	}
	void handleDataMessage(const Message & message) {
		std::clog << __FUNCTION__ << " (requestId=" << message.header().requestId() << ")" << std::endl;
	}
	void handleGetValuesMessage(const Message & message) {
		std::clog << __FUNCTION__ << " (requestId=" << message.header().requestId() << ")" << std::endl;
	}
	void handleUnknownMessage(const Message & message) {
		std::clog << __FUNCTION__ << " (requestId=" << message.header().requestId() << ")" << std::endl;
	}


	static size_t readLength(const unsigned char *& p) {
		size_t res = (p++)[0];
		if ((res >> 7) == 1) {
			res = ((res & 0x7f) << 24) + (p[0] << 16) + (p[1] << 8) + p[2];
			p += 3;
		}
		return res;
	}

	static std::pair<std::string, std::string> readPair(const unsigned char *& p) {
		size_t len1 = readLength(p);
		size_t len2 = readLength(p);
		auto res = std::make_pair(std::string((char*)p, len1), std::string((char*)p+len1, len2));
		p += len1 + len2;
		return res;
	}



	bool parametersRead_ = false;
	bool stdinRead_ = false;

	std::map<std::string, std::string> parameters_;
	std::string stdin_;
};

struct Response {
	std::string stdout;
	std::string stderr;
};

}


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

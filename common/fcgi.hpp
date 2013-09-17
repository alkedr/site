#pragma once

#include <common/protocol_base.hpp>
#include <common/server.hpp>
#include <common/connection.hpp>

#include <map>
#include <string>
#include <iostream>



namespace fcgi {

namespace protocol {

enum class RequestType : uint8_t {
	BEGIN = 1,
	ABORT = 2,
	END = 3,
	PARAMS = 4,
	STDIN = 5,
	STDOUT = 6,
	STDERR = 7,
	DATA = 8,
	GET_VALUES = 9,
	GET_VALUES_RESULT = 10
};


struct Header {
	uint8_t version;
	RequestType type;
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




using Message = ::Message<Header>;


class Request {
public:

	void addMessage(const Message & message) {
		switch (message.header().type) {
			case (RequestType::BEGIN): handleBeginMessage(message); break;
			case (RequestType::ABORT): handleAbortMessage(message); break;
			case (RequestType::PARAMS): handleParamsMessage(message); break;
			case (RequestType::STDIN): handleStdinMessage(message); break;
			case (RequestType::DATA): handleDataMessage(message); break;
			case (RequestType::GET_VALUES): handleGetValuesMessage(message); break;
			default: handleUnknownMessage(message); break;
		}
	}

	bool isComplete() const { return parametersRead_ && stdinRead_; }

	const std::map<std::string, std::string> & parameters() const { return parameters_; }
	std::map<std::string, std::string> & parameters() { return parameters_; }

	const std::string & contentType() const { return parameters_.at("CONTENT_TYPE"); }
	const std::string & uri() const { return parameters_.at("DOCUMENT_URI"); }
	const std::string & clientIp() const { return parameters_.at("REMOTE_ADDR"); }
	const std::string & clientPort() const { return parameters_.at("REMOTE_PORT"); }
	const std::string & method() const { return parameters_.at("REQUEST_METHOD"); }

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


template<class RequestHandler> class ConnectionHandler {
public:
	void operator()(boost::asio::ip::tcp::socket socket) {
		LOG_DEBUG(__PRETTY_FUNCTION__);
		auto connection = std::make_shared<Connection>(std::move(socket));
		auto request = std::make_shared<protocol::Request>();
		startReadingHeader(connection, request);
	}

private:
	void startReadingHeader(std::shared_ptr<Connection> connection, std::shared_ptr<protocol::Request> request) {
		LOG_DEBUG(__PRETTY_FUNCTION__);
		auto message = std::make_shared<protocol::Message>();

		std::cerr << __FUNCTION__ << "  " << message->headerSize() << std::endl;

		boost::asio::async_read(
			connection->socket(),
			boost::asio::buffer(message->headerPtr(), message->headerSize()),
			[this, connection, request, message](boost::system::error_code error, std::size_t length) {
				if (!error) {
					message->header().dprint();
					startReadingBody(connection, request, message);
				} else {
					LOG_ERROR(error);
				}
			}
		);
	}

	void startReadingBody(std::shared_ptr<Connection> connection, std::shared_ptr<protocol::Request> request,
			std::shared_ptr<protocol::Message> message) {
		LOG_DEBUG(__PRETTY_FUNCTION__);

		message->setBodySize(message->header().contentLength() + message->header().paddingLength);
		boost::asio::async_read(
			connection->socket(),
			boost::asio::buffer(message->bodyPtr(), message->bodySize()),
			[this, connection, request, message](boost::system::error_code error, std::size_t length) {
				if (!error) {
					request->addMessage(*message);
					if (request->isComplete()) {
						requestHandler(
							*request,
							[this, connection](std::shared_ptr<protocol::Response> response) {
								startWritingResponse(connection, response);
							}
						);
					} else {
						startReadingHeader(connection, request);
					}
				} else {
					LOG_ERROR(error);
				}
			}
		);
	}

	void startWritingResponse(std::shared_ptr<Connection> connection, std::shared_ptr<protocol::Response> response) {
		LOG_DEBUG(__PRETTY_FUNCTION__);

		{
			protocol::Header header = {
				.version = 1,
				.type = protocol::RequestType::STDOUT,
				.requestIdB0 = 1,
				.requestIdB1 = 0,
				.contentLengthB0 = static_cast<uint8_t>(response->stdout.size()),
				.contentLengthB1 = 0,
				.paddingLength = 0,
				.reserved = {}
			};
			connection->socket().send(boost::asio::buffer(&header, sizeof(header)));
			connection->socket().send(boost::asio::buffer(response->stdout.data(), response->stdout.size()));
		}

		{
			protocol::Header header = {
				.version = 1,
				.type = protocol::RequestType::STDERR,
				.requestIdB0 = 1,
				.requestIdB1 = 0,
				.contentLengthB0 = 0,
				.contentLengthB1 = 0,
				.paddingLength = 0,
				.reserved = {}
			};
			connection->socket().send(boost::asio::buffer(&header, sizeof(header)));
			connection->socket().send(boost::asio::buffer(response->stderr.data(), response->stderr.size()));
		}

		{
			protocol::EndRequestBody body = {};
			protocol::Header header = {
				.version = 1,
				.type = protocol::RequestType::END,
				.requestIdB0 = 1,
				.requestIdB1 = 0,
				.contentLengthB0 = sizeof(body),
				.contentLengthB1 = 0,
				.paddingLength = 0,
				.reserved = {}
			};
			connection->socket().send(boost::asio::buffer(&header, sizeof(header)));
			connection->socket().send(boost::asio::buffer(&body, sizeof(body)));
		}
	}


	RequestHandler requestHandler;
};


template<class RequestHandler> using Server = ::Server<ConnectionHandler<RequestHandler>>;

}

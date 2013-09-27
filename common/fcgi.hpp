#pragma once

#include <common/protocol_base.hpp>
#include <common/server.hpp>
#include <common/connection.hpp>

#include <map>
#include <string>



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

using Version = uint8_t;
using RequestId = uint16_t;
using ContentLength = uint16_t;
using PaddingLength = uint8_t;
using AppStatus = uint32_t;
using ProtocolStatus = uint8_t;


class Header {
public:
	Header(
		RequestType requestType = RequestType::BEGIN,
		RequestId requestId = 1,
		ContentLength contentLength = 0,
		PaddingLength paddingLength = 0
	) :
		requestType_(requestType),
		requestId_(htons(requestId)),
		contentLength_(htons(contentLength)),
		paddingLength_(paddingLength)
	{
	}

	Version version() const { return version_; }
	RequestType requestType() const { return requestType_; }
	RequestId requestId() const { return ntohs(requestId_); }
	ContentLength contentLength() const { return ntohs(contentLength_); }
	PaddingLength paddingLength() const { return paddingLength_; }

	size_t bodyLength() const { return contentLength() + paddingLength(); }

	void dprint() const {
		std::cout
 			<< "Header:" << std::endl
			<< "  version: " << (int)version() << std::endl
			<< "  requestType: " << (int)requestType() << std::endl
			<< "  requestId: " << (int)requestId() << std::endl
			<< "  contentLength: " << (int)contentLength() << std::endl
			<< "  paddingLength: " << (int)paddingLength() << std::endl;
	}

private:
	Version version_ = 1;
	RequestType requestType_;
	RequestId requestId_;
	ContentLength contentLength_;
	PaddingLength paddingLength_;
	[[gnu::unused]] uint8_t reserved_ = 0;
};


class EndRequestBody {
public:
	EndRequestBody(AppStatus appStatus = 0, ProtocolStatus protocolStatus = 0) : appStatus_(htonl(appStatus)), protocolStatus_(protocolStatus) {
	}

	AppStatus appStatus() const { return ntohl(appStatus_); }
	ProtocolStatus protocolStatus() const { return protocolStatus_; }

	void dprint() const {
		std::cout
 			<< "Body:" << std::endl
			<< "  appStatus: " << (int)appStatus() << std::endl
			<< "  protocolStatus: " << (int)protocolStatus() << std::endl;
	}

private:
	AppStatus appStatus_;
	ProtocolStatus protocolStatus_;
	[[gnu::unused]] unsigned char reserved_[3] = {0,0,0};
};




using Message = ::Message<Header>;


class Request {
public:

	void addMessage(const Message & message) {
		switch (message.header().requestType()) {
			case (RequestType::PARAMS): handleParamsMessage(message); break;
			case (RequestType::STDIN): handleStdinMessage(message); break;
			case (RequestType::BEGIN):
			case (RequestType::ABORT):
			case (RequestType::END):
			case (RequestType::STDOUT):
			case (RequestType::STDERR):
			case (RequestType::DATA):
			case (RequestType::GET_VALUES):
			case (RequestType::GET_VALUES_RESULT):
				handleUnsupportedMessage(message); break;
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
	void handleUnsupportedMessage(const Message & message) {
		LOG_WARNING(__FUNCTION__ << " (requestId=" << message.header().requestId() << ")");
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



static void sendStream(boost::asio::ip::tcp::socket & socket, RequestId requestId, const std::string buffer, RequestType requestType) {
	if (buffer.size() > 0xFFFF) throw std::runtime_error("message is too long");
	protocol::Header header(requestType, requestId, (ContentLength)buffer.size(), 0);
	socket.send(boost::asio::buffer(&header, sizeof(header)));
	socket.send(boost::asio::buffer(buffer.data(), buffer.size()));
}

static void sendStdout(boost::asio::ip::tcp::socket & socket, RequestId requestId, const std::string buffer) {
	sendStream(socket, requestId, buffer, RequestType::STDOUT);
}

static void sendStderr(boost::asio::ip::tcp::socket & socket, RequestId requestId, const std::string buffer) {
	sendStream(socket, requestId, buffer, RequestType::STDERR);
}

static void sendEnd(boost::asio::ip::tcp::socket & socket, RequestId requestId, AppStatus appStatus, ProtocolStatus protocolStatus) {
	protocol::EndRequestBody body(appStatus, protocolStatus);
	protocol::Header header(protocol::RequestType::END, requestId, sizeof(body), 0);
	socket.send(boost::asio::buffer(&header, sizeof(header)));
	socket.send(boost::asio::buffer(&body, sizeof(body)));
}

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
				if ((!error) || (length != message->headerSize())) {
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

		message->setBodySize(message->header().contentLength() + message->header().paddingLength());
		boost::asio::async_read(
			connection->socket(),
			boost::asio::buffer(message->bodyPtr(), message->bodySize()),
			[this, connection, request, message](boost::system::error_code error, std::size_t length) {
				if ((!error) || (length != message->bodySize())) {
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

		protocol::sendStdout(connection->socket(), 1, response->stdout);
		protocol::sendStderr(connection->socket(), 1, response->stderr);
		protocol::sendEnd(connection->socket(), 1, 0, 0);
	}


	RequestHandler requestHandler;
};


template<class RequestHandler> using Server = ::Server<ConnectionHandler<RequestHandler>>;

}

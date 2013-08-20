#pragma once

#include <common/daemon_base.hpp>


namespace Daemon {

class Fcgi : public Base {
public:
	struct Request {
		bool parametersRead = false;
		bool stdinRead = false;

		std::map<std::string, std::string> parameters;
		std::string stdin;
	};

	struct Response {
		std::string stdout;
		std::string stderr;
	};

private:
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

	struct BeginRequestBody {
		unsigned char roleB1;
		unsigned char roleB0;
		unsigned char flags;
		unsigned char reserved[5];

		uint16_t role() const { return static_cast<uint16_t>((roleB1 << 8) | roleB0); }

		void dprint() {
			std::cout
 				<< "BeginRequestBody:" << std::endl
				<< "  role: " << (int)role() << std::endl
				<< "  flags: " << (int)flags << std::endl;
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



	std::map<uint16_t, Request> requests;

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

public:
	using Base::Base;

	virtual void serve(const Request & request, Response & response) = 0;

	virtual void handleRequest(boost::asio::ip::tcp::socket && socket) override {
		std::cerr << __FUNCTION__ << std::endl;

		
		static constexpr size_t BUFFER_SIZE = 128*1024;
		std::clog << "allocating buffer ... ";
		std::unique_ptr<uint8_t[]> buffer(new uint8_t[BUFFER_SIZE]);
		std::clog << "OK" << std::endl;

		while (socket.is_open()) {
			Header header;
			std::clog << "reading header" << std::endl;
			boost::system::error_code error;
			boost::asio::read(socket, boost::asio::buffer(&header, sizeof(header)), error);
			if (error == boost::asio::error::eof) {
				std::cerr << "connection closed by nginx" << std::endl;
				socket.close();
				return;
			}

			header.dprint();

			std::clog << "reading content" << std::endl;
			boost::asio::read(socket, boost::asio::buffer(buffer.get(), header.contentLength() + header.paddingLength), error);
			if (error == boost::asio::error::eof) {
				std::cerr << "connection closed by nginx" << std::endl;
				socket.close();
				return;
			}

			switch (header.type) {
				case (REQUEST_TYPE_BEGIN): handleBeginRequest(socket, header, buffer.get()); break;
				case (REQUEST_TYPE_ABORT): handleAbortRequest(socket, header, buffer.get()); break;
				case (REQUEST_TYPE_PARAMS): handleParamsRequest(socket, header, buffer.get()); break;
				case (REQUEST_TYPE_STDIN): handleStdinRequest(socket, header, buffer.get()); break;
				case (REQUEST_TYPE_DATA): handleDataRequest(socket, header, buffer.get()); break;
				case (REQUEST_TYPE_GET_VALUES): handleGetValuesRequest(socket, header, buffer.get()); break;
				default: handleUnknownRequest(socket, header, buffer.get()); break;
			}
		}
	}

	void handleBeginRequest(boost::asio::ip::tcp::socket & socket, const Header & header, const uint8_t * buffer) {
		std::clog << __FUNCTION__ << " (requestId=" << header.requestId() << ")" << std::endl;
		auto it = requests.find(header.requestId());
		if (it != requests.end()) {
			std::cerr << "WARNING: rewriting requestId=" << header.requestId() << " in requests map" << std::endl;
		}
		requests[header.requestId()] = Request();
	}
	void handleAbortRequest(boost::asio::ip::tcp::socket & socket, const Header & header, const uint8_t * buffer) {
		std::clog << __FUNCTION__ << " (requestId=" << header.requestId() << ")" << std::endl;
		requests.erase(header.requestId());
	}
	void handleParamsRequest(boost::asio::ip::tcp::socket & socket, const Header & header, const uint8_t * buffer) {
		std::clog << __FUNCTION__ << " (requestId=" << header.requestId() << ")" << std::endl;
		Request & request = requests[header.requestId()];
		if (header.contentLength() == 0) {
			request.parametersRead = true;
			if (request.parametersRead && request.stdinRead) {
				completeRequest(socket, header.requestId());
			}
		} else {
			const unsigned char * p = buffer;
			while (p < buffer + header.contentLength()) {
				request.parameters.insert(readPair(p));
			}
			
			std::cout << "parameters: " << std::endl;
			for (const auto & pair : request.parameters) {
				std::cout << "  " << pair.first << " = " << pair.second << std::endl;
			}
		}
	}
	void handleStdinRequest(boost::asio::ip::tcp::socket & socket, const Header & header, const uint8_t * buffer) {
		std::clog << __FUNCTION__ << " (requestId=" << header.requestId() << ")" << std::endl;
		Request & request = requests[header.requestId()];
		if (header.contentLength() == 0) {
			request.stdinRead = true;
			if (request.parametersRead && request.stdinRead) {
				completeRequest(socket, header.requestId());
			}
		} else {
		}
	}
	void handleDataRequest(boost::asio::ip::tcp::socket & socket, const Header & header, const uint8_t * buffer) {
		std::clog << __FUNCTION__ << " (requestId=" << header.requestId() << ")" << std::endl;
	}
	void handleGetValuesRequest(boost::asio::ip::tcp::socket & socket, const Header & header, const uint8_t * buffer) {
		std::clog << __FUNCTION__ << " (requestId=" << header.requestId() << ")" << std::endl;
	}
	void handleUnknownRequest(boost::asio::ip::tcp::socket & socket, const Header & header, const uint8_t * buffer) {
		std::clog << __FUNCTION__ << " (requestId=" << header.requestId() << ")" << std::endl;
	}

	void completeRequest(boost::asio::ip::tcp::socket & socket, uint16_t requestId) {
		std::clog << __FUNCTION__ << " (requestId=" << requestId << ")" << std::endl;

		Request request = requests.at(requestId);
		requests.erase(requestId);
		
		Response response;
		serve(request, response);

		{
			Header header = {
				.version = 1,
				.type = 6,
				.requestIdB0 = 1,
				.requestIdB1 = 0,
				.contentLengthB0 = static_cast<uint8_t>(response.stdout.size()),
				.contentLengthB1 = 0,
				.paddingLength = 0
			};
			socket.send(boost::asio::buffer(&header, sizeof(header)));
			socket.send(boost::asio::buffer(response.stdout.data(), response.stdout.size()));
		}

		{
			Header header = {
				.version = 1,
				.type = 3,
				.requestIdB0 = 1,
				.requestIdB1 = 0,
				.contentLengthB0 = 0,
				.contentLengthB1 = 0,
				.paddingLength = 0
			};
			socket.send(boost::asio::buffer(&header, sizeof(header)));
		}

		{
			EndRequestBody body = {};
			Header header = {
				.version = 1,
				.type = 6,
				.requestIdB0 = 1,
				.requestIdB1 = 0,
				.contentLengthB0 = sizeof(body),
				.contentLengthB1 = 0,
				.paddingLength = 0
			};
			socket.send(boost::asio::buffer(&header, sizeof(header)));
			socket.send(boost::asio::buffer(&body, sizeof(body)));
		}

		socket.close();
	}
};

}

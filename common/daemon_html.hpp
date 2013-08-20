#pragma once

#include <common/daemon_fcgi.hpp>
#include <common/templater.hpp>
#include <common/log.hpp>


namespace Daemon {

class Html : public Fcgi {
protected:
	typedef std::string (Html::*UrlMethod)(const Request & request);
	std::map<std::string, UrlMethod> urlMethods;

public:
	using Fcgi::Fcgi;
	virtual void serve(const Request & request, Response & response) override {
		LOG_DEBUG(__PRETTY_FUNCTION__)
		auto itMethod = urlMethods.find(request.parameters.at("DOCUMENT_URI"));
		if (itMethod == urlMethods.end()) {
			LOG_WARNING("page " << request.parameters.at("DOCUMENT_URI") << " not found");
			response.stdout = "Status: 404 Not Found\r\n\r\n";
		} else {
			response.stdout = "Content-type: text/html\r\n\r\n" + (this->*(itMethod->second))(request);
		}
	}
};

}


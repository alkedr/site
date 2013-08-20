#pragma once

#include <common/protocol_base.hpp>


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

#pragma once

namespace Toshi {

	class TFileLexer
	{
	public:
		enum TokenType {
			TOKEN_UNKNOWN = 0,
			TOKEN_IDENT = 16,
			TOKEN_STRING = 17,
			TOKEN_INTEGER = 18,
			TOKEN_UINTEGER = 19,
			TOKEN_FLOAT = 20,
			TOKEN_COMMENT = 21,
		};

	};

}
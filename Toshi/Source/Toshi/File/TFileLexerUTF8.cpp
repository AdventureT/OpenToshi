#include "ToshiPCH.h"
#include "TFileLexerUTF8.h"
#include <Toshi/Core/TSystem.h>

namespace Toshi {

	TFileLexerUTF8::TFileLexerUTF8() :
		m_oEmitter(this)
	{
		m_pFile = 0;
		m_bOutputComments = TFALSE;
		m_iCharLookaheadSize = 0;
		m_iUnk3 = 0;
		m_piCharLookahead = TNULL;
		m_iLastLookaheadIndex = 0;
		m_iUnk5 = 0;
		m_iLine = 0;
		m_iTokenLookaheadSize = 1;
		m_iTokenLookaheadMask = 1;
		m_LookaheadTokens = LookaheadTokens::Allocate()->GetTokens();
		m_iTokenLookaheadBuffered = 0;
		m_iTokenLookaheadFront = 0;
		m_iTokenLookaheadBack = 0;
		m_iSomeNum = 0;
		m_bAllowPreprocessor = TTRUE;
		m_bUnk7 = TFALSE;

#if defined(TOSHI_SKU_WINDOWS)
		Define("TOSHI_SKU_WINDOWS");
#elif defined(TOSHI_SKU_PS2)
		Define("TOSHI_SKU_PS2");
#elif defined(TOSHI_SKU_GC)
		Define("TOSHI_SKU_GC");
#elif defined(TOSHI_SKU_WII)
		Define("TOSHI_SKU_WII");
#else
#error Wrong define?
#endif
	}

	TFileLexerUTF8::TFileLexerUTF8(TFile* a_pInputStream, int a_iTokenLookaheadSize) :
		m_oEmitter(this)
	{
		m_pFile = 0;
		m_bOutputComments = TFALSE;
		m_iCharLookaheadSize = 0;
		m_iUnk3 = 0;
		m_piCharLookahead = TNULL;
		m_iLastLookaheadIndex = 0;
		m_iUnk5 = 0;
		m_iLine = 0;
		m_iTokenLookaheadSize = 1;
		m_iTokenLookaheadMask = 1;
		m_LookaheadTokens = LookaheadTokens::Allocate(a_iTokenLookaheadSize)->GetTokens();
		m_iTokenLookaheadBuffered = 0;
		m_iTokenLookaheadFront = 0;
		m_iTokenLookaheadBack = 0;
		m_iSomeNum = 0;
		m_bAllowPreprocessor = TTRUE;
		m_bUnk7 = TFALSE;

		m_iTokenLookaheadSize = 1 << (((a_iTokenLookaheadSize * 2 + -1) >> 0x17) + 0x81U & 0x1f);
		m_iTokenLookaheadMask = (m_iTokenLookaheadSize - 1 < 2) ? 1 : m_iTokenLookaheadSize - 1;

		TASSERT(a_pInputStream != TNULL)
		SetInputStream(a_pInputStream);

#if defined(TOSHI_SKU_WINDOWS)
		Define("TOSHI_SKU_WINDOWS");
#elif defined(TOSHI_SKU_PS2)
		Define("TOSHI_SKU_PS2");
#elif defined(TOSHI_SKU_GC)
		Define("TOSHI_SKU_GC");
#elif defined(TOSHI_SKU_WII)
		Define("TOSHI_SKU_WII");
#else
#error Wrong define?
#endif
	}

	TFileLexerUTF8::~TFileLexerUTF8()
	{
		TIMPLEMENT();

		LookaheadTokens::Free(m_LookaheadTokens);
	}

	TFileLexerUTF8::Token TFileLexerUTF8::get_next_token()
	{
		int currentValue = peek();

		if (m_bUnk7)
		{
			return Token(TFileLexer::TOKEN_UNKNOWN, m_iLine);
		}

		skipWhiteSpace();

		if (currentValue == -1)
		{
			return Token(TFileLexer::TOKEN_UNKNOWN, m_iLine);
		}

		if (m_bOutputComments)
		{
			TIMPLEMENT();
			if (peek(0) == '/' && peek(1) == '/')
			{
				advance(2);
				for (size_t i = peek(); i != '\n' && i != '\r'; i = m_piCharLookahead[i])
				{

				}
			}
		}

		int len = 0;

		if ((iswalpha(currentValue) != 0) || currentValue == '_')
		{
			do
			{	
				ms_Buffer[len++] = currentValue;
				advance();
				TASSERT(len < WORDBUF_SIZE);
				currentValue = peek();
			} while ((iswalnum(currentValue) != 0) || currentValue == '_');
			ms_Buffer[len] = '\0';
			TCStringPool* pool = TSystem::GetCStringPool();
			TPCString poolString = pool->Get(ms_Buffer);
			return Token(TFileLexer::TOKEN_IDENT, m_iLine, poolString);
		}


		if ((iswdigit(currentValue) == 0) && currentValue != '-')
		{
			if (currentValue == '.')
			{
				if (iswdigit(peek(1)) != 0)
				{
					TIMPLEMENT();
				}
			}
			else if (currentValue == '"')
			{
				advance();
				currentValue = peek();
				do
				{
					TIMPLEMENT();
				} while (true);
			}

			switch (currentValue)
			{
			case '$':
				advance();
				return Token(TFileLexer::TOKEN_DOLLAR, m_iLine);
			case '(':
				advance();
				return Token(TFileLexer::TOKEN_OPENPAREN, m_iLine);
			case ')':
				advance();
				return Token(TFileLexer::TOKEN_CLOSEPAREN, m_iLine);
			case '*':
				advance();
				return Token(TFileLexer::TOKEN_ASTERISK, m_iLine);
			case ',':
				advance();
				return Token(TFileLexer::TOKEN_COMMA, m_iLine);
			case '.':
				advance();
				return Token(TFileLexer::TOKEN_DOT, m_iLine);
			case ':':
				advance();
				return Token(TFileLexer::TOKEN_COLON, m_iLine);
			case ';':
				advance();
				return Token(TFileLexer::TOKEN_SEMI, m_iLine);
			case '<':
				advance();
				return Token(TFileLexer::TOKEN_LESSTHAN, m_iLine);
			case '=':
				advance();
				return Token(TFileLexer::TOKEN_EQUAL, m_iLine);
			case '>':
				advance();
				return Token(TFileLexer::TOKEN_GREATERTHAN, m_iLine);
			case '[':
				advance();
				return Token(TFileLexer::TOKEN_OPENSQR, m_iLine);
			case ']':
				advance();
				return Token(TFileLexer::TOKEN_CLOSESQR, m_iLine);
			case '{':
				advance();
				return Token(TFileLexer::TOKEN_OPENBRACE, m_iLine);
			case '}':
				advance();
				return Token(TFileLexer::TOKEN_CLOSEBRACE, m_iLine);
			default:
				ThrowError("Invalid character");
				return Token();
			}
		}


		return Token();
	}

	void TFileLexerUTF8::skipWhiteSpace()
	{
		int curChar = peek();
		int i = 0;
		do
		{
			bool bNewLine = false;
			for (size_t i = iswspace(curChar); i != 0 || (curChar == ~0x100);)
			{
				advance();
				if (curChar == '\n')
				{
					m_iLine++;
					bNewLine = true;

				}
				else
				{
					bNewLine = false;
				}
				curChar = peek();
			}
			if (!m_bAllowPreprocessor)
			{
				while (curChar != '#' || (!bNewLine))
				{
					advance();
					if (curChar == '\n')
					{
						m_iLine++;
						bNewLine = true;

					}
					else
					{
						bNewLine = false;
					}
					curChar = peek();
				}
			}
			if (bNewLine && curChar == '#')
			{
				advance();
				int len = 0;
				while (iswspace(curChar) == 0 && curChar != -1)
				{
					ms_Buffer[len++] = curChar;
					advance();
					TASSERT(len < WORDBUF_SIZE);
					curChar = peek();
				}
			}
			if (m_bOutputComments) return;
			if (peek(0) != '/')
			{
				if (peek(1) != '/')
				{
					if (peek(0) == '/' && peek(1) == '*')
					{
						advance(2);
						do
						{
							if (peek(0) == '*')
							{
								if (peek(1) == '/')
								{
									//?
									break;
								}
							}
							if (peek(0) == -1)
							{
								m_oEmitter.Throw({ "Unterminated block comment /* ... */" , m_iLine });
								return;
							}
							if (peek() == '\n')
							{
								m_iLine++;
							}
							advance();
						} while (true);
					}
				}
			}
			if (peek(0) == '/' && peek(1) == '*')
			{
				advance(2);
				do
				{
					if (peek(0) == '*')
					{
						if (peek(1) == '/')
						{
							//?
							break;
						}
					}
					if (peek(0) == -1)
					{
						m_oEmitter.Throw({ "Unterminated block comment /* ... */" , m_iLine });
						return;
					}
					if (peek() == '\n')
					{
						m_iLine++;
					}
					advance();
				} while (true);
			}
			TIMPLEMENT();
			// Returing here cause we don't want a infinite loop
			return;
		} while (true);
		TIMPLEMENT();
	}

	void TFileLexerUTF8::fillLookAhead()
	{
		while (m_iUnk5 != m_iLastLookaheadIndex)
		{
			advance();
		}
	}

	void TFileLexerUTF8::advance()
	{
		m_iLastLookaheadIndex = m_iLastLookaheadIndex + 1 & m_iUnk3;
		m_piCharLookahead[m_iUnk5] = m_pFile->GetCChar();
		m_iUnk5 = m_iLastLookaheadIndex;
	}

	TFileLexerUTF8::Token TFileLexerUTF8::GetNextToken()
	{
		if (m_iTokenLookaheadBuffered < 1)
		{
			Token nextToken = get_next_token();
			m_Token.assign(nextToken);
		}
		else
		{
			m_Token.assign(*(m_LookaheadTokens + m_iTokenLookaheadFront * 3));
			m_iTokenLookaheadFront++;
			m_iTokenLookaheadFront &= m_iTokenLookaheadMask;
			m_iTokenLookaheadBuffered--;
		}
		return Token(m_Token);
	}

	TFileLexerUTF8::Token TFileLexerUTF8::PeekNextToken(int a_iDistance)
	{
		TASSERT(a_iDistance < m_iTokenLookaheadSize);
		TASSERT(((m_iTokenLookaheadFront + m_iTokenLookaheadBuffered) & m_iTokenLookaheadMask) == m_iTokenLookaheadBack);
		if (a_iDistance >= m_iTokenLookaheadBuffered)
		{
			for (;m_iTokenLookaheadBuffered <= a_iDistance; m_iTokenLookaheadBuffered++)
			{
				Token nextToken = get_next_token();
				m_LookaheadTokens[m_iTokenLookaheadBack].assign(nextToken);
				m_iTokenLookaheadBack = m_iTokenLookaheadBack + 1 & m_iTokenLookaheadMask;
			}
		}
		m_Token.assign(m_LookaheadTokens[m_iTokenLookaheadFront + a_iDistance & m_iTokenLookaheadMask]);
		return m_Token;
	}

	const char* TFileLexerUTF8::tostring(TFileLexer::TokenType a_eType)
	{
		switch (a_eType)
		{
		case Toshi::TFileLexer::TOKEN_UNKNOWN:
			return "EOF";
		case Toshi::TFileLexer::TOKEN_SEMI:
			return "SEMI";
		case Toshi::TFileLexer::TOKEN_COLON:
			return "COLON";
		case Toshi::TFileLexer::TOKEN_COMMA:
			return "COMMA";
		case Toshi::TFileLexer::TOKEN_DOT:
			return "DOT";
		case Toshi::TFileLexer::TOKEN_DOLLAR:
			return "DOLLAR";
		case Toshi::TFileLexer::TOKEN_OPENSQR:
			return "OPENSQR";
		case Toshi::TFileLexer::TOKEN_CLOSESQR:
			return "CLOSESQR";
		case Toshi::TFileLexer::TOKEN_OPENBRACE:
			return "OPENBRACE";
		case Toshi::TFileLexer::TOKEN_CLOSEBRACE:
			return "CLOSEBRACE";
		case Toshi::TFileLexer::TOKEN_OPENPAREN:
			return "OPENPAREN";
		case Toshi::TFileLexer::TOKEN_CLOSEPAREN:
			return "CLOSEPAREN";
		case Toshi::TFileLexer::TOKEN_LESSTHAN:
			return "LESSTHAN";
		case Toshi::TFileLexer::TOKEN_GREATERTHAN:
			return "GREATERTHAN";
		case Toshi::TFileLexer::TOKEN_EQUAL:
			return "EQUAL";
		default:
			return "?????";
		}
	}

	TBOOL TFileLexerUTF8::ComputePreprocessorAllow()
	{
		for (int i = 0; i < m_iSomeNum; i++)
		{
			bool bFlag;

			if (m_iSomeNum - i < 0)
				bFlag = m_bFlags[0];
			else
				bFlag = m_bFlags[m_iSomeNum - i];

			if (!bFlag)
				return TFALSE;
		}

		return TTRUE;
	}

	void TFileLexerUTF8::SetCharacterLookaheadSize(int a_iLookaheadSize)
	{
		TASSERT(m_pFile != TNULL);
		TASSERT(m_piCharLookahead == TNULL);

		int iIntCount = 1 << ((a_iLookaheadSize * 2 - 1) >> 0x17) + 0x81 & 0x1f;
		m_iCharLookaheadSize = iIntCount;
		m_iUnk3 = iIntCount - 1;
		m_iLastLookaheadIndex = 0;
		m_iUnk5 = 0;
		m_piCharLookahead = new int[iIntCount];

		for (int i = 0; i < iIntCount; i++)
			m_piCharLookahead[i] = m_pFile->GetCChar();
	}

	void TFileLexerUTF8::SetInputStream(TFile* a_pInputStream)
	{
		m_pFile = a_pInputStream;
		m_iLine = 1;
		m_iSomeNum = 0;
		m_bFlags[0] = TTRUE;
		m_bAllowPreprocessor = ComputePreprocessorAllow();
		SetCharacterLookaheadSize(3);
	}

	void TFileLexerUTF8::Define(const char* a_szDefinition)
	{
		TOSHI_ERROR("Has some problems with allocating strings. Skipping for now...");
		return;
		bool bAlreadyDefined = TFALSE;

		for (int i = 0; i < m_Defines.GetNumElements(); i++)
		{
			if (m_Defines[i] == a_szDefinition)
			{
				bAlreadyDefined = TTRUE;
				break;
			}
		}

		if (!bAlreadyDefined)
		{
			m_Defines.Push(a_szDefinition);
		}
	}

	void TFileLexerUTF8::Token::assign(const Token& token)
	{
		if (m_type == TFileLexer::TOKEN_IDENT || m_type == TFileLexer::TOKEN_STRING || m_type == TFileLexer::TOKEN_COMMENT)
		{
			if (token.m_type == TFileLexer::TOKEN_IDENT || token.m_type == TFileLexer::TOKEN_STRING || token.m_type == TFileLexer::TOKEN_COMMENT)
			{
				m_sValue = token.m_sValue;
			}
			else if (token.m_type == TFileLexer::TOKEN_INTEGER)
			{
				m_sValue.~TPCString();
				m_iValue = token.m_iValue;
			}
			else if (token.m_type == TFileLexer::TOKEN_UINTEGER)
			{
				m_sValue.~TPCString();
				m_uiValue = token.m_uiValue;
			}
			else if (token.m_type == TFileLexer::TOKEN_FLOAT)
			{
				m_sValue.~TPCString();
				m_fValue = token.m_fValue;
			}
		}
		else if (m_type == TFileLexer::TOKEN_INTEGER)
		{
			if (token.m_type == TFileLexer::TOKEN_IDENT || token.m_type == TFileLexer::TOKEN_STRING || token.m_type == TFileLexer::TOKEN_COMMENT)
			{
				m_sValue = token.m_sValue;
			}
			else if (token.m_type == TFileLexer::TOKEN_INTEGER)
			{
				m_iValue = token.m_iValue;
			}
			else if (token.m_type == TFileLexer::TOKEN_UINTEGER)
			{
				m_uiValue = token.m_uiValue;
			}
			else if (token.m_type == TFileLexer::TOKEN_FLOAT)
			{
				m_fValue = token.m_fValue;
			}
		}
		else if (m_type == TFileLexer::TOKEN_UINTEGER)
		{
			if (token.m_type == TFileLexer::TOKEN_IDENT || token.m_type == TFileLexer::TOKEN_STRING || token.m_type == TFileLexer::TOKEN_COMMENT)
			{
				m_sValue = token.m_sValue;
			}
			else if (token.m_type == TFileLexer::TOKEN_INTEGER)
			{
				m_iValue = token.m_iValue;
			}
			else if (token.m_type == TFileLexer::TOKEN_UINTEGER)
			{
				m_uiValue = token.m_uiValue;
			}
			else if (token.m_type == TFileLexer::TOKEN_FLOAT)
			{
				m_fValue = token.m_fValue;
			}
		}
		else if (m_type == TFileLexer::TOKEN_FLOAT)
		{
			if (token.m_type == TFileLexer::TOKEN_IDENT || token.m_type == TFileLexer::TOKEN_STRING || token.m_type == TFileLexer::TOKEN_COMMENT)
			{
				m_sValue = token.m_sValue;
			}
			else if (token.m_type == TFileLexer::TOKEN_INTEGER)
			{
				m_iValue = token.m_iValue;
			}
			else if (token.m_type == TFileLexer::TOKEN_UINTEGER)
			{
				m_uiValue = token.m_uiValue;
			}
			else if (token.m_type == TFileLexer::TOKEN_FLOAT)
			{
				m_fValue = token.m_fValue;
			}
		}
	}

	TCString TFileLexerUTF8::Token::tostring() const
	{
		TCString res = TCString();
		if (m_type == TFileLexer::TOKEN_IDENT)
		{
			res += "IDENT:";
			res += *GetString();
		}
		return TCString();
	}

}

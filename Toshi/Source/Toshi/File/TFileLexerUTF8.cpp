#include "ToshiPCH.h"
#include "TFileLexerUTF8.h"

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
		if (m_bUnk7)
		{
			return Token(TFileLexer::TOKEN_UNKNOWN, m_iLine);
		}

		skipWhiteSpace();

		if (m_piCharLookahead[m_iLastLookaheadIndex] == -1)
		{
			return Token(TFileLexer::TOKEN_UNKNOWN, m_iLine);
		}

		if (m_bOutputComments)
		{
			TIMPLEMENT();
			
		}

		return Token();
	}

	void TFileLexerUTF8::skipWhiteSpace()
	{
		TIMPLEMENT();
	}

	void TFileLexerUTF8::fillLookAhead()
	{
		while (m_iUnk5 != m_iLastLookaheadIndex)
		{
			m_piCharLookahead[m_iUnk5] = m_pFile->GetCChar();
			m_iUnk5++;
			m_iUnk5 &= m_iUnk3;
		}
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

	const char* TFileLexerUTF8::tostring(TFileLexer::TokenType a_eType)
	{
		switch (a_eType)
		{
		case Toshi::TFileLexer::TOKEN_UNKNOWN:
			return "EOF";
		case Toshi::TFileLexer::TOKEN_SEMICOLON:
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
		m_iUnk3 = iIntCount;
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
			else
			{
				// ~Token()?
				
			}
		}
		else if (m_type == TFileLexer::TOKEN_INTEGER)
		{
			m_iValue = token.m_iValue;
		}
		else if (m_type == TFileLexer::TOKEN_UINTEGER)
		{
			m_uiValue = token.m_uiValue;
		}
		else if (m_type == TFileLexer::TOKEN_FLOAT)
		{
			m_fValue = token.m_fValue;
		}
		TIMPLEMENT();
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

#pragma once
#include "TFileLexer.h"
#include "Toshi/File/TFile.h"
#include "Toshi/Core/TArray.h"
#include "Toshi/Strings/TPCString.h"

#define WORDBUF_SIZE 0x800

namespace Toshi {

	class TFileLexerUTF8 :
		public TGenericClassDerived<TFileLexerUTF8, TObject, "TFileLexerUTF8", TMAKEVERSION(1, 0), TFALSE>
	{
	public:
		struct Token
		{
		public:

			Token(TFileLexer::TokenType a_eType, int a_iLine) : m_type(a_eType), m_iLine(a_iLine)
			{
				TASSERT(m_type != TFileLexer::TOKEN_IDENT && m_type != TFileLexer::TOKEN_STRING && m_type != TFileLexer::TOKEN_INTEGER && m_type != TFileLexer::TOKEN_FLOAT && m_type != TFileLexer::TOKEN_COMMENT);
			}

			Token()
			{
				m_type = TFileLexer::TOKEN_UNKNOWN;
				m_iLine = 0;
			}

			Token(const Token& other)
			{
				m_type = TFileLexer::TOKEN_UNKNOWN;
				assign(other);
			}

			~Token()
			{
				if (m_type == TFileLexer::TOKEN_IDENT || m_type == TFileLexer::TOKEN_STRING || m_type == TFileLexer::TOKEN_COMMENT)
				{
					TPCString string = GetString();
					string.~TPCString();
				}
			}

			const TPCString& GetString() const
			{
				TASSERT(m_type == TFileLexer::TOKEN_IDENT || m_type == TFileLexer::TOKEN_STRING || m_type == TFileLexer::TOKEN_COMMENT);
				return m_sValue;
			}

			TPCString& GetString()
			{
				TASSERT(m_type == TFileLexer::TOKEN_IDENT || m_type == TFileLexer::TOKEN_STRING || m_type == TFileLexer::TOKEN_COMMENT);
				return m_sValue;
			}

			float GetFloat() const
			{
				TASSERT(m_type == TFileLexer::TOKEN_FLOAT);
				return m_uiValue;
			}

			int GetInteger() const
			{
				TASSERT(m_type == TFileLexer::TOKEN_INTEGER);
				return m_uiValue;
			}

			uint32_t GetUInteger() const
			{
				TASSERT(m_type == TFileLexer::TOKEN_UINTEGER);
				return m_uiValue;
			}

			TFileLexer::TokenType GetType() const { return m_type; }
			int GetLine() const { return m_iLine; }


			void assign(const Token& token);
			TCString tostring() const;

		public:
			TFileLexer::TokenType m_type = TFileLexer::TOKEN_UNKNOWN;
			int m_iLine = 0;
			union
			{
				float m_fValue;
				uint32_t m_uiValue;
				int m_iValue;
				TPCString m_sValue;
			};
		};

		struct LookaheadTokens
		{
		public:
			Token* GetTokens()
			{
				return TREINTERPRETCAST(Token*, this + 1);
			}

			static LookaheadTokens* Allocate(int a_iCount = 1)
			{
				return
					TREINTERPRETCAST(
						LookaheadTokens*,
						TMalloc(sizeof(LookaheadTokens) + sizeof(Token) * a_iCount)
					);
			}

			static void Free(Token* a_pHeadToken)
			{
				TFree(FromToken(a_pHeadToken));
			}

			static LookaheadTokens* FromToken(Token* a_pHeadToken)
			{
				return TREINTERPRETCAST(
					LookaheadTokens*,
					TREINTERPRETCAST(uintptr_t, a_pHeadToken) - sizeof(LookaheadTokens)
				);
			}

		public:
			int m_iCount;
		};

		struct ParseError
		{
			const char* m_szMessage;
			int m_iLine;

		public:
			ParseError& operator=(const ParseError& other)
			{
				m_szMessage = other.m_szMessage;
				m_iLine = other.m_iLine;
			}
		};

	protected:
		void ThrowError(const char* m_szError)
		{
			ParseError error = { m_szError, m_iLine };
			m_oEmitter.Throw(&error);
		}
		Token get_next_token();
		void skipWhiteSpace();
		void fillLookAhead();
		void advance();
		void advance(int a_dist)
		{
			TASSERT(a_dist < m_iCharLookaheadSize);
			m_iLastLookaheadIndex = a_dist + m_iLastLookaheadIndex & m_iUnk3;
			fillLookAhead();
		}
		int peek()
		{
			return m_piCharLookahead[m_iLastLookaheadIndex];
		}
		int peek(int a_dist)
		{
			TASSERT(a_dist < m_iCharLookaheadSize);
			return m_piCharLookahead[a_dist + m_iLastLookaheadIndex & m_iUnk3];
		}

	public:
		TFileLexerUTF8();
		TFileLexerUTF8(TFile* a_pInputStream, int a_iTokenLookaheadSize);
		~TFileLexerUTF8();

		Token GetNextToken();

		static const char* tostring(TFileLexer::TokenType a_eType);

		TBOOL ComputePreprocessorAllow();
		void SetCharacterLookaheadSize(int a_iLookaheadSize);
		void SetInputStream(TFile* a_pInputStream);
		void Define(const char* a_szDefinition);

		TEmitter<TFileLexerUTF8, ParseError>* GetParseErrorEmitter() { return &m_oEmitter; }
		void SetOutputComments(TBOOL a_bOutputComments) { m_bOutputComments = a_bOutputComments; }

	private:

		static inline char ms_Buffer[WORDBUF_SIZE];

		TFile* m_pFile;                                    // 0x04
		bool m_bOutputComments;                            // 0x08
		int m_iCharLookaheadSize;                          // 0x0C
		int m_iUnk3;                                       // 0x10
		int* m_piCharLookahead;                            // 0x14
		int m_iLastLookaheadIndex;                         // 0x18
		int m_iUnk5;                                       // 0x1C
		int m_iLine;                                       // 0x20
		int m_iTokenLookaheadSize;                         // 0x24
		int m_iTokenLookaheadMask;                         // 0x28
		Token m_Token;                                     // 0x2C
		Token* m_LookaheadTokens;                          // 0x38
		int m_iTokenLookaheadBuffered;                     // 0x3C
		int m_iTokenLookaheadFront;                        // 0x40
		int m_iTokenLookaheadBack;                         // 0x44
		int m_iSomeNum;                                    // 0x48
		TBOOL m_bFlags[4];                                 // 0x4C
		TBOOL m_bAllowPreprocessor;                        // 0x6C
		TBOOL m_bUnk7;                                     // 0x6D
		TArray<TCString>::Storage m_Defines;               // 0x70
		TEmitter<TFileLexerUTF8, ParseError> m_oEmitter;   // 0x80
	};

}
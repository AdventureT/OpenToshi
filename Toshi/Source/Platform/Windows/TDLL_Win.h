#pragma once

namespace Toshi {

	class TDLL
	{
	public:
		TDLL();
		TDLL(const TDLL& other);

		virtual ~TDLL();

		TBOOL Load(const TCString& a_sFileName);
		void Unload();

		void* GetAddress(const TCString& a_sSymbolName);

		const TCString& GetFileName() const
		{
			return m_sFileName;
		}

		HMODULE GetDLL() const
		{
			return m_pDLL;
		}

		TDLL& operator=(const TDLL& other)
		{
			m_sFileName = other.m_sFileName;
			m_pDLL = other.m_pDLL;
		}

	private:
		TCString m_sFileName;
		HMODULE m_pDLL;
	};
}
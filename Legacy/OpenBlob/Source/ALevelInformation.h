#include "Toshi/File/TTRB.h"
#include "Toshi/Core/THashTable.h"

class ALevelInformation : public Toshi::TTRB
    , public Toshi::TSingleton<ALevelInformation>
{
public:
	struct LevelProperties
	{
		TCHAR* m_levelName;
		TCHAR* levelNamePath;
		TCHAR* levelFileName;
		TCHAR  smth[0x58];
	};

	struct LevelHeader
	{
		TINT             m_levelCount;
		LevelProperties* m_pLevelProps;
	};

public:
	// $deBlob: FUNCTION 0056da50
	ALevelInformation()
	{
		m_pData = TNULL;
	}

	void         Create(const TCHAR* a_levelInfo);
	void         MakeStringToIDHashTable();
	LevelHeader* GetLevelHeader() { return reinterpret_cast<LevelHeader*>(m_pData); }
	const TCHAR* GetLevelName(TINT a_iLevelIndex);

public:
	static Toshi::THashTable::t_ItemCompareFunc LevelNameHashCompareFunc;

private:
	TUINT8*           m_pData;      // 0x14
	Toshi::THashTable m_pHashTable; // 0x18
};

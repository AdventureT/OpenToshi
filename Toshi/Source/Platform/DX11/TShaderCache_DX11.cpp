#include "ToshiPCH.h"
#include "TShaderCache_DX11.h"
#include "TRender_DX11.h"
#include "File/TFile.h"
#include "Platform/Windows/TNativeFile_Win.h"

#include <d3dcompiler.h>
#include <stdio.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// Returns TTRUE if a_szPath\hash.txt is missing or doesn't hold these hashes
static TBOOL CompareHashFile( const TCHAR* a_szPath, TUINT32 a_uiVSHash, TUINT32 a_uiPSHash )
{
	TCHAR szFileName[ 2048 ];
	sprintf( szFileName, "%s\\hash.txt", a_szPath );

	TBOOL bRecompile = TFALSE;
	FILE* pFile      = fopen( szFileName, "rb" );

	if ( pFile == TNULL )
		return TTRUE;

	TUINT32 uiVSHash;
	TUINT32 uiPSHash;
	fscanf( pFile, "0x%x\n", &uiVSHash );
	fscanf( pFile, "0x%x", &uiPSHash );

	if ( uiVSHash != a_uiVSHash || uiPSHash != a_uiPSHash )
		bRecompile = TTRUE;

	fclose( pFile );
	return bRecompile;
}

#ifdef DEBLOB_COMMUNITY_PATCH
// Returns the prefix of the read-only "gamedir" file system (-gamedir), or an empty string
static TString8 GetGameDirPrefix()
{
	TFileSystem* pGameDir = TFileManager::GetSingleton()->FindFileSystem( "gamedir" );
	return pGameDir ? pGameDir->GetPrefix() : TString8();
}
#endif // DEBLOB_COMMUNITY_PATCH

// $deBlob: FUNCTION 007072a0
void TShaderCache_DX11::Create( const TCHAR* a_szPath, const TCHAR* a_szVSSource, const TCHAR* a_szPSSource, const t_CompileVS& a_fnCompileVS, const t_CreateInputLayout& a_fnCreateInputLayout, const t_CompilePS& a_fnCompilePS )
{
	m_strPath = a_szPath;

	if ( m_strPath[ m_strPath.Length() - 1 ] != '\\' )
		m_strPath += TString8( "\\" );

	TUINT32 uiVSHash = HashString( a_szVSSource );
	TUINT32 uiPSHash = HashString( a_szPSSource );

	TString8 strListPath = m_strPath;
	TBOOL    bRecompile  = NeedsRecompiling( a_szPath, uiVSHash, uiPSHash );

#ifdef DEBLOB_COMMUNITY_PATCH
	// NOTE: retail reads hash.txt and lists the cache folder relative to the working directory, which is the game
	// folder. With -gamedir (dev runs from another folder) the cache is read from the game folder instead
	if ( bRecompile )
	{
		TString8 strGameDir = GetGameDirPrefix();

		if ( strGameDir.Length() != 0 )
		{
			TString8 strGamePath = strGameDir + a_szPath;

			if ( !CompareHashFile( strGamePath.GetString(), uiVSHash, uiPSHash ) )
			{
				strListPath = strGameDir + m_strPath;
				bRecompile  = TFALSE;
			}
		}
	}
#endif // DEBLOB_COMMUNITY_PATCH

	if ( !bRecompile )
	{
		// The cached permutations are up to date: load them
		TNativeFileSystem* pFileSystem = new TNativeFileSystem( "SkinShaders" );
		TString8           strFileName;

		if ( pFileSystem->GetFirstFile( strListPath, strFileName, 0xFF ) )
		{
			ID3D11Device* pDevice = TRenderDX11::Interface()->GetDevice();

			do
			{
				if ( strFileName != "." && strFileName != ".." )
				{
					if ( strFileName.Find( ".vs", 0 ) > 0 )
					{
						VertexShaderId oId = { 0, 0, 0 };
						sscanf( strFileName.GetString(), "%u_%u_%u.vs", &oId.uiFlags, &oId.uiTexGen23, &oId.uiTexGen4 );

						ID3D11VertexShader* pShader = TNULL;
						TFile*              pFile   = TFile::Create( m_strPath + strFileName, TFILEMODE_READ );
						TASSERT( pFile != TNULL );

						if ( pFile != TNULL )
						{
							ID3DBlob* pBlob  = TNULL;
							TSIZE     uiSize = pFile->GetSize();
							D3DCreateBlob( uiSize, &pBlob );
							pFile->Read( pBlob->GetBufferPointer(), uiSize );

							HRESULT hr = pDevice->CreateVertexShader( pBlob->GetBufferPointer(), uiSize, TNULL, &pShader );
							TASSERT( hr == S_OK );

							if ( hr == S_OK )
							{
								ID3DBlob*          pBlobArg     = pBlob;
								ID3D11InputLayout* pInputLayout = a_fnCreateInputLayout( oId, pBlobArg );
								AddVertexShader( oId, pShader, pInputLayout, TNULL, 0 );
							}

							pBlob->Release();
						}

						TFile::Destroy( pFile );
					}
					else if ( strFileName.Find( ".ps", 0 ) > 0 )
					{
						// NOTE: retail scans each %d straight into the bytes of the id (each write spills into the next
						// bytes, which the next one overwrites); the result is the same
						TINT          aStages[ 5 ] = { -1, -1, -1, -1, -1 };
						PixelShaderId oId          = {};
						oId.uiFlags                = 0;
						sscanf( strFileName.GetString(), "%d_%d_%d_%d_%d_%u.ps", &aStages[ 0 ], &aStages[ 1 ], &aStages[ 2 ], &aStages[ 3 ], &aStages[ 4 ], &oId.uiFlags );

						for ( TINT i = 0; i < 5; i++ )
							oId.aStages[ i ] = TINT8( aStages[ i ] );

						ID3D11PixelShader* pShader = TNULL;
						TFile*             pFile   = TFile::Create( m_strPath + strFileName, TFILEMODE_READ );
						TASSERT( pFile != TNULL );

						if ( pFile != TNULL )
						{
							ID3DBlob* pBlob  = TNULL;
							TSIZE     uiSize = pFile->GetSize();
							D3DCreateBlob( uiSize, &pBlob );
							pFile->Read( pBlob->GetBufferPointer(), uiSize );

							HRESULT hr = pDevice->CreatePixelShader( pBlob->GetBufferPointer(), uiSize, TNULL, &pShader );
							TASSERT( hr == S_OK );

							if ( hr == S_OK )
								AddPixelShader( oId, pShader, TNULL, 0 );

							pBlob->Release();
						}

						TFile::Destroy( pFile );
					}
				}
			} while ( pFileSystem->GetNextFile( strFileName, 0xFF ) );
		}

		delete pFileSystem;
	}
	else
	{
		// The sources changed: recompile every permutation listed in the folder and save the new files
		TRenderDX11*     pRender = TRenderDX11::Interface();
		TCHAR            szFilter[ 1024 ];
		WIN32_FIND_DATAA oFindData;

		T2String8::Format( szFilter, sizeof( szFilter ), "%s/*.vs", a_szPath );
		HANDLE hFind = FindFirstFileA( szFilter, &oFindData );

		if ( hFind != INVALID_HANDLE_VALUE )
		{
			do
			{
				if ( oFindData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE )
				{
					VertexShaderId oId = { 0, 0, 0 };
					sscanf( oFindData.cFileName, "%u_%u_%u.vs", &oId.uiFlags, &oId.uiTexGen23, &oId.uiTexGen4 );

					ID3D11VertexShader* pShader = TNULL;
					printf( "Recompiling %s\n", oFindData.cFileName );

					ID3DBlob*          pBlob        = a_fnCompileVS( oId );
					ID3D11InputLayout* pInputLayout = a_fnCreateInputLayout( oId, pBlob );

					HRESULT hr = pRender->CreateVertexShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &pShader );
					TASSERT( hr == S_OK );

					AddVertexShader( oId, pShader, pInputLayout, pBlob->GetBufferPointer(), pBlob->GetBufferSize() );
					pBlob->Release();
				}
			} while ( FindNextFileA( hFind, &oFindData ) );
		}

		// NOTE: retail closes the handle even when FindFirstFileA failed
		FindClose( hFind );

		T2String8::Format( szFilter, sizeof( szFilter ), "%s/*.ps", a_szPath );
		hFind = FindFirstFileA( szFilter, &oFindData );

		if ( hFind != INVALID_HANDLE_VALUE )
		{
			do
			{
				if ( oFindData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE )
				{
					TINT          aStages[ 5 ] = { -1, -1, -1, -1, -1 };
					PixelShaderId oId          = {};
					oId.uiFlags                = 0;
					sscanf( oFindData.cFileName, "%d_%d_%d_%d_%d_%u.ps", &aStages[ 0 ], &aStages[ 1 ], &aStages[ 2 ], &aStages[ 3 ], &aStages[ 4 ], &oId.uiFlags );

					for ( TINT i = 0; i < 5; i++ )
						oId.aStages[ i ] = TINT8( aStages[ i ] );

					ID3DBlob* pBlob = a_fnCompilePS( oId );
					printf( "Recompiling %s\n", oFindData.cFileName );

					ID3D11PixelShader* pShader = TNULL;
					HRESULT            hr      = pRender->CreatePixelShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &pShader );
					TASSERT( hr == S_OK );

					AddPixelShader( oId, pShader, pBlob->GetBufferPointer(), pBlob->GetBufferSize() );
					pBlob->Release();
				}
			} while ( FindNextFileA( hFind, &oFindData ) );
		}

		FindClose( hFind );
		SaveHash( a_szPath, uiVSHash, uiPSHash );
	}
}

// $deBlob: FUNCTION 00707b40
TBOOL TShaderCache_DX11::FindPixelShader( const PixelShaderId& a_rId, ID3D11PixelShader*& a_rOutShader )
{
	auto it = m_PixelShaders.find( a_rId );

	if ( it != m_PixelShaders.end() )
	{
		a_rOutShader = it->second;
		return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 00707b70
void TShaderCache_DX11::AddPixelShader( const PixelShaderId& a_rId, ID3D11PixelShader* a_pShader, const void* a_pBytecode, SIZE_T a_uiBytecodeLength )
{
	m_PixelShaders.insert( std::make_pair( a_rId, a_pShader ) );

	if ( a_pBytecode != TNULL )
	{
		// Save the compiled permutation in the cache folder
		TCHAR szFileName[ 256 ];
		T2String8::Format( szFileName, sizeof( szFileName ), "%d_%d_%d_%d_%d_%u.ps", a_rId.aStages[ 0 ], a_rId.aStages[ 1 ], a_rId.aStages[ 2 ], a_rId.aStages[ 3 ], a_rId.aStages[ 4 ], a_rId.uiFlags );

		TFile* pFile = TFile::Create( m_strPath + TString8( szFileName ), TFILEMODE_WRITE | TFILEMODE_CREATENEW );

		if ( pFile != TNULL )
		{
			pFile->Write( a_pBytecode, a_uiBytecodeLength );
			TFile::Destroy( pFile );
		}
	}
}

// $deBlob: FUNCTION 00707c60
TBOOL TShaderCache_DX11::FindVertexShader( const VertexShaderId& a_rId, ID3D11VertexShader*& a_rOutShader, ID3D11InputLayout*& a_rOutInputLayout )
{
	auto it = m_VertexShaders.find( a_rId );

	if ( it != m_VertexShaders.end() )
	{
		a_rOutShader      = it->second.pShader;
		a_rOutInputLayout = it->second.pInputLayout;
		return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 00707cd0
void TShaderCache_DX11::AddVertexShader( const VertexShaderId& a_rId, ID3D11VertexShader* a_pShader, ID3D11InputLayout* a_pInputLayout, const void* a_pBytecode, SIZE_T a_uiBytecodeLength )
{
	VertexShaderEntry oEntry;
	oEntry.pShader      = a_pShader;
	oEntry.pInputLayout = a_pInputLayout;
	m_VertexShaders.insert( std::make_pair( a_rId, oEntry ) );

	if ( a_pBytecode != TNULL )
	{
		// Save the compiled permutation in the cache folder
		TCHAR szFileName[ 256 ];
		T2String8::Format( szFileName, sizeof( szFileName ), "%u_%u_%u.vs", a_rId.uiFlags, a_rId.uiTexGen23, a_rId.uiTexGen4 );

		TFile* pFile = TFile::Create( m_strPath + TString8( szFileName ), TFILEMODE_WRITE | TFILEMODE_CREATENEW );

		if ( pFile != TNULL )
		{
			pFile->Write( a_pBytecode, a_uiBytecodeLength );
			TFile::Destroy( pFile );
		}
	}
}

// $deBlob: FUNCTION 00707dc0
TBOOL TShaderCache_DX11::NeedsRecompiling( const TCHAR* a_szPath, TUINT32 a_uiVSHash, TUINT32 a_uiPSHash )
{
	CreateDirectories( a_szPath );
	return CompareHashFile( a_szPath, a_uiVSHash, a_uiPSHash );
}

// $deBlob: FUNCTION 00707e80
void TShaderCache_DX11::SaveHash( const TCHAR* a_szPath, TUINT32 a_uiVSHash, TUINT32 a_uiPSHash )
{
	TCHAR szFileName[ 2048 ];
	sprintf( szFileName, "%s\\hash.txt", a_szPath );

	FILE* pFile = fopen( szFileName, "wb" );

	if ( pFile != TNULL )
	{
		fprintf( pFile, "0x%x\n", a_uiVSHash );
		fprintf( pFile, "0x%x\n", a_uiPSHash );
		fclose( pFile );
	}
}

// $deBlob: FUNCTION 00707f00
void TShaderCache_DX11::CreateDirectories( const TCHAR* a_szPath )
{
	TCHAR       szDirectory[ 2048 ];
	const TCHAR* pSeparator = strpbrk( a_szPath, "\\/" );

	while ( pSeparator != TNULL )
	{
		TINT iCount = TINT( pSeparator - a_szPath );
		strncpy( szDirectory, a_szPath, iCount );
		TASSERT( iCount < TINT( sizeof( szDirectory ) ) );
		szDirectory[ iCount ] = '\0';

		CreateDirectoryA( szDirectory, TNULL );
		pSeparator = strpbrk( pSeparator + 1, "\\/" );
	}

	CreateDirectoryA( a_szPath, TNULL );
}

// $deBlob: FUNCTION 00707fa0
void TShaderCache_DX11::DeleteFiles( const TCHAR* a_szPath )
{
	TCHAR            szFilter[ 2048 ];
	TCHAR            szFileName[ 2048 ];
	WIN32_FIND_DATAA oFindData;

	T2String8::Format( szFilter, sizeof( szFilter ), "%s\\*.*", a_szPath );
	HANDLE hFind = FindFirstFileA( szFilter, &oFindData );

	if ( hFind != INVALID_HANDLE_VALUE )
	{
		do
		{
			if ( oFindData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE )
			{
				T2String8::Format( szFileName, sizeof( szFileName ), "%s\\%s", a_szPath, oFindData.cFileName );
				DeleteFileA( szFileName );
			}
		} while ( FindNextFileA( hFind, &oFindData ) );
	}
}

// $deBlob: FUNCTION 00708060
TUINT32 TShaderCache_DX11::HashString( const TCHAR* a_szString )
{
	// djb2 (the characters are signed)
	TUINT32 uiHash = 5381;

	TINT iChar = *a_szString;

	while ( iChar != 0 )
	{
		a_szString++;
		uiHash = ( uiHash << 5 ) + iChar + uiHash;
		iChar  = *a_szString;
	}

	return uiHash;
}

TOSHI_NAMESPACE_END

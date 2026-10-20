#pragma once
#include "IO/AIOEntityComponent.h"

//-----------------------------------------------------------------------------
// Purpose: a movie of an IO entity, its component of type 3 (AIOMovieComponent.cpp): the action 0x25 pushes an
// AMovieState playing the data's movie.
//-----------------------------------------------------------------------------
class AIOMovieComponent
    : public AIOEntityComponent
{
public:
	static TCONSTEXPR_VAR TINT TYPE = 3;

	static TCONSTEXPR_VAR TINT ACTION_PlayMovie = 0x25;

	struct IOEntityData
	{
		TINT          iType; // 0x00 TYPE
		const TCHAR** pData; // 0x04 the movie
	};

public:
	AIOMovieComponent( AIOEntity* a_pOwner );
	virtual ~AIOMovieComponent() override; // +0x00

	// $deBlob: FUNCTION 007cc800
	virtual void Tick( TFLOAT a_fUnk ) override {} // +0x08

	// SetIOEntityData: a_pUnk1 is the component's IOEntityData
	virtual void SetIOEntityData( void* a_pData, Toshi::TTRB* a_pTRB ) override; // +0x0C

	// ExecuteAction: ACTION_PlayMovie
	virtual TBOOL ExecuteActionInvoker( TINT* a_pMessage, AIOEntity* a_pEntity, ABaseEntity* a_pOther ) override; // +0x10

private:
	const TCHAR* m_szMovie; // 0x14
};                          // 0x18

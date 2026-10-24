#pragma once

class ABaseEntity;

namespace Toshi
{
class TClass;
}

//-----------------------------------------------------------------------------
// Purpose: the functor AEntityHash's queries call on every candidate entity (its operator() in vtable slot 0). A query
// returns the entity only if the filter accepts it.
// NOTE: retail's functor object is opaque here (its vtable slot 0 is the call); the queries ported so far pass none.
//-----------------------------------------------------------------------------
class AEntityHashFilter
{
public:
	virtual ~AEntityHashFilter() = default;

	// +0x00
	virtual TBOOL operator()( ABaseEntity* a_pEntity ) = 0;
};

//-----------------------------------------------------------------------------
// Purpose: accepts the entities of one class (not the derived ones).
//-----------------------------------------------------------------------------
class AEntityHashFilterOnlyThisType
    : public AEntityHashFilter
{
public:
	AEntityHashFilterOnlyThisType( const Toshi::TClass* a_pClass )
	    : m_pClass( a_pClass )
	{}

	virtual TBOOL operator()( ABaseEntity* a_pEntity ) override;

private:
	const Toshi::TClass* m_pClass; // 0x04
};

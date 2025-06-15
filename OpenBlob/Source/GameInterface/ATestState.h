#pragma once
#include "GameInterface/AGameState.h"
#include <Toshi2/T2DynamicVector.h>

class ATestState : public Toshi::TGenericClassDerived<ATestState, AGameState, "ATestState", TMAKEVERSION(1, 0), TTRUE>
{
public:
	ATestState()
	    : m_Elements(Toshi::GetGlobalAllocator(), 1, 4)
	{
	}

	virtual UpdateResult OnUpdate(TFLOAT deltaTime) override;
	virtual void         OnRemoval() override;
	virtual void         OnInsertion() override;
	virtual void         OnDeactivate() override;
	virtual void         OnActivate() override;

#ifdef TOSHI_DEBUG
	virtual void DEBUG_RenderImGui() override;
#endif

private:
	Toshi::T2GUIRectangle                          m_Menu;
	Toshi::T2DynamicVector<Toshi::T2GUIRectangle*> m_Elements;
};

#include "ToshiPCH.h"
#include "T2RedBlackTree.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 0053f900
// $deBlob: FUNCTION 00859490
TCONSTINIT T2GenericRedBlackTreeNode T2GenericRedBlackTree::ms_oNil( T2GenericRedBlackTreeNode::NIL );

// $deBlob: FUNCTION 006f21f0
T2GenericRedBlackTree::~T2GenericRedBlackTree()
{
	TASSERT( m_iNumElements == 0 );
}

// $deBlob: FUNCTION 006f2ac0
void T2GenericRedBlackTree::CheckValid()
{
	TASSERT( ms_oNil.red == 0 );
	TASSERT( m_oRoot.red == 0 );
}

// $deBlob: FUNCTION 006f2a60
// $Barnyard: FUNCTION 006cf1e0
T2GenericRedBlackTreeNode* T2GenericRedBlackTree::GetFirstNode() const
{
	T2GenericRedBlackTreeNode* pCurrentNode = &m_oRoot;
	T2GenericRedBlackTreeNode* pResult      = TNULL;

	while ( pCurrentNode != &ms_oNil )
	{
		pResult      = pCurrentNode;
		pCurrentNode = pResult->m_pLeft;
	}

	return pResult;
}

// $deBlob: FUNCTION 006f23c0
// $Barnyard: FUNCTION 006ceff0
T2GenericRedBlackTreeNode* T2GenericRedBlackTree::Insert( T2GenericRedBlackTreeNode* pNode )
{
	T2GenericRedBlackTreeNode* pUncle;
	T2GenericRedBlackTreeNode* pCurrent = pNode;

	m_iNumElements += 1;
	pNode->red = 1;

	while ( pCurrent->m_pParent->red )
	{
		if ( pCurrent->m_pParent == pCurrent->m_pParent->m_pParent->m_pLeft )
		{
			pUncle = pCurrent->m_pParent->m_pParent->m_pRight;

			if ( pUncle->red )
			{
				pCurrent->m_pParent->red                  = 0;
				pUncle->red                               = 0;
				pCurrent->m_pParent->m_pParent->red = 1;
				pCurrent                                  = pCurrent->m_pParent->m_pParent;
			}
			else
			{
				if ( pCurrent == pCurrent->m_pParent->m_pRight )
				{
					pCurrent = pCurrent->m_pParent;
					LeftRotate( pCurrent );
				}

				pCurrent->m_pParent->red                  = 0;
				pCurrent->m_pParent->m_pParent->red = 1;
				RightRotate( pCurrent->m_pParent->m_pParent );
			}
		}
		else
		{
			pUncle = pCurrent->m_pParent->m_pParent->m_pLeft;

			if ( pUncle->red )
			{
				pCurrent->m_pParent->red                  = 0;
				pUncle->red                               = 0;
				pCurrent->m_pParent->m_pParent->red = 1;
				pCurrent                                  = pCurrent->m_pParent->m_pParent;
			}
			else
			{
				if ( pCurrent == pCurrent->m_pParent->m_pLeft )
				{
					pCurrent = pCurrent->m_pParent;
					RightRotate( pCurrent );
				}

				pCurrent->m_pParent->red                  = 0;
				pCurrent->m_pParent->m_pParent->red = 1;
				LeftRotate( pCurrent->m_pParent->m_pParent );
			}
		}
	}

	m_oRoot.m_pLeft->red = 0;
	TASSERT( 0 == ms_oNil.red, "nil not red in T2GenericRedBlackTree::Insert" );
	TASSERT( 0 == m_oRoot.red, "root not red in T2GenericRedBlackTree::Insert" );

	return pNode;
}

// $deBlob: FUNCTION 006f2710
// $Barnyard: FUNCTION 006cf200
void T2GenericRedBlackTree::DeleteFixUp( T2GenericRedBlackTreeNode* pNode )
{
	T2GenericRedBlackTreeNode* pRoot = m_oRoot.m_pLeft;
	T2GenericRedBlackTreeNode* pSibling;

	while ( !pNode->red && pRoot != pNode )
	{
		if ( pNode == pNode->m_pParent->m_pLeft )
		{
			pSibling = pNode->m_pParent->m_pRight;

			if ( pSibling->red )
			{
				pSibling->red         = 0;
				pNode->m_pParent->red = 1;
				LeftRotate( pNode->m_pParent );
				pSibling = pNode->m_pParent->m_pRight;
			}

			if ( !pSibling->m_pRight->red && !pSibling->m_pLeft->red )
			{
				pSibling->red = 1;
				pNode         = pNode->m_pParent;
			}
			else
			{
				if ( !pSibling->m_pRight->red )
				{
					pSibling->m_pLeft->red = 0;
					pSibling->red          = 1;
					RightRotate( pSibling );
					pSibling = pNode->m_pParent->m_pRight;
				}

				pSibling->red           = pNode->m_pParent->red;
				pNode->m_pParent->red   = 0;
				pSibling->m_pRight->red = 0;
				LeftRotate( pNode->m_pParent );
				pNode = pRoot;
			}
		}
		else
		{
			pSibling = pNode->m_pParent->m_pLeft;

			if ( pSibling->red )
			{
				pSibling->red         = 0;
				pNode->m_pParent->red = 1;
				RightRotate( pNode->m_pParent );
				pSibling = pNode->m_pParent->m_pLeft;
			}

			if ( !pSibling->m_pRight->red && !pSibling->m_pLeft->red )
			{
				pSibling->red = 1;
				pNode         = pNode->m_pParent;
			}
			else
			{
				if ( !pSibling->m_pLeft->red )
				{
					pSibling->m_pRight->red = 0;
					pSibling->red           = 1;
					LeftRotate( pSibling );
					pSibling = pNode->m_pParent->m_pLeft;
				}

				pSibling->red          = pNode->m_pParent->red;
				pNode->m_pParent->red  = 0;
				pSibling->m_pLeft->red = 0;
				RightRotate( pNode->m_pParent );
				pNode = pRoot;
			}
		}
	}

	pNode->red = 0;
	CheckValid();
	TASSERT( 0 == ms_oNil.red, "nil not red in T2GenericRedBlackTree::DeleteFixUp" );
}

// $deBlob: FUNCTION 006f2230
// $Barnyard: FUNCTION 006cf460
T2GenericRedBlackTreeNode* T2GenericRedBlackTree::DeleteNode( T2GenericRedBlackTreeNode* pNode )
{
	T2GenericRedBlackTreeNode* pTVar1;
	T2GenericRedBlackTreeNode* y;
	T2GenericRedBlackTreeNode* pTVar3;

	y = pNode;
	if ( ( pNode->m_pLeft != &ms_oNil ) && ( pNode->m_pRight != &ms_oNil ) )
	{
		y = GetSuccessorOf( pNode );
	}

	pTVar3 = y->m_pLeft;
	if ( pTVar3 == &ms_oNil )
	{
		pTVar3 = y->m_pRight;
	}

	pTVar1 = y->m_pParent;
	pTVar3->m_pParent = pTVar1;

	if ( &m_oRoot == pTVar1 )
	{
		m_oRoot.m_pLeft = pTVar3;
	}
	else
	{
		pTVar1 = y->m_pParent;

		if ( y == pTVar1->m_pLeft )
		{
			pTVar1->m_pLeft = pTVar3;
		}
		else
		{
			pTVar1->m_pRight = pTVar3;
		}
	}

	if ( y == pNode )
	{
		if ( y->red == 0 )
		{
			DeleteFixUp( pTVar3 );
		}

#ifdef DEBLOB_COMMUNITY_PATCH
		// NOTE: when the removed node's child is ms_oNil, DeleteNode/DeleteFixUp write the shared nil's parent (and
		// can leave it red); reset it so other trees don't see a stale nil
		ms_oNil.red       = 0;
		ms_oNil.m_pParent = &ms_oNil;
#endif

		CheckValid();
		TASSERT( 0 == ms_oNil.red, "nil not red in T2GenericRedBlackTree::Delete" );
		m_iNumElements -= 1;

		return y;
	}

	TASSERT( ( y != &ms_oNil ), "y is nil in DeleteNode" );

	y->m_pLeft                 = pNode->m_pLeft;
	y->m_pRight                = pNode->m_pRight;
	y->m_pParent               = pNode->m_pParent;
	pNode->m_pRight->m_pParent = y;
	pNode->m_pLeft->m_pParent  = y;

	pTVar1 = pNode->m_pParent;

	if ( pNode == pTVar1->m_pLeft )
	{
		pTVar1->m_pLeft = y;
	}
	else
	{
		pTVar1->m_pRight = y;
	}

	TINT oldRedValue = y->red;
	y->red           = pNode->red;

	if ( oldRedValue == 0 )
	{
		DeleteFixUp( pTVar3 );
	}

#ifdef DEBLOB_COMMUNITY_PATCH
	// NOTE: when the removed node's child is ms_oNil, DeleteNode/DeleteFixUp write the shared nil's parent (and
	// can leave it red); reset it so other trees don't see a stale nil
	ms_oNil.red       = 0;
	ms_oNil.m_pParent = &ms_oNil;
#endif

	CheckValid();
	TASSERT( 0 == ms_oNil.red, "nil not red in T2GenericRedBlackTree::Delete" );
	m_iNumElements -= 1;
	return pNode;
}

// $deBlob: FUNCTION 006f2610
void T2GenericRedBlackTree::LeftRotate( T2GenericRedBlackTreeNode* pNode )
{
	T2GenericRedBlackTreeNode* pTVar1;
	T2GenericRedBlackTreeNode* pTVar2;

	pTVar2          = pNode->m_pRight;
	pNode->m_pRight = pTVar2->m_pLeft;

	if ( pTVar2->m_pLeft != &ms_oNil )
	{
		pTVar2->m_pLeft->m_pParent = pNode;
	}

	pTVar2->m_pParent = pNode->m_pParent;
	pTVar1            = pNode->m_pParent;

	if ( pNode == pTVar1->m_pLeft )
	{
		pTVar1->m_pLeft = pTVar2;
	}
	else
	{
		pTVar1->m_pRight = pTVar2;
	}

	pTVar2->m_pLeft  = pNode;
	pNode->m_pParent = pTVar2;
	CheckValid();
	TASSERT( 0 == ms_oNil.red, "nil not red in T2GenericRedBlackTree::LeftRotate" );
}

// $deBlob: FUNCTION 006f2690
void T2GenericRedBlackTree::RightRotate( T2GenericRedBlackTreeNode* pNode )
{
	T2GenericRedBlackTreeNode* pTVar1;
	T2GenericRedBlackTreeNode* pTVar2;

	pTVar2         = pNode->m_pLeft;
	pNode->m_pLeft = pTVar2->m_pRight;

	if ( pTVar2->m_pRight != &ms_oNil )
	{
		pTVar2->m_pRight->m_pParent = pNode;
	}

	pTVar2->m_pParent = pNode->m_pParent;
	pTVar1            = pNode->m_pParent;

	if ( pNode == pTVar1->m_pLeft )
	{
		pTVar1->m_pLeft = pTVar2;
	}
	else
	{
		pTVar1->m_pRight = pTVar2;
	}

	pTVar2->m_pRight = pNode;
	pNode->m_pParent = pTVar2;
	CheckValid();
	TASSERT( 0 == ms_oNil.red, "nil not red in T2GenericRedBlackTree::RightRotate" );
}

// $deBlob: FUNCTION 006f25b0
// $Barnyard: FUNCTION 006cf190
T2GenericRedBlackTreeNode* T2GenericRedBlackTree::GetSuccessorOf( const T2GenericRedBlackTreeNode* pNode )
{
	T2GenericRedBlackTreeNode* pTVar1;
	T2GenericRedBlackTreeNode* pTVar2;
	bool                       bVar3;

	pTVar1 = pNode->m_pRight;
	if ( pTVar1 == &ms_oNil || pTVar1 == TNULL )
	{
		pTVar1 = pNode->m_pParent;
		pTVar2 = pTVar1;
		if ( pNode == pTVar1->m_pRight )
		{
			do {
				pTVar1 = pTVar2->m_pParent;
				bVar3  = pTVar2 == pTVar1->m_pRight;
				pTVar2 = pTVar1;
			} while ( bVar3 );
		}
		if ( pTVar1->m_pParent == pTVar1 )
		{
			pTVar1 = &ms_oNil;
		}
	}
	else
	{
		pTVar2 = pTVar1->m_pLeft;
		if ( pTVar2 != &ms_oNil )
		{
			do {
				pTVar1 = pTVar2;
				pTVar2 = pTVar2->m_pLeft;
			} while ( pTVar2 != &ms_oNil );
			return pTVar1;
		}
	}
	return pTVar1;
}

// $deBlob: FUNCTION 006f2550
T2GenericRedBlackTreeNode* T2GenericRedBlackTree::GetPredecessorOf( const T2GenericRedBlackTreeNode* pNode )
{
	T2GenericRedBlackTreeNode* pTVar1;
	T2GenericRedBlackTreeNode* pTVar2 = pNode->m_pLeft;
	TBOOL                      bVar3;

	if ( pTVar2 == &ms_oNil )
	{
		pTVar2 = pNode->m_pParent;
		if ( pNode == pTVar2->m_pLeft )
		{
			do
			{
				pTVar1 = pTVar2->m_pParent;

				if ( pTVar1 == pTVar2 )
				{
					return &ms_oNil;
				}

				bVar3  = pTVar2 == pTVar1->m_pLeft;
				pTVar2 = pTVar1;
			} while ( bVar3 );

			return pTVar1;
		}
	}
	else
	{
		pTVar1 = pTVar2->m_pRight;

		if ( pTVar2->m_pRight != &ms_oNil )
		{
			do
			{
				pTVar2 = pTVar1;
				pTVar1 = pTVar2->m_pRight;
			} while ( pTVar2->m_pRight != &ms_oNil );
		}
	}

	return pTVar2;
}

TOSHI_NAMESPACE_END

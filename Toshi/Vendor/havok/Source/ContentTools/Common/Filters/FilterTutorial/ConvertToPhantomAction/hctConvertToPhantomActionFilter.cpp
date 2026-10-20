/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <ContentTools/Common/Filters/FilterTutorial/hctFilterTutorial.h>

#include <ContentTools/Common/Filters/FilterTutorial/ConvertToPhantomAction/hctConvertToPhantomActionFilter.h>

#include <Physics/Utilities/Serialize/hkpPhysicsData.h>

#include <Common/SceneData/Graph/hkxNode.h>
#include <Common/SceneData/Attributes/hkxAttributeGroup.h>

#include <Common/Base/Math/Util/hkMathUtil.h>

#include <Physics/Dynamics/Entity/hkpRigidBody.h>
#include <Physics/Dynamics/World/hkpPhysicsSystem.h>

#include <Common/Serialize/Util/hkBuiltinTypeRegistry.h>
#include <Common/Base/Reflection/hkClass.h>
#include <Common/Base/Reflection/hkTypeInfo.h>

#include <ContentTools/Common/Filters/FilterTutorial/ConvertToPhantomAction/MyPhantomShape.h>
#include <Physics/Collide/Shape/Misc/Bv/hkpBvShape.h>

// The only instance of our filter descriptor
hctConvertToPhantomActionFilterDesc g_convertToPhantomActionDesc;

hctConvertToPhantomActionFilter::hctConvertToPhantomActionFilter(const hctFilterManagerInterface* owner)
:	hctFilterInterface (owner)
{
	m_options.m_removeMeshes = false;
	m_optionsDialog = HK_NULL;
}
	
hctConvertToPhantomActionFilter::~hctConvertToPhantomActionFilter()
{

}

void hctConvertToPhantomActionFilter::process( hkRootLevelContainer& data  )
{
	// Find and hkxScene and a hkPhysics Data objects in the root level container
	hkxScene* scenePtr = reinterpret_cast<hkxScene*>( data.findObjectByType( hkxSceneClass.getName() ) );
	if (scenePtr == HK_NULL || (scenePtr->m_rootNode == HK_NULL) )
	{
		HK_WARN_ALWAYS(0xabbaa5f0, "No scene data (or scene data root hkxNode) found. Can't continue.");
		return;
	}
	hkpPhysicsData* physicsPtr = reinterpret_cast<hkpPhysicsData*>( data.findObjectByType( hkpPhysicsDataClass.getName() ) );
	if (physicsPtr == HK_NULL)
	{
		HK_WARN_ALWAYS(0xabbaa3d0, "No physics data found, you need to use Create Rigid Bodies before this filter or have bodies already in the input data.");
		return;
	}

	// Keep track of how many phantoms we created so we can reported
	int numConverted = 0;

	// Search for rigid bodies in the scene
	for (int psi=0; psi<physicsPtr->getPhysicsSystems().getSize(); psi++)
	{
		const hkpPhysicsSystem* psystem = physicsPtr->getPhysicsSystems()[psi];

		for (int rbi=0; rbi<psystem->getRigidBodies().getSize(); rbi++)
		{
			hkpRigidBody* rbody = psystem->getRigidBodies()[rbi];
			const char* rbName = rbody->getName();
			
			// Require an associated node in the scene
			hkxNode* rbNode = (rbName)? scenePtr->findNodeByName(rbName): HK_NULL;
			if( !rbNode )
			{
				continue;
			}
			
			// Require an 'hkPhantomAction' attribute group
			const hkxAttributeGroup* attrGroup = rbNode->findAttributeGroupByName("hkPhantomAction");
			if (!attrGroup)
			{
				continue;
			}
			
			// Create our phantom shape
			MyPhantomShape* myPhantomShape = new MyPhantomShape();
			{
				// Set action type (required)
				const char* actionTypeStr = HK_NULL;
				attrGroup->getStringValue( "action", true, actionTypeStr );
				if( actionTypeStr )
				{
					if( hkString::strCasecmp( actionTypeStr, "wind" ) == 0 )
					{
						myPhantomShape->m_actionType = MyPhantomShape::ACTION_WIND;
					}
					else if( hkString::strCasecmp( actionTypeStr, "attract" ) == 0 )
					{
						myPhantomShape->m_actionType = MyPhantomShape::ACTION_ATTRACT;
					}
					else if( hkString::strCasecmp( actionTypeStr, "deflect" ) == 0 )
					{
						myPhantomShape->m_actionType = MyPhantomShape::ACTION_DEFLECT;
					}
					else
					{
						HK_WARN_ALWAYS(0xabbad3b4, "Unknow action type ("<<actionTypeStr<<").");
					}
				}
				else
				{
					HK_WARN_ALWAYS(0xabba9834, "Can't fine \"action\" attribute");
				}
				
				// Set other attributes (optional)
				attrGroup->getVectorValue( "direction", false, myPhantomShape->m_direction );
				attrGroup->getFloatValue( "strength", false, myPhantomShape->m_strength );

				// Useful warnings
				if ((myPhantomShape->m_actionType == MyPhantomShape::ACTION_WIND) && (myPhantomShape->m_direction.lengthSquared3()==0.0f))
				{
					HK_WARN_ALWAYS(0xabbadf82, "Wind direction is invalid - action will have no effect");
				}
				if (myPhantomShape->m_strength==0.0f)
				{
					HK_WARN_ALWAYS(0xabbacc83, "Strength is 0 - action will have no effect");
				}

			}
			
			// Set the phantom as a new bounding shape for the body
			{
				const hkpShape* oldShape = rbody->getCollidable()->getShape();
				
				hkpBvShape* bvShape = new hkpBvShape( oldShape, myPhantomShape );
				myPhantomShape->removeReference();
				
				rbody->setShape( bvShape );
				bvShape->removeReference();
			}
			
			// Remove meshes if the user chose to do so
			if (m_options.m_removeMeshes && (hkString::strCmp(rbNode->m_object.m_class->getName(), "hkxMesh")==0) )
			{
				rbNode->m_object.m_object = HK_NULL;
			}
			
			HK_REPORT("Converted rigid body \""<<rbName<<"\" to phantom action");
			numConverted++;
		}
	}

	// Give a warning if the filter didn't do anything useful
	if (numConverted==0)
	{
		HK_WARN_ALWAYS(0xabba7632, "No rigid bodies converted to phantom action.");
	}
	else
	{
		HK_REPORT("Converted "<<numConverted<<" rigid bodies.");
	}
}


/*
* Havok SDK - NO SOURCE PC DOWNLOAD, BUILD(#20090216)
* 
* Confidential Information of Havok.  (C) Copyright 1999-2009
* Telekinesys Research Limited t/a Havok. All Rights Reserved. The Havok
* Logo, and the Havok buzzsaw logo are trademarks of Havok.  Title, ownership
* rights, and intellectual property rights in the Havok software remain in
* Havok and/or its suppliers.
* 
* Use of this software for evaluation purposes is subject to and indicates
* acceptance of the End User licence Agreement for this product. A copy of
* the license is included with this software and is also available at www.havok.com/tryhavok.
* 
*/

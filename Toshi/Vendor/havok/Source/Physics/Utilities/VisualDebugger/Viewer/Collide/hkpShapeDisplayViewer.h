/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_UTILITIES2_SHAPE_DISPLAY_VIEWER_H
#define HK_UTILITIES2_SHAPE_DISPLAY_VIEWER_H

#include <Physics/Utilities/VisualDebugger/Viewer/Dynamics/hkpWorldViewerBase.h>
#include <Physics/Dynamics/World/Listener/hkpWorldPostSimulationListener.h>
#include <Physics/Dynamics/Entity/hkpEntityListener.h>
#include <Common/Base/Container/PointerMap/hkPointerMap.h>

class hkDebugDisplayHandler;
class hkpWorld;

class hkpCollidable;
#define HK_PROPERTY_DISPLAY_PTR 0x1234
#define HK_DISPLAY_VIEWER_OPTIONS_CONTEXT "ShapeDisplayViewerOptions"

#define HK_PROPERTY_DISPLAY_PTR 0x1234

	/// Displays all the entities in a world.
class hkpShapeDisplayViewer :	public hkpWorldViewerBase,
								protected hkpEntityListener, public hkpWorldPostSimulationListener
{
	public:

			/// Creates a hkpShapeDisplayViewer.
		static hkProcess* HK_CALL create( const hkArray<hkProcessContext*>& contexts );

			/// Registers the hkpShapeDisplayViewer with the hkViewerFactory.
		static void HK_CALL registerViewer();

			/// Gets the tag associated with this viewer type
		virtual int getProcessTag() { return m_tag; }

		virtual void init();

		static inline const char* HK_CALL getName() { return "Shapes"; }

		virtual void synchronizeTransforms( class hkDebugDisplayHandler* displayHandler, hkpWorld* world );

		virtual void synchronizeTransforms(hkpWorld* world);

			/// By default the world current time is used for display, but you can
			/// set this to a different value if for example you are half - stepping the world
			/// using the integrate() and collide() functions, and want to display interpolated transforms
			/// between these calls
		void setTimeForDisplay( hkTime time ) { m_timeForDisplay = time; }

		void setDisplayBodyCreateEnabled( bool on ); // default : true, otherwise you handler will have to have precreated ones for it
		bool getDisplayBodyCreateEnabled( ) const { return m_enableDisplayCreation; } 

		void setInstancingEnabled( bool on ); // default : false
		bool getInstancingEnabled( ) const { return m_enableInstancing; } 
	
		void setAutoColorMode( bool on ); // default: true
		bool getAutoColorMode( ) const { return m_enableAutoColor; } 

		void setDisplayBodyCachingEnabled( bool on ); // default : false
		bool getDisplayBodyCachingEnabled( ) const { return m_enableDisplayCaching; } 
		void clearCache();

		void setAutoGeometryCreation( bool on ); // default : true
		bool getAutoGeometryCreation( ) const { return m_autoGeometryCreation; } 

	public:
	
		class ShapeDisplayViewerOptions: public hkReferencedObject, public hkProcessContext
		{
			public:
				virtual const char* getType(){ return HK_DISPLAY_VIEWER_OPTIONS_CONTEXT; }
				ShapeDisplayViewerOptions(): m_enableShapeTransformUpdate(true){}
				hkBool m_enableShapeTransformUpdate;
		};

	public:
		virtual void worldAddedCallback( hkpWorld* world );
		virtual void worldRemovedCallback( hkpWorld* world ); 

	protected:

		hkpShapeDisplayViewer( const hkArray<hkProcessContext*>& contexts );
		virtual ~hkpShapeDisplayViewer();

	public:
		virtual void entityAddedCallback( hkpEntity* entity );
		virtual void entityRemovedCallback( hkpEntity* entity );
		virtual void postSimulationCallback( hkpWorld* world );


		void addWorld( hkpWorld* world );
		int findWorld( hkpWorld* world );

		void removeWorld( int worldIndex );
		void removeAllGeometries( int worldIndex );
		
		void inactiveEntityMovedCallback( hkpEntity* entity );
		
		struct WorldToEntityData {
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_VDB, WorldToEntityData);
			hkpWorld* world;
			hkArray<hkUlong> entitiesCreated;
		};

		hkArray< WorldToEntityData* > m_worldEntities;
		hkPointerMap < const class hkpShape*, hkUlong > m_instancedShapeToGeomID;
		hkPointerMap < const class hkpShape*, hkUlong > m_instancedShapeToUsageCount;
		hkPointerMap < const class hkpShape*, hkUlong > m_cachedShapes;

		hkBool m_enableShapeTransformUpdate;
		hkBool m_enableInstancing;
		hkBool m_enableDisplayCaching;
		hkBool m_enableDisplayCreation;
		hkBool m_autoGeometryCreation;
		hkBool m_enableAutoColor;
		hkTime m_timeForDisplay;

	public:
		static inline hkUlong HK_CALL getId( hkpEntity* entity )
		{
			return (entity->hasProperty(HK_PROPERTY_DEBUG_DISPLAY_ID)) ?
				hkUlong(entity->getProperty(HK_PROPERTY_DEBUG_DISPLAY_ID).getInt()) :  hkUlong(entity->getCollidable());
		}
	public:
		static int m_tag;
};

#endif	// HK_UTILITIES2_SHAPE_DISPLAY_VIEWER_H


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

/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_CHARACTER_PROXY_H
#define HK_CHARACTER_PROXY_H

#include <Common/Base/Types/Physics/hkStepInfo.h>
#include <Physics/Dynamics/Entity/hkpEntityListener.h>
#include <Physics/Dynamics/Phantom/hkpPhantomListener.h>
#include <Physics/Utilities/CharacterControl/hkpCharacterControl.h>
#include <Physics/Utilities/CharacterControl/CharacterProxy/hkpCharacterProxyCinfo.h>
#include <Physics/Utilities/CharacterControl/CharacterProxy/hkpCharacterProxyListener.h>

class hkpShapePhantom;
class hkpAllCdPointCollector;
class hkpClosestCdPointCollector;
struct hkpRootCdPoint;
struct hkpSurfaceConstraintInfo;
struct hkpSurfaceConstraintInteraction;
struct hkpSimplexSolverOutput;
struct hkpLinearCastInput;

/// The character proxy class is used to represent a non penetrating shape that can move dynamically around
/// the scene. It is called character proxy because it is usually used to represent game characters. It could
/// just as easily be used to represent any dynamic game object.
class hkpCharacterProxy :	public hkReferencedObject,
							public hkpEntityListener,
							public hkpPhantomListener
{
	public:

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_CHARACTER);

			/// Initialize members from the construction info.
			/// Add references to each of the shapes.
		hkpCharacterProxy(const hkpCharacterProxyCinfo& info);

			/// Remove references to the phantom
			/// Remove the phantom if it has been added to the world
		virtual ~hkpCharacterProxy();

			/// Get construction info from this character proxy.
		void getCinfo(hkpCharacterProxyCinfo& info) const;

			/// Update from a given construction info.
		void updateFromCinfo( const hkpCharacterProxyCinfo& cinfo );

			/// Update and move the character. To override the logic defining which bodies the character can collide
			/// with or capture this information for your own use (e.g. to deal with 'trigger' volumes), use ::integrateWithCollectors() instead.
			/// By default ::integrate() uses hkAllCdPointCollectors for the character's collision queries.
			/// The worldGravity parameter is only used when the character's mass is greater than 0, to apply downward impulses to dynamic objects.
		void integrate(const hkStepInfo& stepInfo, const hkVector4& worldGravity);

			/// Update and move the character. You must pass the two collectors used internally in the hkpShapePhantom::setPositionAndLinearCast()
			/// calls. Implementing your own hkpAllCdPointCollector will allow you to customize the character's movement behavior
			/// or extract information about objects it overlaps with or objects it linear casts through.
			/// The gravity parameter is only used when the character's mass is greater than 0, to apply downward impulses to dynamic objects.
		void integrateWithCollectors(const hkStepInfo& stepInfo, const hkVector4& gravity, hkpAllCdPointCollector& castCollector, hkpAllCdPointCollector& startPointCollector);

			/// Check and see if the character is supported in the given direction (and by what).
			/// This call checks the geometry immediately around the character, and does not take velocity into account.
			/// i.e. if the character is moving away from the ground, but is still touching the ground, this function
			/// will return SUPPORTED as the supported state.
		void checkSupport(const hkVector4& direction, hkpSurfaceInfo& ground);

			/// This function is the same as the checkSupportFunction, except it allows you to pass your own collectors to filter
			/// collisions. You only need to call this if you are using the integrateWithCollectors call.
		void checkSupportWithCollector(const hkVector4& direction, hkpSurfaceInfo& ground, hkpAllCdPointCollector& startPointCollector);

			/// When hkpCharacterProxy::integrate() is called, or hkpWorld::stepDeltaTime() is called, the manifold
			/// information will become out of date.  If you use the manifold in your state machine, and need it to be
			/// up to date, you should call this function first. Note that this function is called by checkSupport automatically,
			/// so if you call checkSupport just before your state machine, you do not need to call this function.
		void refreshManifold( hkpAllCdPointCollector& startPointCollector );


			/// Read access to the current manifold for the character
		const hkArray<hkpRootCdPoint>& getManifold() const;

			/// Get current position
		const hkVector4& getPosition() const;

			/// Warp the character (through walls etc.)
		void setPosition(const hkVector4& position);

			/// Gets the linear velocity
		const hkVector4& getLinearVelocity() const;

			/// Sets the velocity
		void setLinearVelocity( const hkVector4& vel );

			/// Get the character phantom
		hkpShapePhantom*	getShapePhantom();

			/// Get the character phantom const version
		const hkpShapePhantom* getShapePhantom() const;

			/// The 3.0.0 version of check support
		void checkSupportDeprecated(const hkVector4& direction, hkpSurfaceInfoDeprecated& ground) const;

			/// Add a hkpCharacterProxyListener
		void addCharacterProxyListener(hkpCharacterProxyListener* listener);

			/// Remove a hkpCharacterProxyListener
		void removeCharacterProxyListener(hkpCharacterProxyListener* listener);


	protected:

			/// Update the manifold of contact points without calling collision detection
		virtual void updateManifold(const hkpAllCdPointCollector& startPointCollector, const hkpAllCdPointCollector& castCollector);

			/// Apply impulses to the object.
		void applySurfaceInteractions( const hkStepInfo& info, const hkVector4& gravity );

			/// Build surface constraint information from the point returned by a cast.
			/// Extract the information from a contact point returned by the linear caster.
			/// This information is translated into surface constraint information.
			/// This is the bridge between the collision detector and the character controller.
		virtual void extractSurfaceConstraintInfo(const hkpRootCdPoint& hit, hkpSurfaceConstraintInfo& surfaceOut, hkReal timeTravelled) const;

			// Search the current manifold for this surface
			// returns -1 if the surface has not been found
		int findSurface(const hkpRootCdPoint& info) const;

			// Defines a distance metric for keeping and discarding planes
			// Used to update the mnaifold
		inline hkReal surfaceDistance(const hkpRootCdPoint& p1, const hkpRootCdPoint& p2) const;

			// Extract the surface velocity at a point returned by a cast
		inline void extractSurfaceVelocity(const hkpRootCdPoint& hit, hkVector4& velocityOut) const;

			/// Converts the distance stored as a fraction in the cast collector to a proper euclidean distance.
			/// The cast collector returns distances as a fraction of the cast.
			///	We need to convert these to Euclidian distances, and project them onto the
			/// normal of the collision plane. This replaces the fraction by the
			/// correct distance and moves the fraction to the normal.w component.
		inline void convertFractionToDistance( const hkpRootCdPoint* hits , int numHits, const hkVector4& displacement) const;

			// utility methods
		void fireContactAdded(const hkpRootCdPoint& point) const;
		void fireContactRemoved(const hkpRootCdPoint& point) const;
		void fireConstraintsProcessed( const hkArray<hkpRootCdPoint>& manifold, hkpSimplexSolverInput& input ) const;

			// Set position and return time travelled.
		hkReal moveToLinearCastHitPosition(const hkpSimplexSolverOutput& output,
			const hkpAllCdPointCollector& castCollector,
			const hkpLinearCastInput& castInput,
			hkVector4& position);

	public:
			// used internally to update the manifold if a body is deleted
		void entityRemovedCallback( hkpEntity* entity );

		// used internally to update the manifold if a phantom is deleted
		void phantomRemovedCallback( hkpPhantom* entity );

        void calcContentStatistics( hkStatisticsCollector* collector, const hkClass* cls) const;

	public:

		hkVector4				m_velocity;

		hkVector4				m_oldDisplacement;

		hkpShapePhantom*		m_shapePhantom;

		hkReal					m_dynamicFriction;

		hkReal					m_staticFriction;

		hkVector4				m_up;

		hkReal					m_extraUpStaticFriction;

		hkReal					m_extraDownStaticFriction;

		hkReal					m_keepDistance;

		hkReal					m_keepContactTolerance;

		hkReal					m_contactAngleSensitivity;

		int						m_userPlanes;

		hkReal					m_maxCharacterSpeedForSolver;

		hkReal					m_characterStrength;

		hkReal					m_characterMass;

		hkArray<hkpRootCdPoint>	m_manifold;

		hkArray<hkpCharacterProxyListener*> m_listeners;

		hkArray<hkpRigidBody*>	m_bodies;

		hkArray<hkpPhantom*>		m_phantoms;

		hkReal					m_maxSlopeCosine;

		hkReal					m_penetrationRecoverySpeed;

		int						m_maxCastIterations;

		bool					m_refreshManifoldInCheckSupport;

};

#endif //HK_CHARACTER_PROXY_H

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

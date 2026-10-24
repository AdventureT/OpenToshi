/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_CONSTRAINT_PROJECTOR_H
#define	HK_CONSTRAINT_PROJECTOR_H

/// This is an utility class that can be used to project constraints pivots and limits in order to correct large positional and angular errors.
class	hkpConstraintProjector
{
public:
	/// Projection presets
	enum	Preset
	{
		HK_DEFAULT,									///
		HK_PROJECT_PIVOTS,							///
		HK_HIERARCHY,								///
	};
	/// Constraint sort type
	enum	Sort
	{
		HK_SORT_NONE,								///
		HK_SORT_PER_ERROR,							///
		HK_SORT_PER_MASS,							///
	};
public:
	Sort					m_sortMode;				///
	int						m_numPositionIterations;///
	int						m_numVelocityIterations;///
	hkReal					m_globalTau;			///
	hkReal					m_finalTau;				///
	hkReal					m_linearTau;			///
	hkReal					m_angularTau;			///
	hkReal					m_maxLinearError;		///
	hkReal					m_maxAngularError;		///
	hkReal					m_maxLinearVelocity;	///
	hkReal					m_maxAngularVelocity;	///
	hkReal					m_warmupT0;				///
	hkReal					m_warmupT1;				///
	hkReal					m_linearDamping;		///
	hkReal					m_angularDamping;		///
	hkBool					m_finalProject;			///
	hkBool					m_projectOnly;			///
public:
		/// The constructor initialize projection parameters with the given preset
	hkpConstraintProjector(Preset preset=HK_DEFAULT) { setPreset(preset); }
public:
		/// Set the preset used for constraints projection
		/// Notes: Projection parameters can be manually tuned by directly settings their values.
	void setPreset(Preset preset);
		/// Project constraints by moving rigid bodies referenced by constraints.
	void project(const hkArray<hkpConstraintInstance*>& constraints,hkpRigidBody* root=HK_NULL,hkReal timestep=0) const;
};

#endif

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

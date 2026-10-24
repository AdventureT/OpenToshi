/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_SERIALIZE_PATCH_MANAGER_H
#define HK_SERIALIZE_PATCH_MANAGER_H

#include <Common/Base/Types/hkTypedUnion.h>
#include <Common/Base/Container/StringMap/hkStringMap.h>
#include <Common/Serialize/Data/hkDataObject.h>

class hkClassNameRegistry;
class hkDataClassDict;
class hkDataWorldDict;

	/// Class defining and managing versions of classes using patches.
	/// You should hkVersionPatchManager::getInstance() to access instance
	/// of the manager.
class hkVersionPatchManager : public hkReferencedObject, public hkSingleton<hkVersionPatchManager>
{
	public:
			/// Supported patch component types.
		enum PatchType
		{
			PATCH_INVALID,

			PATCH_MEMBER_ADDED,
			PATCH_MEMBER_REMOVED,
			PATCH_MEMBER_RENAMED,

			PATCH_FUNCTION,

			PATCH_CAST,

			PATCH_DEPENDS,

			PATCH_PARENT_SET,

			PATCH_END
		};

			/// Class defining a versioning patch for reflected class.
		struct PatchInfo
		{
				/// Class defining a component of the versioning patch, e.g. add and/or remove class member.
			struct Component
			{
				PatchType type;
				const void* patch;
			private:
				HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_SERIALIZE, Component);
			};
			const char* oldName;
			const char* newName;
			hkInt32 oldVersion;
			hkInt32 newVersion;
			const Component* component;
			hkInt32 numComponent;
		private:
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_SERIALIZE, PatchInfo);
		};
		
		typedef void (*Function)(hkDataObject& o);

		typedef hkStringMap<hkDataClassDict*> ClassImplFromName;
		
			/// hkVersionPatchManager constructor.
		hkVersionPatchManager();

			/// hkVersionPatchManager destructor.
		~hkVersionPatchManager();

			/// Set class version dependency in patch.
		struct DependsPatch
		{
			enum { PATCH_TYPE=PATCH_DEPENDS };
			const char* name;
			hkInt32 version;
		private:
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_SERIALIZE, DependsPatch);
		};

			/// Define class member renamed in patch.
		struct MemberRenamedPatch
		{
			enum { PATCH_TYPE=PATCH_MEMBER_RENAMED};
			const char* oldName;
			const char* newName;
		private:
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_SERIALIZE, MemberRenamedPatch);
		};

			/// Define class member added in patch.
		struct MemberAddedPatch
		{
			enum { PATCH_TYPE=PATCH_MEMBER_ADDED};
			const char* name;
			hkDataObject::Type type;
			const char* typeName;
			hkInt32 tuples;
		private:
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_SERIALIZE, MemberAddedPatch);
		};

			/// Define class member removed in patch.
		struct MemberRemovedPatch
		{
			enum { PATCH_TYPE=PATCH_MEMBER_REMOVED};
			const char* name;
			hkDataObject::Type type;
			const char* typeName;
			hkInt32 tuples;
		private:
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_SERIALIZE, MemberRemovedPatch);
		};

			/// Set class parent in patch.
 		struct SetParentPatch
		{
			enum { PATCH_TYPE=PATCH_PARENT_SET };
			const char* oldParent;
			const char* newParent;
		private:
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_SERIALIZE, SetParentPatch);
		};

			/// Set function to be called in patch.
		struct FunctionPatch
		{
			enum { PATCH_TYPE=PATCH_FUNCTION };
			const char* name;
			Function function;
		private:
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_SERIALIZE, FunctionPatch);
		};

			/// Set cast to class in patch.
		struct CastPatch
		{
			enum { PATCH_TYPE=PATCH_CAST };
			const char* name;
		private:
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_SERIALIZE, CastPatch);
		};

			/// Apply registered patches to the 'world'.
			/// The function optionally takes custom class name registry specifying final classes
			/// for versioning data in 'world'. By default, the hkBuiltinTypeRegistry is used.
			/// This function already calls hkVersionPatchManager::recomputePatchDependencies().
		hkResult applyPatches(hkDataWorldDict& world, const hkClassNameRegistry* finalClassesReg = HK_NULL) const;

			/// Register individual patch with the version patch manager.
			/// The patch dependencies must be recomputed to guarantee proper sequence
			/// of patches when applied to classes in a dictionary world.
		void addPatch(const PatchInfo* p);

			/// Recompute all patch dependencies and set them in proper order,
			/// so classes can be updated accordingly.
		hkResult recomputePatchDependencies() const;


			// Used internally.
		hkResult applyPatchesDebug(hkDataWorldDict& world) const;

			// Used internally.
			// Return all registered patches.
			// The order of the patches may not be set according to dependencies,
			// you should call hkVersionPatchManager::recomputePatchDependencies()
			// to guarantee it.
		const hkArray<const PatchInfo*>& getPatches() const;

			// Used internally.
			// Generate uid, an unique 64-bit value, from class name and version.
		hkUint64 getUid( const char* name, hkInt32 ver ) const;

			// Used internally.
			// Get class name from uid.
		const char* getClassName( hkUint64 uid ) const;

			// Used internally.
			// Get class version from uid.
		hkInt32 getClassVersion( hkUint64 uid ) const;

			// Used internally.
			// Get update patch index from uid, if any.
			// Return -1 if no update patch found.
		HK_FORCE_INLINE hkInt32 getPatchIndex( hkUint64 uid ) const;

			// Used internally.
			// Get patch by index.
		HK_FORCE_INLINE const PatchInfo* getPatch( hkInt32 patchIndex ) const;

			// Used internally.
			// Find last registered patch index for given uid.
			// The patches should be recomputed before calling this function
			// (see hkVersionPatchManager::recomputePatchDependencies() for details).
		int findLastPatchIndexForUid(hkUint64 uid, hkBool32 allowRenames = false) const;

	private:

		class UidFromClassVersion;
		UidFromClassVersion* m_uidFromClassVersion;
			// all patches
		mutable hkArray<const PatchInfo*> m_patchInfos;
		mutable hkPointerMap<hkUint64, hkInt32> m_patchIndexFromUid;

	private:

		hkResult preparePatches(hkDataWorldDict& worldToUpdate, const hkClassNameRegistry& finalClassesReg, hkArray<const PatchInfo*>& patchInfosOut) const;
};

#include <Common/Serialize/Version/hkVersionPatchManager.inl>

#endif // HK_SERIALIZE_PATCH_MANAGER_H

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

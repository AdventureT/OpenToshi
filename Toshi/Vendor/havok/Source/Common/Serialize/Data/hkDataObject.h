/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_DATA_OBJECT_H
#define HK_DATA_OBJECT_H

#include <Common/Base/Container/StringMap/hkStorageStringMap.h>

class hkDataObject;
class hkDataArray;
class hkDataClass;
class hkDataWorld;

class hkDataObjectImpl;
class hkDataArrayImpl;
class hkDataClassImpl;

	/// The hkDataObject class is a public accessor to reflected data
	/// managed by the private hkDataObjectImpl class.
	/// It may only contain data defined by hkDataClass.
	/// The class implements the Python-alike object data access.
class hkDataObject
{
	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_SERIALIZE, hkDataObject);

			/// Constructor - instantiates a temporary public wrapper
			/// using the private hkDataObjectImpl.
		hkDataObject( hkDataObjectImpl* impl );
			/// Constructor - instantiates a temporary public wrapper
			/// using the hkDataObjectImpl shared by the given 'o'.
		hkDataObject( const hkDataObject& o );

			/// Assign a temporary wrapper using the hkDataObjectImpl shared by the given 'o'.
		void operator=( const hkDataObject& o );
			/// Return true if hkDataObjects are the same.
		bool operator==( const hkDataObject& o );
			/// Return true if hkDataObjects are different.
		bool operator!=( const hkDataObject& o );

			/// Destructor.
		~hkDataObject();

			/// This struct defines the unique hkDataObject.
		struct Handle
		{ 
			void* p0;
			void* p1;
		};

			/// hkDataObject data types.
		enum Type
		{
				/// Not reflected.
			TYPE_VOID = 0,

				/// Byte, signed or unsigned 8-bit integer.
			TYPE_BYTE,
				/// Signed or unsigned 16/32/64-bit integer. 
			TYPE_INT,
				/// 32-bit float.
			TYPE_REAL,
				/// Fixed array of 4 TYPE_REAL (e.g. hkVector, hkQuaternion).
			TYPE_VEC_4,
				/// Fixed array of 8 TYPE_REAL.
			TYPE_VEC_8,
				/// Fixed array of 12 TYPE_REAL (e.g. hkMatrix3, hkQsTransform, hkRotation).
			TYPE_VEC_12,
				/// Fixed array of 16 TYPE_REAL (e.g. hkTransform, hkMatrix4).
			TYPE_VEC_16,
				/// hkDataObject.
			TYPE_OBJECT,
				/// hkDataObject (embedded struct data).
			TYPE_STRUCT,
				/// C-style string.
			TYPE_CSTRING,
				/// The number of basic hkDataObject types.
			TYPE_NUM_BASIC_TYPES,
				/// Mask for the basic hkDataObject types.
			TYPE_MASK_BASIC_TYPES = 0xf,

				/// Bit indicating an array of the basic type data.
			TYPE_ARRAY = 0x10, // per object size array
				/// Array of TYPE_BYTE.
			TYPE_ARRAY_BYTE = TYPE_ARRAY | TYPE_BYTE,
				/// Array of TYPE_INT.
			TYPE_ARRAY_INT = TYPE_ARRAY | TYPE_INT,
				/// Array of TYPE_REAL.
			TYPE_ARRAY_REAL = TYPE_ARRAY | TYPE_REAL,
				/// Array of TYPE_VEC_4.
			TYPE_ARRAY_VEC_4 = TYPE_ARRAY | TYPE_VEC_4,
				/// Array of TYPE_VEC_8.
			TYPE_ARRAY_VEC_8 = TYPE_ARRAY | TYPE_VEC_8,
				/// Array of TYPE_VEC_12.
			TYPE_ARRAY_VEC_12 = TYPE_ARRAY | TYPE_VEC_12,
				/// Array of TYPE_VEC_16.
			TYPE_ARRAY_VEC_16 = TYPE_ARRAY | TYPE_VEC_16,
				/// Array of TYPE_OBJECT.
			TYPE_ARRAY_OBJECT = TYPE_ARRAY | TYPE_OBJECT,
				/// Array of TYPE_STRUCT.
			TYPE_ARRAY_STRUCT = TYPE_ARRAY | TYPE_STRUCT,
				/// Array of TYPE_CSTRING.
			TYPE_ARRAY_CSTRING = TYPE_ARRAY | TYPE_CSTRING,

				/// Bit indicating a tuple of the basic type data.
			TYPE_TUPLE = 0x20, // fixed size array, size is per class
				/// Tuple of TYPE_BYTE.
			TYPE_TUPLE_BYTE = TYPE_TUPLE | TYPE_BYTE,
				/// Tuple of TYPE_INT.
			TYPE_TUPLE_INT = TYPE_TUPLE | TYPE_INT,
				/// Tuple of TYPE_REAL.
			TYPE_TUPLE_REAL = TYPE_TUPLE | TYPE_REAL,
				/// Tuple of TYPE_VEC_4.
			TYPE_TUPLE_VEC_4 = TYPE_TUPLE | TYPE_VEC_4,
				/// Tuple of TYPE_VEC_8.
			TYPE_TUPLE_VEC_8 = TYPE_TUPLE | TYPE_VEC_8,
				/// Tuple of TYPE_VEC_12.
			TYPE_TUPLE_VEC_12 = TYPE_TUPLE | TYPE_VEC_12,
				/// Tuple of TYPE_VEC_16.
			TYPE_TUPLE_VEC_16 = TYPE_TUPLE | TYPE_VEC_16,
				/// Tuple of TYPE_OBJECT.
			TYPE_TUPLE_OBJECT = TYPE_TUPLE | TYPE_OBJECT,
				/// Tuple of TYPE_STRUCT.
			TYPE_TUPLE_STRUCT = TYPE_TUPLE | TYPE_STRUCT,
				/// Tuple of TYPE_CSTRING.
			TYPE_TUPLE_CSTRING = TYPE_TUPLE | TYPE_CSTRING,
		};

			/// Return the number of 32-bit floats from one of the fixed array types.
		static inline int getNumRealsFromType(hkDataObject::Type mtype);

			/// This class wraps and handles data values stored in hkDataObject.
		class Value
		{
			public:

				HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_SERIALIZE, Value);

					/// Assign 'l' to value.
				void operator=(const Value& l);
					/// Assign 32-bit integer to value.
				void operator=(int i);
					/// Assign 64-bit integer to value.
 				void operator=(hkInt64 i);
					/// Assign 32-bit float to value.
 				void operator=(hkReal r);
					/// Assign c-style string to value.
				void operator=(const char* s);
					/// Assign array or tuple to value.
				void operator=(const hkDataArray& l);
					/// Assign object to value.
 				void operator=(const hkDataObject& o);

					/// Get value as 32-bit integer.
				int asInt() const;
					/// Get value as 64-bit integer.
				hkInt64 asInt64() const;
					/// Get value as 32-bit float.
				hkReal asReal() const;
					/// Get value as c-style string.
				const char* asString() const;
					/// Get value as array or tuple.
				hkDataArray asArray() const;
					/// Get value as object.
				hkDataObject asObject() const;
					/// Get value as array of floats, e.g. hkVector4, hkMatrix4.
					/// 'n' indicates the number of floats you expect to get, e.g. 4, 8, 12 or 16.
				const hkReal* asVec(int n) const;

					/// Get value type.
				Type getType() const;

				// Used internally.
				void setVec(const hkReal* r, int n);
#				define HK_VALUE_ACCESS2(HKTYPE,TYPE) \
					void operator=(const HKTYPE& v) \
					{ this->setVec(reinterpret_cast<const hkReal*>(&v), sizeof(HKTYPE)/sizeof(hkReal)); } \
					const HKTYPE& as##TYPE() const \
					{ return *reinterpret_cast<const HKTYPE*>(asVec(sizeof(HKTYPE)/sizeof(hkReal))); }
#				define HK_VALUE_ACCESS(TYPE) HK_VALUE_ACCESS2(hk##TYPE,TYPE)
				HK_VALUE_ACCESS(Vector4)
				HK_VALUE_ACCESS(Quaternion)
				HK_VALUE_ACCESS(Matrix3)
				HK_VALUE_ACCESS(Rotation)
				HK_VALUE_ACCESS(QsTransform)
				HK_VALUE_ACCESS(Matrix4)
				HK_VALUE_ACCESS(Transform)
#undef			HK_VALUE_ACCESS2
#undef			HK_VALUE_ACCESS

				friend class hkDataObjectImpl;
				Value(hkDataObjectImpl* impl, const char* name) : m_impl(impl), m_name(name) { }
				hkDataObjectImpl* m_impl;
				const char* m_name;
		};

			/// Get value of class member named 'name'.
		Value operator[](const char* name);
			/// Get const value of class member named 'name'.
		const Value operator[](const char* name) const;

			/// Get object's handle.
		Handle getHandle() const;
			/// Get object's implementation.
		hkDataObjectImpl* getImplementation() { return m_impl; }
			/// Get object's const implementation.
		const hkDataObjectImpl* getImplementation() const { return m_impl; }

			/// Get object's class.
		const hkDataClass getClass() const;
			/// Is the object null.
		hkBool32 isNull() const;

		typedef int Iterator;

			/// Get class member iterator.
		Iterator getMemberIterator() const;
			/// Is the iterator valid.
		hkBool32 isValid(Iterator i) const;
			/// Get next iterator.
		Iterator getNextMember(Iterator i) const;
			/// Get member value by iterator.
		const Value getMemberValue(Iterator i) const;
			/// Get member name by iterator.
		const char* getMemberName(Iterator i) const;

			/// Return true if object stores value of class member named 'name'.
		hkBool32 hasMember(const char* name) const;

	private:

		hkDataObjectImpl* m_impl;
};

	/// The hkDataArray is a public accessor to a reflected array of data.
class hkDataArray
{
	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_SERIALIZE, hkDataArray);

		typedef hkDataObject::Type Type;

			/// This class wraps and handles data value of hkDataArray items.
		class Value
		{
			public:

				HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_SERIALIZE, Value);

					/// Assign object value 'l' to value.
				void operator=(const hkDataObject::Value& l);
					/// Assign array value 'l' to value.
				void operator=(const Value& l);
					/// Assign 32-bit integer to value.
				void operator=(int i);
					/// Assign 64-bit integer to value.
 				void operator=(hkInt64 i);
					/// Assign 32-bit float to value.
 				void operator=(hkReal r);
					/// Assign array or a tuple to value.
  				void operator=(const hkDataArray& l);
					/// Assign object to value.
 				void operator=(const hkDataObject& o);
					/// Assign c-style string to value.
				void operator=(const char* s);

					/// Get value as 32-bit integer.
				int asInt() const;
					/// Get value as 32-bit float.
				hkReal asReal() const;
					/// Get value as object.
				hkDataObject asObject() const;
					/// Get value as array or tuple.
				hkDataArray asArray() const;
					/// Get value as c-style string.
				const char* asString() const;
					/// Get value as array of floats, e.g. hkVector4, hkMatrix4.
					/// 'n' indicates the number of floats you expect to get, e.g. 4, 8, 12 or 16.
				const hkReal* asVec(int n) const;

					/// Clear the value.
				void clear();

					/// Get the value type.
				Type getType() const;

				// Used internally.
				void setVec(const hkReal* r, int nreal);
#				define HK_VALUE_ACCESS2(HKTYPE,TYPE) \
					void operator=(const HKTYPE& v) \
						{ this->setVec(reinterpret_cast<const hkReal*>(&v), sizeof(HKTYPE)/sizeof(hkReal)); } \
					const HKTYPE& as##TYPE() const \
						{ return *reinterpret_cast<const HKTYPE*>(asVec(sizeof(HKTYPE)/sizeof(hkReal))); }
#				define HK_VALUE_ACCESS(TYPE) HK_VALUE_ACCESS2(hk##TYPE,TYPE)
				HK_VALUE_ACCESS(Vector4)
				HK_VALUE_ACCESS(Quaternion)
				HK_VALUE_ACCESS(Matrix3)
				HK_VALUE_ACCESS(Rotation)
				HK_VALUE_ACCESS(QsTransform)
				HK_VALUE_ACCESS(Matrix4)
				HK_VALUE_ACCESS(Transform)
#undef			HK_VALUE_ACCESS2
#undef			HK_VALUE_ACCESS

			private:

				friend class hkDataArray;
				Value(hkDataArrayImpl* impl, int idx) : m_impl(impl), m_index(idx) {}
				hkDataArrayImpl* m_impl;
				int m_index;
		};
			/// Constructor - instantiates a temporary public wrapper
			/// using the private hkDataArrayImpl.
		explicit hkDataArray( hkDataArrayImpl* i );
			/// Constructor - instantiates a temporary public wrapper
			/// using the hkDataArrayImpl shared by the given 'a'.
		hkDataArray(const hkDataArray& a);
			/// Assign a temporary wrapper using the hkDataArrayImpl shared by the given 'a'.
		void operator=(const hkDataArray& a);
			/// Destructor.
		~hkDataArray();

			/// Clear array. Size set to 0.
		void clear();
			/// Get array size.
		int getSize() const;
			/// Reserve array capacity.
		void reserve(int n);
			/// Set array size.
 		void setSize(int n);
			/// Get value of i'th array item.
	  	Value operator[](int i);
			/// Get const value of i'th array item.
  		const Value operator[](int i) const;
			/// Get array item type.
		hkDataObject::Type getType() const;
			/// Get array item class.
		hkDataClass getClass() const;
			/// Get data slice of class member named 'name' from object items in array.
		hkDataArray swizzleObjectMember(const char* name) const;

			/// Set array size to 'n' and initialize the array items with values from 'a'.
		void setAll(const hkBool* a, int n);
			/// Set array size to 'n' and initialize the array items with values from 'a'.
		void setAll(const char* a, int n);
			/// Set array size to 'n' and initialize the array items with values from 'a'.
		void setAll(const hkInt8* a, int n);
			/// Set array size to 'n' and initialize the array items with values from 'a'.
		void setAll(const hkUint8* a, int n);
			/// Set array size to 'n' and initialize the array items with values from 'a'.
		void setAll(const hkInt16* a, int n);
			/// Set array size to 'n' and initialize the array items with values from 'a'.
		void setAll(const hkUint16* a, int n);
			/// Set array size to 'n' and initialize the array items with values from 'a'.
		void setAll(const hkInt32* a, int n);
			/// Set array size to 'n' and initialize the array items with values from 'a'.
		void setAll(const hkUint32* a, int n);
			/// Set array size to 'n' and initialize the array items with values from 'a'.
		void setAll(const hkInt64* a, int n);
			/// Set array size to 'n' and initialize the array items with values from 'a'.
		void setAll(const hkUint64* a, int n);
			/// Set array size to 'n' and initialize the array items with values from 'a'.
		void setAll(const hkReal* a, int n);

		//getAll?

			/// Get array's implementation.
		hkDataArrayImpl* getImplementation() { return m_impl; }
			/// Get array's const implementation.
		const hkDataArrayImpl* getImplementation() const { return m_impl; }

	private:

		hkDataArrayImpl* m_impl;
};

	/// This class defines reflection information for any reflected type.
	/// Unlike hkClass it defines reflection for class data members only.
class hkDataClass
{
	public:
		
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_SERIALIZE, hkDataClass);
		
			/// This structure is used to describe a member of hkDataClass.
		struct MemberInfo;

			/// Constructor - instantiates a temporary public wrapper
			/// using the private hkDataClassImpl.
		hkDataClass( hkDataClassImpl* impl );
			/// Destructor.
		~hkDataClass();

			/// Is the class null.
		hkBool32 isNull() const;
			/// Get class name.
		const char* getName() const;
			/// Get class version.
		int getVersion() const;
			/// Get class parent.
		hkDataClass getParent() const;

			/// Get the world the class belongs to.
		const hkDataWorld* getWorld() const;
			/// Is this a parent class of class k.
		hkBool isSuperClass(const hkDataClass& k) const;

			/// Get the number of members declared in this class (not including parent class members).
		int getNumDeclaredMembers() const;
			/// Get the i'th member declared in this class (not including parent class members).
		void getDeclaredMemberInfo(int i, MemberInfo& info) const;
			/// Get the index of declared member named 'name' or -1 if not found.
		int getDeclaredMemberIndexByName(const char* name) const;
			/// Get all declared members (not including parent class members).
		void getAllDeclaredMemberInfo( hkArray<MemberInfo>& minfo ) const;

			/// Get the number of members in this class (including parent class members).
		int getNumMembers() const;
			/// Get the i'th member (including parent class members).
		void getMemberInfo(int i, MemberInfo& info) const;
			/// Get the index of member named 'name' or -1 if not found.
		int getMemberIndexByName(const char* name) const;
			/// Get all members (including parent class members).
		void getAllMemberInfo( hkArray<MemberInfo>& minfo ) const;

			/// Get the const private implementation of the class.
		const hkDataClassImpl* getImplementation() const { return m_impl; }
			/// Get the private implementation of the class.
		hkDataClassImpl* getImplementation() { return m_impl; }

	private:

		hkDataClassImpl* m_impl;
};

struct hkDataClass::MemberInfo
{
	MemberInfo() : m_name(HK_NULL), m_owner(HK_NULL), m_type(hkDataObject::TYPE_VOID), m_class(HK_NULL), m_tupleCount(0) {}
	const char* m_name;
	const hkDataClassImpl* m_owner;
	hkDataObject::Type m_type;
	const hkDataClassImpl* m_class;
	int m_tupleCount;
};

	/// This is an abstract class which defines interfaces to implement a container,
	/// and to store and manage reflected data defined by hkDataClass and represented
	/// by hkDataObject and hkDataArray.
class hkDataWorld : public hkReferencedObject
{
	public:

			/// This structure contains all the information needed to construct a hkDataClass
			/// object.
		struct ClassCinfo
		{
				/// This structure contains all the information needed to construct
				/// a member of hkDataClass.
			struct Member
			{
				const char* name;
				hkDataObject::Type type;
				int tupleCount;
				const char* className;
			};
			const char* name;
			int version;
			hkDataClassImpl* parent;
			hkArray<Member> members;
		};

			/// Create a new class from the cinfo.
		virtual hkDataClassImpl* newClass(const ClassCinfo& cinfo) = 0;

			/// Create a new object of the given type in the world.
			/// The 'klass' must belong to the world when instantiating hkDataObject.
		virtual hkDataObjectImpl* newObject(const hkDataClass& klass) const = 0;

			/// Create a new array as a member of obj.
			/// The 'obj' must belong to the world when instantiating hkDataArray.
		virtual hkDataArrayImpl* newArray(hkDataObject& obj, const hkDataClass::MemberInfo& minfo) const = 0;

			/// hkDataWorld supported types.
		enum DataWorldType
		{
				/// Native type. hkDataWorld contains wrapped native
				/// object and hkClass pointers that are represented
				/// by hkDataObject and hkDataClass respectively.
				/// See hkDataWorldNative for details.
			TYPE_NATIVE,

				/// Dictionary type. hkDataWorld contains instances of
				/// hkDataObject described by hkDataClass.
				/// See hkDataWorldDict for details.
			TYPE_DICTIONARY
		};

			/// Get world type.
		virtual hkEnum<DataWorldType, hkInt32> getType() const = 0;
			/// Get top level object from the world.
			/// By default, the first created hkDataObject is the top level object.
		virtual hkDataObject getContents() const = 0;
			/// Find all the registered classes in the world.
		virtual void findAllClasses(hkArray<hkDataClassImpl*>& classesOut) const = 0;
			/// Find the registered hkDataClass in the world by name.
		virtual hkDataClassImpl* findClass(const char* name) const = 0;
};

inline int hkDataObject::getNumRealsFromType(hkDataObject::Type mtype)
{
	HK_ASSERT(0x47254701, mtype >= hkDataObject::TYPE_VEC_4 && mtype <= hkDataObject::TYPE_VEC_16);
	return (1 + mtype - hkDataObject::TYPE_VEC_4) * 4;
}

#endif // HK_DATA_OBJECT_H

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

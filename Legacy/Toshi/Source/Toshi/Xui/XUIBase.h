#pragma once
#include "Toshi/Math/Math.h"

#include <cstdint>

struct BezierPoint
{
	Toshi::TVector2 Point;
	Toshi::TVector2 Ctrl1;
	Toshi::TVector2 Ctrl2;
};

enum XUI_ELEMENT_PROPERTY_TYPE
{
	XUI_EPT_EMPTY,
	XUI_EPT_BOOL,
	XUI_EPT_INTEGER,
	XUI_EPT_UNSIGNED,
	XUI_EPT_FLOAT,
	XUI_EPT_STRING,
	XUI_EPT_COLOR,
	XUI_EPT_VECTOR,
	XUI_EPT_QUATERNION,
	XUI_EPT_OBJECT,
	XUI_EPT_CUSTOM,
	XUI_EPT_SHORT32,
	XUI_EPT_USHORT32,
	XUI_EPT_UINT8
};

typedef TBOOL   XUIEPTBool;
typedef TINT32  XUIEPTInteger;
typedef TUINT32 XUIEPTUnsigned;
typedef TFLOAT  XUIEPTFloat;
typedef TUINT16 XUIEPTString;
typedef TUINT32 XUIEPTColor;
typedef TUINT16 XUIEPTVector;
typedef TUINT16 XUIEPTQuaternion;
typedef TUINT16 XUIEPTObject;
typedef TUINT32 XUIEPTCustom;
typedef TINT16  XUIEPTShort32;
typedef TUINT16 XUIEPTUShort32;
typedef TUINT32 XUIEPTUColor;
typedef TUINT8  XUIEPTUInt8;

// property flags
#define XUI_ELEMENT_PROPF_NONE     0x0000 // no flags specified
#define XUI_ELEMENT_PROPF_INDEXED  0x0001 // the property is indexed
#define XUI_ELEMENT_PROPF_HIDDEN   0x0002 // the property is hidden in the design time environment
#define XUI_ELEMENT_PROPF_LOCALIZE 0x0004 // the property is localizable
#define XUI_ELEMENT_PROPF_NOANIM   0x0008 // the property must not be animated
#define XUI_ELEMENT_PROPF_FILEPATH 0x0010 // the property represents a file path
#define XUI_ELEMENT_PROPF_EVENT    0x0020 // treat as an event (set on keyframes only)
#define XUI_ELEMENT_PROPF_DESIGN   0x0040 // design time only property

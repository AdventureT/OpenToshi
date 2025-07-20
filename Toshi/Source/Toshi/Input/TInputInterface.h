#pragma once
#include "Toshi/Core/TNodeList.h"
#include "Toshi/Core/TArray.h"

TOSHI_NAMESPACE_START

class TInputDevice;

class TInputInterface
    : public TGenericClassDerived<TInputInterface, TObject, "TInputInterface", TMAKEVERSION(1, 0), TFALSE>
    , public TSingleton<TInputInterface>
{
public:
	enum EVENT_TYPE
	{
		EVENT_TYPE_GONE_DOWN,
		EVENT_TYPE_GONE_UP,
		EVENT_TYPE_REPEAT,
		EVENT_TYPE_UNKNOWN,
		EVENT_TYPE_MOVED
	};

	class InputEvent
	{
	public:
		InputEvent(TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType);
		InputEvent(TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType, TWCHAR* a_wszString);
		InputEvent(TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType, TINT a_iMagnitude1);
		InputEvent(TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType, TINT a_iMagnitude1, TINT a_iMagnitude2);
		InputEvent(TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType, TFLOAT a_fMagnitude1);
		InputEvent(TInputDevice* a_pDevice, TINT a_iDoodad, EVENT_TYPE a_eEventType, TFLOAT a_fMagnitude1, TFLOAT a_fMagnitude2);

		TINT GetMagnitudeInt(TINT a_iAxis);

		TFLOAT GetMagnitudeFloat(TINT a_iAxis);

		int8_t GetAxisCount() const { return m_iAxisCount; }

		TINT GetDoodad() const { return m_iDoodad; }

		EVENT_TYPE GetEventType() const { return m_eEventType; }

		TInputDevice* GetSource() const { return m_pSource; }

		TBOOL IsMagnitudeFloat() { return m_bIsMagnitudeFloat; }

		TBOOL IsMagnitudeInt() { return !m_bIsMagnitudeFloat; }

	public:
		TINT          m_iDoodad;           // 0x00
		EVENT_TYPE    m_eEventType;        // 0x04
		TBOOL         m_bIsMagnitudeFloat; // 0x08
		int8_t        m_iAxisCount;        // 0x09
		TInputDevice* m_pSource;           // 0x0C
		TWCHAR        m_wszString[4];      // 0x10

		union
		{
			TFLOAT Floats[2];
			TINT   Ints[2];
		} m_Magnitude; // 0x18 De blob 0x10 JPOG
	};

public:
	TInputInterface() { m_bIsExclusiveMode = TFALSE; }

	TInputDevice* GetDeviceByIndex(TClass* pClass, size_t index);

	template <class C> C* GetDeviceByIndex(size_t index = 0)
	{
		C* pDevice = TSTATICCAST(C, GetDeviceByIndex(TGetClass(C), index));
		TASSERT(pDevice == TNULL || pDevice->GetClass()->IsA(TGetClass(C)));
		return pDevice;
	}

	void AddDevice(TInputDevice* device);
	void RemoveDevice(TInputDevice* device);

	virtual TBOOL Initialise() { return TTRUE; }
	virtual TBOOL Deinitialise();

	virtual TBOOL AcquireAll();
	virtual TBOOL UnacquireAll();
	virtual TBOOL FlushAll();
	virtual void  SetExclusiveMode(TBOOL mode);
	virtual TBOOL GetExclusiveMode() const;
	virtual TINT  ProcessEvents(TFLOAT fUnk);
	virtual void  StopAllRepeats();

private:
	TNodeList<TInputDevice>                                m_DeviceList;       // 0x04
	TBOOL                                                  m_bIsExclusiveMode; // 0x14
	TEmitter<TInputInterface, TInputInterface::InputEvent> m_Emitter1;         // 0x24
	TGenericEmitter                                        m_Emitter2;         // 0x28
};

class TInputDevice
    : public TGenericClassDerived<TInputDevice, TObject, "TInputDevice", TMAKEVERSION(1, 0), TFALSE>
    , public TNodeList<TInputDevice>::TNode
{
public:
	static constexpr TINT INPUT_DEVICE_MOUSE_BUTTONS = 3;
	static constexpr TINT INPUT_DEVICE_MOUSE_WHEEL   = 4;

	using EventEmitter = TEmitter<TInputInterface, TInputInterface::InputEvent>;
	using Doodad       = TINT;

	struct DoodadProperties
	{
		TINT  m_iUnk;
		TBOOL m_bFlag;
	};

	struct RepeatInfo
	{
		Doodad iDoodad;
		TFLOAT fLeftTime;
		TFLOAT fRepeatTime;
	};

public:
	TInputDevice()
	    : m_Repeats(0, 16), m_Array2(0, 16)
	{
		m_pInputInterface           = TNULL;
		m_bUnknown                  = TFALSE;
		m_bIsAcquired               = TFALSE;
		m_uiDeviceIndex             = s_uiDeviceCount++;
		s_aDevices[m_uiDeviceIndex] = this;
	}

	virtual TBOOL        Acquire()                = 0;
	virtual TBOOL        Unacquire()              = 0;
	virtual void         Release()                = 0;
	virtual void         Update(TFLOAT deltaTime) = 0;
	virtual TBOOL        Flush() { return TTRUE; }
	virtual TINT         ProcessEvents(EventEmitter& emitter, TFLOAT deltaTime)                = 0;
	virtual TINT         GetButtonCount() const                                                = 0;
	virtual TINT         GetAxisCount() const                                                  = 0;
	virtual TBOOL        GetDoodadProperties(TINT doodad, DoodadProperties& doodadProps) const = 0;
	virtual TBOOL        StartRepeat(Doodad a_iDoodad, TFLOAT a_fFirstRepeatTime, TFLOAT a_fRepeatTime);
	virtual void         StopRepeat(Doodad a_iDoodad);
	virtual void         StopAllRepeats();
	virtual TBOOL        IsForceFeedbackDevice() { return TFALSE; }
	virtual Platform     GetPlatform() const                        = 0;
	virtual const TCHAR* GetButtonFromDoodad(TINT a_iDoodad) const  = 0;
	virtual TBOOL        IsDown(TINT doodad) const                  = 0;
	virtual TINT         GetAxisInt(TINT doodad, TINT axis) const   = 0;
	virtual TFLOAT       GetAxisFloat(TINT doodad, TINT axis) const = 0;
	virtual TBOOL        IsEnabled() const                          = 0;
	virtual void         ThrowRepeatEvent(EventEmitter& emitter, RepeatInfo* repeatInfo, TFLOAT deltaTime);

	TBOOL IsAcquired() const { return m_bIsAcquired; }

	void SetAcquired(TBOOL a_bAcquired) { m_bIsAcquired = a_bAcquired; }

	TInputInterface* GetInputInterface() { return m_pInputInterface; }

	void SetInputInterface(TInputInterface* a_pInterface) { m_pInputInterface = a_pInterface; }

protected:
	TINT ProcessRepeats(EventEmitter& a_rEmitter, TFLOAT a_fDeltaTime);

public:
	static TInputDevice** GetRegisteredDevices() { return s_aDevices; }

	static size_t GetNumRegisteredDevices() { return s_uiDeviceCount; }

protected:
	static constexpr size_t MAX_DEVICE_COUNT = 14;

	inline static TInputDevice* s_aDevices[MAX_DEVICE_COUNT];
	inline static size_t        s_uiDeviceCount;

protected:
	size_t                      m_uiDeviceIndex;   // 0x14
	TArray<RepeatInfo>::Storage m_Repeats;         // 0x18 FIXME: replace void* with some structure whose size is 0xC
	TArray<void*>::Storage      m_Array2;          // 0x28 FIXME: replace void* with some structure whose size is 0x4
	TBOOL                       m_bUnknown;        // 0x38
	TBOOL                       m_bIsAcquired;     // 0x39 de blob 0x35 JPOG
	TInputInterface*            m_pInputInterface; // 0x3C
};

TOSHI_NAMESPACE_END

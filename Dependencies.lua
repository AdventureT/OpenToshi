IncludeDir = {}
IncludeDir.toshi = "%{wks.location}/Toshi/Source"
IncludeDir.fmod = "%{wks.location}/Toshi/Vendor/fmod/include"
-- FMOD Ex + FMOD Event 4.44 (what retail uses; headers + import libs in the repo)
IncludeDir.fmodex = "%{wks.location}/Toshi/Vendor/fmodex/include"
IncludeDir.libogg = "%{wks.location}/Toshi/Vendor/libogg/include"
IncludeDir.libvorbis = "%{wks.location}/Toshi/Vendor/libvorbis/include"
IncludeDir.libtheora = "%{wks.location}/Toshi/Vendor/libtheora/include"
IncludeDir.theoraplay = "%{wks.location}/Toshi/Vendor/theoraplay/include"
IncludeDir.stb = "%{wks.location}/Toshi/Vendor/stb"
-- Havok 6.5 SDK (retail linked Havok Physics 5.5.0, which isn't available)
IncludeDir.havok = "%{wks.location}/Toshi/Vendor/havok/Source"

LibDir = {}
LibDir.fmod = "%{wks.location}/Toshi/Vendor/fmod/lib"
LibDir.fmodex = "%{wks.location}/Toshi/Vendor/fmodex/lib"
-- VS2008 libs; the _dll variants match our CRT DLL runtime (/MDd in Debug, /MD otherwise)
LibDir.havok = "%{wks.location}/Toshi/Vendor/havok/Lib/win32_net_9-0"

-- content of these folders should be copied to any client application
ClientContentCommon = "%{wks.location}Content/Common/"
ClientContentArch   = "%{wks.location}Content"

-- modify paths based on architecture
LibDir.fmod = LibDir.fmod .. "/" .. _OPTIONS["arch"] .. "/"
LibDir.fmodex = LibDir.fmodex .. "/" .. _OPTIONS["arch"] .. "/"
ClientContentArch = ClientContentArch .. "/" .. _OPTIONS["arch"] .. "/"

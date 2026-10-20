project "Toshi"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"
	characterset "ASCII"

	pchheader "ToshiPCH.h"
	pchsource "Source/ToshiPCH.cpp"
	
	files
	{
		"Source/ToshiPCH.cpp",
		"Source/Core/**.h",
		"Source/Core/**.cpp",
		"Source/Toshi/**.h",
		"Source/Toshi/**.cpp",
		"Source/ToshiTools/**.h",
		"Source/ToshiTools/**.cpp",
		"Source/File/**.h",
		"Source/File/**.cpp",
		"Source/Math/**.h",
		"Source/Math/**.cpp",
		"Source/Input/**.h",
		"Source/Input/**.cpp",
		"Source/Render/**.h",
		"Source/Render/**.cpp",
		"Source/Shaders/**.h",
		"Source/Shaders/**.cpp",
		"Source/TParticleSystem/**.h",
		"Source/TParticleSystem/**.cpp",
		"Source/Sound/**.h",
		"Source/Sound/**.cpp",
		"Source/T2Locale/**.h",
		"Source/T2Locale/**.cpp",
		"Source/Thread/**.h",
		"Source/Thread/**.cpp",
		"Source/Plugins/**.h",
		"Source/Plugins/**.cpp",
		"Source/GUI/**.h",
		"Source/GUI/**.cpp",
		"Source/XUI/**.h",
		"Source/XUI/**.cpp",
		"Source/Physics/**.h",
		"Source/Physics/**.cpp"
	}

	includedirs
	{
		"Source",
		"Vendor/WiiYourself",
		"Icons",
		"Resources",
		"%{IncludeDir.fmodex}",
		"%{IncludeDir.stb}",
		"%{IncludeDir.havok}"
	}
	
	libdirs
	{
		"%{LibDir.fmodex}"
	}

	defines
	{
		"TOSHI_USER_ENGINE",
		"_CRT_SECURE_NO_WARNINGS"
	}
	
	filter "files:**.c"
		enablepch "Off"
		
	filter "files:Source/Profiler/**.*"
		enablepch "Off"

	-- WiiYourself! 1.15 (third-party, gl.tter): the Wii remotes of TInputWiinDeviceController. It links setupapi.lib,
	-- hid.lib and winmm.lib itself (#pragma comment)
	filter "files:Vendor/WiiYourself/**.cpp"
		enablepch "Off"
		warnings "Off"

	filter "system:windows"
		systemversion "latest"

		-- FMOD Ex + FMOD Event 4.44 (fmodex.dll, fmod_event.dll; retail doesn't load fmod_event_net.dll)
		links
		{
			"fmodex_vc.lib",
			"fmod_event.lib"
		}

		-- Havok (Physics + Common, as retail)
		links
		{
			"hkBase.lib",
			"hkSerialize.lib",
			"hkSceneData.lib",
			"hkInternal.lib",
			"hkGeometryUtilities.lib",
			"hkVisualize.lib",
			"hkCompat.lib",
			"hkpCollide.lib",
			"hkpConstraintSolver.lib",
			"hkpDynamics.lib",
			"hkpInternal.lib",
			"hkpUtilities.lib"
		}

		files
		{
			"Source/Platform/Windows/**.cpp",
			"Source/Platform/Windows/**.c",
			"Source/Platform/Windows/**.h",
			"Vendor/WiiYourself/wiimote.cpp",
			"Vendor/WiiYourself/*.h",
			"Icons/*.ico",
			"Resources/*.h",
			"Resources/*.aps",
			"Resources/*.rc"
		}
		
		filter "options:renderer=DX11"
			links
			{
				"d3d11.lib",
				"dxgi.lib",
				"d3dcompiler.lib",
				"dxguid.lib",
				"winmm.lib",
				"dinput8.lib"
			}

			files
			{
				"Source/Platform/DX11/**.cpp",
				"Source/Platform/DX11/**.c",
				"Source/Platform/DX11/**.h"
			}

		defines
		{
			"TOSHI_SKU_WINDOWS"
		}

	filter "configurations:Debug"
		defines "TOSHI_DEBUG"
		runtime "Debug"
		symbols "On"
		libdirs "%{LibDir.havok}/debug_multithreaded_dll"

	filter "configurations:Release or Final"
		libdirs "%{LibDir.havok}/release_multithreaded_dll"

	filter "configurations:Release"
		defines "TOSHI_RELEASE"
		runtime "Release"
		optimize "Speed" -- /O2, as retail (root premake5.lua)

	filter "configurations:Final"
		defines "TOSHI_FINAL"
		runtime "Release"
		optimize "Speed" -- /O2, as retail (root premake5.lua)
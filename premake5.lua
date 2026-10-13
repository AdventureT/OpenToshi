include "Settings.lua"
include "Dependencies.lua"

workspace "OpenBlob"
	cppdialect "C++20"
	characterset "ASCII"

	configurations { "Debug", "Release", "Final" }
	platforms "Win32"

	-- Enables fast builds
	multiprocessorcompile "on"

	disablewarnings { "4996" }

	debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	editandcontinue "Off"

	-- Global defines
	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"NOMINMAX",
		"DEBLOB_COMMUNITY_PATCH", -- Fixes of retail bugs and dev features; the original code is kept in #else
		-- NOTE: no "USE_ATOMIC" (Barnyard's atomic TMutex, TSemaphore and TFifo): de Blob uses the Win32 objects
	}

	-- Global settings for include dirs
	includedirs
	{
		"%{IncludeDir.toshi}"
	}

	filter "options:arch=x86"
		architecture "x86"

	-- Global Windows parameters
	filter "system:windows"
		systemversion "latest"
		vectorextensions "SSE2"

		exceptionhandling "Off"
		linkoptions "/SAFESEH:NO"

		defines
		{
			"TOSHI_SKU_WINDOWS"
		}

	filter "options:renderer=DX11"
		defines
		{
			"TRENDERINTERFACE_DX11"
		}

	filter "configurations:Debug"
		runtime "Debug"
		defines "TOSHI_DEBUG"
		symbols "On"

	filter "configurations:Release"
		runtime "Release"
		defines "TOSHI_RELEASE"
		optimize "On"

	filter "configurations:Final"
		runtime "Release"
		defines "TOSHI_FINAL"
		optimize "On"

filter {}

-- Include the projects
group "Engine"
	include "Toshi"
	include "OpenBlob"

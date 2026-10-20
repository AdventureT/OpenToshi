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

		-- Retail's build shape (DevTools/Matching/README.md): /Gz (__stdcall default calling convention: free and static
		-- functions end in "ret n") and /EHsc (retail has C++ EH frames). Every project, the vendored C libraries too
		-- (retail's ogg/vorbis code is __stdcall as well; their qsort comparators are explicitly __cdecl)
		callingconvention "StdCall"   -- /Gz
		exceptionhandling "On"        -- /EHsc
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

	filter "configurations:Final"
		runtime "Release"
		defines "TOSHI_FINAL"

	-- Retail's optimisation flags: /O2 /Oy- /Ob2 /Oi /GS /Gy /GR /GF /arch:SSE2 /fp:precise /MD, no /GL
	filter "configurations:Release or Final"
		optimize "Speed"              -- /O2
		omitframepointer "Off"        -- /Oy-: every retail function has push ebp / mov ebp, esp
		inlining "Auto"               -- /Ob2
		intrinsics "On"               -- /Oi
		functionlevellinking "On"     -- /Gy
		stringpooling "On"            -- /GF
		buffersecuritycheck "On"      -- /GS
		rtti "On"                     -- /GR
		floatingpoint "Default"       -- /fp:precise
		linktimeoptimization "Off"    -- no /GL

filter {}

-- Include the projects
group "Engine"
	include "Toshi"
	include "OpenBlob"

-- Ogg Theora movies (Data\Movies\*.bik.ogv): libogg, libvorbis, libtheora and theoraplay (library code, see
-- Tools/ProgressCalculation/ignorelist.txt)
group "Vendor"
	include "Toshi/Vendor/libogg"
	include "Toshi/Vendor/libvorbis"
	include "Toshi/Vendor/libtheora"
	include "Toshi/Vendor/theoraplay"

-- NOTE: the vendored projects link the static CRT; Toshi and OpenBlob use the CRT DLL
for _, name in ipairs( { "libogg", "libvorbis", "libtheora", "theoraplay" } ) do
	project( name )
		staticruntime "off"
		warnings "Off"

		-- NOTE: optimised in Debug too (an unoptimised decoder can't keep up with the movies)
		filter "configurations:Debug"
			optimize "Speed"
			runtimechecks "Off"

		filter "configurations:Final"
			runtime "Release"
			optimize "On"

		filter {}
end

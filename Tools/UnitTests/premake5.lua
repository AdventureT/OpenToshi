project "UnitTests"
	language "C++"
	cppdialect "C++20"
	characterset "ASCII"
	staticruntime "on"

	links
	{
		TOSHI_PROJECT_NAME,
	}

	files
	{
		"Source/**.h",
		"Source/**.hpp",
		"Source/**.cpp",
	}

	includedirs
	{
		"Source",
		"%{wks.location}/Toshi/Source",
		"%{wks.location}/Shared/Source",
		"%{IncludeDir.spdlog}",
	}
	
	defines
	{
		"TOSHI_USER_CLIENT",
		"SPDLOG_WCHAR_TO_UTF8_SUPPORT"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"TOSHI_CONSOLE",
			"TOSHI_SKU_WINDOWS"
		}

	filter "options:renderer=DX11"
		defines
		{
			"TOSHI_RENDERER_DX11"
		}

	filter "configurations:Debug"
		kind "ConsoleApp"
		runtime "Debug"
		defines "TOSHI_DEBUG"
		symbols "On"

	filter "configurations:Release"
		kind "ConsoleApp"
		runtime "Release"
		defines "TOSHI_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		kind "ConsoleApp"
		runtime "Release"
		defines "TOSHI_DIST"
		optimize "On"
		
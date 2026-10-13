project "OpenBlob"
	language "C++"
	staticruntime "off"

	pchheader "pch.h"
	pchsource "Source/pch.cpp"

	links
	{
		"Toshi"
	}

	linkoptions
	{
		"%{wks.location}/bin-int/" .. outputdir .. "/Toshi/Toshi.res",
	}

	-- NOTE: game subfolders are listed explicitly; add new ones here
	files
	{
		"Source/*.h",
		"Source/*.cpp"
	}

	libdirs
	{
		"%{LibDir.fmod}"
	}

	includedirs
	{
		"Source",
		"%{wks.location}/Toshi/Resources",
		"%{IncludeDir.fmod}",
		"%{IncludeDir.stb}"
	}

	postbuildcommands
	{
		"{COPYDIR} \"" .. ClientContentCommon  .. "\" \"%{wks.location}bin/" .. outputdir .. "/%{prj.name}/\"",
	}

	filter "configurations:Debug"
		kind "ConsoleApp"

	filter "configurations:Release"
		kind "ConsoleApp"

	filter "configurations:Final"
		kind "WindowedApp"

	filter "system:windows"
		files
		{
			"Source/Platform/Windows/**.cpp",
			"Source/Platform/Windows/**.h"
		}

	filter "options:renderer=DX11"
		files
		{
			"Source/Platform/DX11/**.cpp",
			"Source/Platform/DX11/**.h"
		}

function ExtractLib(options)
	local catch2Location = "../Vendor/Catch2/lib/" .. _OPTIONS["arch"]
	local zipLocation = catch2Location .. "/Catch2" .. options.dbg .. ".zip"
	local libLocation = catch2Location .. "/Catch2" .. options.dbg .. ".lib"
	
	if not os.isfile(libLocation) then
		zip.extract(zipLocation, catch2Location)
		os.remove(zipLocation)
	end
end


project "UnitTests"
	language "C++"
	cppdialect "C++20"
	characterset "ASCII"
	staticruntime "on"	

	links
	{
		TOSHI_PROJECT_NAME,
	}
	
	libdirs
	{
		"%{LibDir.catch2}",
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
		"%{IncludeDir.catch2}"
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
		
		ExtractLib { dbg = "d" }
		
		links
		{
			"Catch2d.lib"
		}

	filter "configurations:Release"
		kind "ConsoleApp"
		runtime "Release"
		defines "TOSHI_RELEASE"
		optimize "On"
		
		ExtractLib { dbg = ""}
		
		links
		{
			"Catch2.lib"
		}

	filter "configurations:Dist"
		kind "ConsoleApp"
		runtime "Release"
		defines "TOSHI_DIST"
		optimize "On"
		
		ExtractLib { dbg = "" }
		
		links
		{
			"Catch2.lib"
		}

		
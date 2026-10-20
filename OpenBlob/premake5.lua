project "OpenBlob"
	language "C++"
	staticruntime "off"

	pchheader "pch.h"
	pchsource "Source/pch.cpp"

	links
	{
		"Toshi",
		"libogg",
		"libvorbis",
		"libtheora",
		"theoraplay"
	}

	linkoptions
	{
		"%{wks.location}/bin-int/" .. outputdir .. "/Toshi/Toshi.res",
		-- NOTE: the Havok 6.5 libs (VS2008) add a Microsoft.VC90(.Debug)CRT manifest dependency through their
		-- objects' linker directives, and that SxS assembly isn't installed (CreateProcess error 14001); their CRT
		-- calls resolve to the UCRT, so the loader is told to ignore the manifest's dependencies
		"/ALLOWISOLATION:NO",
	}

	-- NOTE: game subfolders are listed explicitly; add new ones here
	files
	{
		"Source/*.h",
		"Source/*.cpp",
		"Source/Console/*.h",
		"Source/Console/*.cpp",
		"Source/Cameras/*.h",
		"Source/Cameras/*.cpp",
		"Source/Debug/*.h",
		"Source/Debug/*.cpp",
		"Source/GameController/*.h",
		"Source/GameController/*.cpp",
		"Source/GameInterface/*.h",
		"Source/GameInterface/*.cpp",
		"Source/Animation/*.h",
		"Source/Animation/*.cpp",
		"Source/Collision/*.h",
		"Source/Collision/*.cpp",
		"Source/Entities/*.h",
		"Source/Entities/*.cpp",
		"Source/Entities/ABlobEntity/*.h",
		"Source/Entities/ABlobEntity/*.cpp",
		"Source/Entities/ABoss/*.h",
		"Source/Entities/ABoss/*.cpp",
		"Source/Entities/ABossTrigger/*.h",
		"Source/Entities/ABossTrigger/*.cpp",
		"Source/Entities/ACar/*.h",
		"Source/Entities/ACar/*.cpp",
		"Source/Entities/APropTurret/*.h",
		"Source/Entities/APropTurret/*.cpp",
		"Source/Entities/APropHazard/*.h",
		"Source/Entities/APropHazard/*.cpp",
		"Source/Entities/APropWindCannon/*.h",
		"Source/Entities/APropWindCannon/*.cpp",
		"Source/Entities/Helpers/*.h",
		"Source/Entities/Helpers/*.cpp",
		"Source/Entities/Projectiles/*.h",
		"Source/Entities/Projectiles/*.cpp",
		"Source/Entities/AEnemyWrapper/*.h",
		"Source/Entities/AEnemyWrapper/*.cpp",
		"Source/Entities/Enemies/*.h",
		"Source/Entities/Enemies/*.cpp",
		"Source/Entities/ALeechbot2/*.h",
		"Source/Entities/ALeechbot2/*.cpp",
		"Source/Entities/AInky_Pen/*.h",
		"Source/Entities/AInky_Pen/*.cpp",
		"Source/Entities/AInky_Baton/*.h",
		"Source/Entities/AInky_Baton/*.cpp",
		"Source/Entities/AInky_Baton_Elite/*.h",
		"Source/Entities/AInky_Baton_Elite/*.cpp",
		"Source/Entities/AInkJet/*.h",
		"Source/Entities/AInkJet/*.cpp",
		"Source/Entities/AInkJetSpawner/*.h",
		"Source/Entities/AInkJetSpawner/*.cpp",
		"Source/Entities/ATankSpawner/*.h",
		"Source/Entities/ATankSpawner/*.cpp",
		"Source/Entities/AStreetSpawner/*.h",
		"Source/Entities/AStreetSpawner/*.cpp",
		"Source/Entities/ASquad/*.h",
		"Source/Entities/ASquad/*.cpp",
		"Source/Entities/APropTriggerEnemyGroup/*.h",
		"Source/Entities/APropTriggerEnemyGroup/*.cpp",
		"Source/Entities/ABigPaintBot/*.h",
		"Source/Entities/ABigPaintBot/*.cpp",
		"Source/Entities/AJetbikeSpawner/*.h",
		"Source/Entities/AJetbikeSpawner/*.cpp",
		"Source/Entities/AI/Behaviour/*.h",
		"Source/Entities/AI/Behaviour/*.cpp",
		"Source/Entities/AI/Schedule/*.h",
		"Source/Entities/AI/Schedule/*.cpp",
		"Source/Entities/PathFinding/*.h",
		"Source/Entities/PathFinding/*.cpp",
		"Source/FXComposition/*.h",
		"Source/FXComposition/*.cpp",
		"Source/Fx/*.h",
		"Source/Fx/*.cpp",
		"Source/Fx/Shadows/*.h",
		"Source/Fx/Shadows/*.cpp",
		"Source/GUI/*.h",
		"Source/GUI/*.cpp",
		"Source/JobSystem/*.h",
		"Source/JobSystem/*.cpp",
		"Source/Input/*.h",
		"Source/Input/*.cpp",
		"Source/IO/*.h",
		"Source/IO/*.cpp",
		"Source/Locale/*.h",
		"Source/Locale/*.cpp",
		"Source/Memory/*.h",
		"Source/Memory/*.cpp",
		"Source/MemoryCard/*.h",
		"Source/MemoryCard/*.cpp",
		"Source/Options/*.h",
		"Source/Options/*.cpp",
		"Source/Sound/*.h",
		"Source/Sound/*.cpp",
		"Source/Subtitles/*.h",
		"Source/Subtitles/*.cpp",
		"Source/Movie/*.h",
		"Source/Movie/*.cpp",
		"Source/Render/*.h",
		"Source/Render/*.cpp",
		"Source/XUI/*.h",
		"Source/XUI/*.cpp",
		"Source/XUI/FrontEnd/*.h",
		"Source/XUI/FrontEnd/*.cpp",
		"Source/XUI/FrontEnd/Multiplayer/*.h",
		"Source/XUI/FrontEnd/Multiplayer/*.cpp",
		"Source/XUI/FrontEnd/Lists/*.h",
		"Source/XUI/FrontEnd/Lists/*.cpp",
		"Source/XUI/ElementTypes/*.h",
		"Source/XUI/ElementTypes/*.cpp",
		"Source/XUI/InGame/*.h",
		"Source/XUI/InGame/*.cpp",
		"Source/XUI/InGame/HUD/*.h",
		"Source/XUI/InGame/HUD/*.cpp",
		"Source/Render/Lighting/*.h",
		"Source/Render/Lighting/*.cpp",
		"Source/Render/Sky/*.h",
		"Source/Render/Sky/*.cpp",
		"Source/Render/Decal/*.h",
		"Source/Render/Decal/*.cpp",
		"Source/Render/Material/*.h",
		"Source/Render/Material/*.cpp",
		"Source/Render/UVAnimation/*.h",
		"Source/Render/UVAnimation/*.cpp",
		"Source/Render/ModelLoading/*.h",
		"Source/Render/ModelLoading/*.cpp",
		"Source/Render/PaintParticleSystem/*.h",
		"Source/Render/PaintParticleSystem/*.cpp",
		"Source/ParticleSystem/*.h",
		"Source/ParticleSystem/*.cpp",
		"Source/Buildings/*.h",
		"Source/Buildings/*.cpp",
		"Source/Cheats/*.h",
		"Source/Cheats/*.cpp",
		"Source/Challenges/*.h",
		"Source/Challenges/*.cpp",
		"Source/ARevolutionary/*.h",
		"Source/ARevolutionary/*.cpp",
		"Source/Terrain2/*.h",
		"Source/Terrain2/*.cpp",
		"Source/Shaders/**.h",
		"Source/Shaders/**.cpp"
	}

	libdirs
	{
		"%{LibDir.fmodex}"
	}

	filter "configurations:Debug"
		libdirs "%{LibDir.havok}/debug_multithreaded_dll"

	filter "configurations:Release or Final"
		libdirs "%{LibDir.havok}/release_multithreaded_dll"

	filter {}

	includedirs
	{
		"Source",
		"%{wks.location}/Toshi/Resources",
		"%{wks.location}/Toshi/Vendor/WiiYourself",
		"%{IncludeDir.fmodex}",
		"%{IncludeDir.stb}",
		"%{IncludeDir.theoraplay}",
		"%{IncludeDir.havok}"
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

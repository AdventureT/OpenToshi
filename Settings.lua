newoption
{
	trigger = "arch",
	description = "Choose architecture you want to build for",
	allowed = {
		{ "x86", "32bit"},
	},
	default = "x86"
}

newoption
{
	trigger = "renderer",
	description = "Choose renderer API you want to build for",
	allowed = {
		{ "DX11", "DirectX 11"},
	},
	default = "DX11"
}

outputdir = ("%{cfg.buildcfg}_%{cfg.platform}_%{cfg.architecture}_" .. _OPTIONS["renderer"])

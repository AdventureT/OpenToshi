#pragma once
#include "Toshi/Core/Core.h"

#include <spdlog/spdlog.h>

TOSHI_NAMESPACE_START

class TLog
{
public:
	static void Create();
	static void Destroy();

	inline static STL::Ref<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; };
	inline static STL::Ref<spdlog::logger>& GetAppLogger() { return s_AppLogger; };

private:
	static TBOOL                    s_Created;
	static STL::Ref<spdlog::logger> s_CoreLogger;
	static STL::Ref<spdlog::logger> s_AppLogger;
};

TOSHI_NAMESPACE_END

// Engine log macros
#define TOSHI_CORE_INFO(...)     Toshi::TLog::GetCoreLogger()->info(__VA_ARGS__)
#define TOSHI_CORE_WARN(...)     Toshi::TLog::GetCoreLogger()->warn(__VA_ARGS__)
#define TOSHI_CORE_ERROR(...)    Toshi::TLog::GetCoreLogger()->error(__VA_ARGS__)
#define TOSHI_CORE_TRACE(...)    Toshi::TLog::GetCoreLogger()->trace(__VA_ARGS__)
#define TOSHI_CORE_CRITICAL(...) Toshi::TLog::GetCoreLogger()->critical(__VA_ARGS__)

// App log macros
#define TOSHI_INFO(...)     Toshi::TLog::GetAppLogger()->info(__VA_ARGS__)
#define TOSHI_WARN(...)     Toshi::TLog::GetAppLogger()->warn(__VA_ARGS__)
#define TOSHI_ERROR(...)    Toshi::TLog::GetAppLogger()->error(__VA_ARGS__)
#define TOSHI_TRACE(...)    Toshi::TLog::GetAppLogger()->trace(__VA_ARGS__)
#define TOSHI_CRITICAL(...) Toshi::TLog::GetAppLogger()->critical(__VA_ARGS__)

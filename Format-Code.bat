@echo off
set clang_format=%cd%\vendor\clang-format.exe
set directories=(Toshi\Source,OpenBlob\Source,OpenGlobs\Source,Shared\Source)

for %%P in %directories% do (for %%G in (.cpp, .hpp, .h) do forfiles /p .\%%P\ /s /m *%%G /c "cmd /c %clang_format% -i @path")
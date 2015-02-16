cd avisynthfilter
"\Program Files\Microsoft Visual Studio 9.0\Vc\vcpackages\vcbuild.exe" filter.sln "Release|Win32" 
copy Release\filter.dll .. /y
cd ..

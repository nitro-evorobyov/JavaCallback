
#pragma once


typedef  void(*callback) (const char *, const char*);
__declspec(dllexport) void __stdcall RunSimple(int stepsCount, callback javaCallback);


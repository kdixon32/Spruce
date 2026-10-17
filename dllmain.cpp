#include "pch.h"
#include <d3d9.h>
#include <cstdio>

using Direct3DCreate9_t = IDirect3D9 * (WINAPI*)(UINT);

static HMODULE			g_realDll = nullptr;
static Direct3DCreate9_t	g_realCreate = nullptr;

static void Log(const char* msg)
{
	FILE* f = nullptr;
	if (fopen_s(&f, "d3d9_proxy.log", "a") == 0)
	{
		fprintf(f, "%s\n", msg);
		fclose(f);
	}
}

static bool LoadRealD3D9()
{
	if (g_realCreate) return true;

	char path[MAX_PATH];
	char line[512];

	UINT len = GetSystemWow64DirectoryA(path, MAX_PATH);
	if (len == 0)
	{
		GetSystemDirectoryA(path, MAX_PATH);
	}
	strcat_s(path, "\\d3d9.dll");

	g_realDll = LoadLibraryA(path);
	if (!g_realDll) 
	{
		sprintf_s(line, "LoadLibraryA failed for '%s' (error %lu)", path, GetLastError());
		Log(line);
		return false;
	}

	g_realCreate = reinterpret_cast<Direct3DCreate9_t>(GetProcAddress(g_realDll, "Direct3DCreate9"));
	if (!g_realCreate)
	{
		sprintf_s(line, "GetProcAddress failed in '%s' (error %lu)", path, GetLastError());
		Log(line);
		return false;
	}

	sprintf_s(line, "Loaded real d3d9.dll from '%s'", path);
	Log(line);
	return true;
}

extern "C" IDirect3D9* WINAPI Proxy_Direct3DCreate9(UINT sdkVersion)
{
	if (!LoadRealD3D9())
	{
		Log("Failed to load the real d3d9.dll");
		return nullptr;
	}

	IDirect3D9* d3d = g_realCreate(sdkVersion);
	
	char line[128];
	sprintf_s(line, "Direct3DCreate9(sdk=%u) -> %p", sdkVersion, static_cast<void*>(d3d));
	Log(line);
	return d3d;
}

BOOL APIENTRY DllMain(HMODULE, DWORD, LPVOID)
{
	return TRUE;
}
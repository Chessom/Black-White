#pragma once
#include"stdafx.h"
#include"core.hpp"
#include<Windows.h>
namespace bw::env {
	enum class CPU { intel, amd };
	enum class GPU { nvidia, amd };
	extern int cpu, gpu;
	extern int winver;
	void set_codepage(int codepage) {
		SetConsoleCP(codepage);
	}
	void set_font(const std::wstring& wstr) {
		CONSOLE_FONT_INFOEX info = { 0 };
		info.cbSize = sizeof(info);
		info.dwFontSize.Y = 16;
		info.FontWeight = FW_NORMAL;
		wcscpy(info.FaceName, wstr.c_str());
		SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), NULL, &info);
	}
	void set_window_size(core::coord crd) {
		HANDLE outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		SMALL_RECT winSize = { 0, 0 };
		winSize.Right = crd.x;
		winSize.Bottom = crd.y;
		SetConsoleWindowInfo(outputHandle, 1, &winSize);
	}
	void set_window_size(int x, int y) {
		set_window_size({ x,y });
	}
}
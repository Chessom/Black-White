#pragma once
#include"stdafx.h"
#include"env.hpp"
#include"config.hpp"
#include"archive.hpp"
namespace bw {
	void initialize() {
		bw::env::set_codepage(65001);
		bw::env::set_font(L"ĞÂËÎÌå");
		bw::env::set_window_size(101, 45);
		
	}
}
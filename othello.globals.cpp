﻿#pragma once
#include "stdafx.h"
#include "othello.globals.hpp"
namespace bw::othello {
	namespace symbols {
		std::string col0 = reinterpret_cast<const char*>(u8"⚫"), col1 = reinterpret_cast<const char*>(u8"⚪"), none = reinterpret_cast<const char*>(u8"■");
	}
	namespace numbers {
		std::string col0 = "1 ", col1 = "2 ", none = "0 ";
	}
	charmap CharMap{};
	std::vector<std::string> gamer_list = { "Human","ComputerLow","ComputerMiddle","ComputerHigh" };
	std::vector<std::string> size_list = { "8","4","6","10","12","14","16" };
}
std::string bw::othello::charmap::operator[](const int& col) {
	std::string ret;
	if (stl == symb) {
		switch (col) {
		case 0:
			ret = symbols::col0;
			break;
		case 1:
			ret = symbols::col1;
			break;
		case -1:
			ret = symbols::none;
			break;
		}
	}
	if (stl == numb) {
		switch (col) {
		case 0:
			ret = numbers::col0;
			break;
		case 1:
			ret = numbers::col1;
			break;
		case -1:
			ret = numbers::none;
			break;
		}
	}
	return ret;
}
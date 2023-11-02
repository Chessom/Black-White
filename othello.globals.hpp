#pragma once
#include"stdafx.h"
namespace bw::othello {
	namespace symbols {
		extern std::string col0;
		extern std::string col1;
		extern std::string none;
	}
	namespace numbers {
		extern std::string col0;
		extern std::string col1;
		extern std::string none;
	}
	enum { symb, numb };

	class charmap {
	public:
		charmap() = default;
		charmap(int st) :stl(st) {};
		std::string operator[](const int& col);
		int& current() {
			return stl;
		}
	private:
		int stl = symb;
	};
	extern charmap CharMap;
	extern std::vector<std::string> gamer_list;
	extern std::vector<std::string> size_list;
}
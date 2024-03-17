#pragma once
#include"stdafx.h"
#include"core.hpp"
using std::string;
namespace bw {
	class basic_gamer {//gamer的基础数据
	public:
		enum { human, computer, remote, invalid };
		basic_gamer() = default;
		basic_gamer(core::color Color, int ID = 0, const string& Name = "", int GamerType = invalid, int Gametype = core::gameid::unspecific)
			:col(Color), id(ID), name(Name), gamertype(GamerType), gametype(Gametype) {
		}
		virtual ~basic_gamer() = default;
		string name = "";
		core::color col = core::col0;
		int id = 0;
		int gamertype = invalid;
		int gametype = core::gameid::unspecific;
	};
	
	using basic_gamer_ptr = std::shared_ptr<basic_gamer>;
	using basic_gamer_info = basic_gamer;

	REFLECTION(basic_gamer, name, col, id, gamertype, gametype);
}
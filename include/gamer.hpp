#pragma once
#include"stdafx.h"
#include"core.hpp"
namespace bw {
	using std::string;
	class basic_gamer {//gamer的基础数据
	public:
		enum { human, computer, remote, online, external, invalid };//作为基础类别
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
		bool is_human() const {
			return gamertype == human;
		}
		bool is_computer() const {
			return gamertype == computer;
		}
		bool is_remote() const {
			return gamertype == remote;
		}
	};
	
	using basic_gamer_ptr = std::shared_ptr<basic_gamer>;
	using basic_gamer_info = basic_gamer;

	REFLECTION(basic_gamer, name, col, id, gamertype, gametype);
}
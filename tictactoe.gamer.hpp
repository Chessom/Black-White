#pragma once
#include"gamer.hpp"
#include"tictactoe.move.hpp"
#include"tictactoe.board.hpp"
namespace bw::tictactoe {
	struct gamer :bw::basic_gamer {
		gamer() { gametype = core::gameid::tictactoe;; };
		gamer(core::color Color, int ID = 0, const std::string& Name = "", int GamerType = invalid)
			:basic_gamer(Color, ID, Name, GamerType, core::gameid::tictactoe) {
		};
		virtual move getmove(const board& brd, core::color col) = 0;
	};
}
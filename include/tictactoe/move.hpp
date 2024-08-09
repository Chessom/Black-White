#pragma once
#include"core.hpp"
namespace bw::tictactoe {
	struct move {
		enum { mv, pass, lose, win, draw, suspend, regret, quit, invalid, str, start, prepared };
		int mvtype = invalid;
		core::coord crd = {};
		core::color col = core::none;
	};
	REFLECTION(move, mvtype, crd, col);
}
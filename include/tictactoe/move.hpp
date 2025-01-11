#pragma once
#include"core.hpp"
namespace bw::tictactoe {
	struct move {
		enum { mv, lose, win, draw, suspend, regret, quit, invalid, str, start, prepared };
		int mvtype = invalid;
		core::coord crd = {};
		core::color col = core::none;
	};
	YLT_REFL(move, mvtype, crd, col);
}
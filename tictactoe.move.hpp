#pragma once
#include"core.hpp"
namespace bw::tictactoe {
	struct move {
		enum { mv, pass, lose, win, draw, suspend, regret, quit, invalid, str, start, preparaed };
		int mvtype;
		core::coord crd;
		core::color col;
	};
}
#pragma once
#include"stdafx.h"
#include"core.hpp"
namespace bw::othello {
	using namespace bw::core;
	enum { max_move_num = 100 };
	struct move {
		enum { mv, pass, lose, win, draw, suspend, regret, quit, invalid, str, start };
		int mvtype;
		coord pos;
		color c;
		std::string msg;
	};
	REFLECTION(move, mvtype, pos, c, msg);
};

template<class Archive>
void serialize(Archive& archive, bw::othello::move& m) {
	archive(m.mvtype, m.pos, m.c, m.msg);
}
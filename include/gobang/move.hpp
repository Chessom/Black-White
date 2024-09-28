#pragma once
#include"stdafx.h"
#include"core.hpp"
namespace bw::gobang {
	using namespace bw::core;
	enum { max_move_num = 100 };
	struct move {
		enum { mv, lose, win, draw, suspend, regret, quit, invalid, str, start, prepared, brd };
		int mvtype = invalid;
		coord pos;
		color c = core::none;
		std::string msg;
	};
	REFLECTION(move, mvtype, pos, c, msg);
};

template<class Archive>
void serialize(Archive& archive, bw::gobang::move& m) {
	archive(m.mvtype, m.pos, m.c, m.msg);
}
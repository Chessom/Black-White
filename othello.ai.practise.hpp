#pragma once
#include"stdafx.h"
#include"othello.board.hpp"
namespace bw::othello::ai {
	template<int Size>
	auto generate_random_board(int step) {
		static_assert(Size <= 8, "Size must be less than 8.");
		bitbrd_t<Size> brd;
		std::random_device e;
		brd.initialize();

		color col = col0;
		color op = 0;
		ull mvs = 0;
		srand(e());
		for (int i = 0; i < step; ++i) {
			op = col ^ 1;
			mvs = brd.getmoves(col);
			if (!mvs) {
				mvs = brd.getmoves(op);
				if (!mvs) {
					break;
				}
				col = op;
				op = col ^ 1;
				--i;
				continue;
			}
			auto t = rand() % std::popcount(mvs);
			for (int i = 0; i < t; ++i) {
				mvs = mvs & (mvs - 1);
			}
			brd.applymove(bitbrd::bit2crd(mvs & (-ll(mvs))), col);
			col = op;
		}
		return brd;
	}
	void test_convergence(bitbrd, int, int);
	void full_test();
}
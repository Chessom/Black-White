﻿#pragma once
#include"stdafx.h"
#include"othello/board.hpp"
namespace bw::othello::ai {
	template<int Size>
	inline auto generate_random_board(int step) {
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
			brd.apply_move(mvs & (-ll(mvs)), col);
			col = op;
		}
		return brd;
	}
	inline void test_convergence(bitbrd brd, int times, int epoch) {
		std::random_device e;
		int col0_win_cnt = 0;

		color col = col0;
		color op = 0;
		ull mvs = 0;

		for (int i = 1; i <= times; ++i) {
			for (int j = 0; j < epoch; j++)
			{
				srand(e());
				brd.clear();
				brd.initialize();
				col = col0;
				while (true) {
					op = col ^ 1;
					mvs = brd.getmoves(col);
					if (mvs == 0ull) {
						mvs = brd.getmoves(op);
						if (mvs == 0ull) {
							if (brd.countpiece(col0) > brd.countpiece(col1)) {
								++col0_win_cnt;
							}
							break;
						}
						col = op;
						op = col ^ 1;
						continue;
					}
					auto t = rand() % std::popcount(mvs);
					for (int i = 0; i < t; ++i) {
						mvs = mvs & (mvs - 1);
					}
					brd.apply_move(bitbrd::bit2crd(mvs & (-ll(mvs))), col);
					col = op;
				}
			}
			std::print("time:{:6} total:{:6} col0_win_cnt:{:6} frequency:{:.4}\n", i, epoch * i, col0_win_cnt, double(col0_win_cnt) / double(epoch * i));
		}
	}
	inline void full_test() {
		int step, times, epoch;
		while (true) {
			std::cin >> step;
			if (step < 0)break;
			auto brd = generate_random_board<8>(step);
			std::print("{}\n", brd);
			std::cin >> times >> epoch;
			test_convergence(brd, times, epoch);
			std::print("----------Done---------\n");
		}
	}
}
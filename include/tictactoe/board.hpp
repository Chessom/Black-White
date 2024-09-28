#pragma once
#include"stdafx.h"
#include"core.hpp"
#include"tictactoe/move.hpp"
namespace bw::tictactoe {
#define EQUALLINE(x) mat[x][0]!=core::none&&mat[x][0]==mat[x][1]&&mat[x][0]==mat[x][2]
#define EQUALCOL(y) mat[0][y]!=core::none&&mat[0][y]==mat[1][y]&&mat[0][y]==mat[2][y]
	class board {
	public:
		const static int size = 3;
		board() { initialize(); };
		void initialize() {
			for (int i = 0; i < 3; ++i) {
				for (int j = 0; j < 3; ++j) {
					mat[i][j] = core::none;
				}
			}
		}
		void set_col(const core::coord& crd, core::color col) {
			mat[crd.x][crd.y] = col;
		}
		core::color get_col(const core::coord& crd) {
			return mat[crd.x][crd.y];
		}
		void apply_move(const core::coord& crd, core::color col) {
			set_col(crd, col);
			++cnt;
		}
		bool check_move(const core::coord& crd, core::color col) const {
			return cnt != 3 * 3 && mat[crd.x][crd.y] == core::none;
		}
		core::color check_winner() const {
			if (EQUALLINE(0)) {
				return mat[0][0];
			}
			if (EQUALLINE(1)) {
				return mat[1][0];
			}
			if (EQUALLINE(2)) {
				return mat[2][0];
			}
			if (EQUALCOL(0)) {
				return mat[0][0];
			}
			if (EQUALCOL(1)) {
				return mat[0][1];
			}
			if (EQUALCOL(2)) {
				return mat[0][2];
			}
			if (mat[1][1] != core::none && mat[0][0] == mat[1][1] && mat[0][0] == mat[2][2]) {
				return mat[1][1];
			}
			if (mat[1][1] != core::none && mat[2][0] == mat[1][1] && mat[1][1] == mat[0][2]) {
				return mat[1][1];
			}
			return core::none;
		}
		int cnt = 0;
		char mat[3][3];
	};
#undef EQUALLINE
#undef EQUALCOL
}
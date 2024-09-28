#pragma once
#include"core.hpp"
namespace bw::gobang {
	struct board {
	public:
		const static int size = 15;
		const static int cnct_size = 5;//connect size
		board() { initialize(); };
		void initialize() {
			for (int i = 0; i < size * size; ++i) {
				mat[i] = core::none;
			}
		}
		void set_col(const core::coord& crd, core::color col) {
			mat[crd.x * size + crd.y] = col;
		}
		core::color get_col(const core::coord& crd) const {
			return mat[crd.x * size + crd.y];
		}
		void apply_move(const core::coord& crd, core::color col) {
			using namespace core;
			using drc_t = int;
			set_col(crd, col);
			++cnt;
			coord iter;
			for (drc_t drc = directions::R; drc <= directions::DL; ++drc) {
				drc_t op_drc = op_direction(drc);
				int cnt0 = 0, cnt1 = 0;
				iter = crd;
				while (in_board(iter.to_next(drc))) {
					if (get_col(iter) == col) {
						++cnt0;
					}
					else {
						break;
					}
				}
				iter = crd;
				while (in_board(iter.to_next(op_drc))) {
					if (get_col(iter) == col) {
						++cnt1;
					}
					else {
						break;
					}
				}
				if (cnt0 + 1 + cnt1 >= cnct_size) {
					winner = col;
					break;
				}
			}
		}
		void apply_move_dry(const core::coord& crd, core::color col) {
			set_col(crd, col);
			++cnt;
		}
		bool check_move(const core::coord& crd, core::color col) const {
			return cnt != size * size && mat[crd.x * size + crd.y] == core::none;
		}
		bool in_board(const core::coord& crd) const {
			return crd.x >= 0 && crd.x < size && crd.y >= 0 && crd.y < size;
		}
		core::color get_winner() const {
			return winner;
		}
	private:
		int cnt = 0;
		core::color winner = core::none;
		core::color mat[size * size];
	};
}
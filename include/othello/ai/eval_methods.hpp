#pragma once
#include"common.hpp"
namespace bw::othello::ai {
	struct traits_eval {
		template<int BoardSize>
		static float eval_board(const bitbrd_t<BoardSize>& brd, color setter, color player) {
			float value = 0.0f;

			// 行动力
			int player_moves = std::popcount(brd.getmoves(player));
			int opponent_moves = std::popcount(brd.getmoves(op_col(player)));
			value += (player_moves - opponent_moves) * 5;
			if (setter == player) {
				return value;
			}
			else {
				return -value;
			}
		}
		template<>
		static float eval_board<8>(const bitbrd_t<8>& brd, color setter, color player) {
			float value = 0.0f;

			// 位置优势
			static const int position_weights[8][8] = {
				{40, -3, 11, 8, 8, 11, -3, 40},
				{-3, -10, -4, 1, 1, -4, -10, -3},
				{11, -4, 2, 2, 2, 2, -4, 11},
				{8, 1, 2, -3, -3, 2, 1, 8},
				{8, 1, 2, -3, -3, 2, 1, 8},
				{11, -4, 2, 2, 2, 2, -4, 11},
				{-3, -10, -4, 1, 1, -4, -10, -3},
				{40, -3, 11, 8, 8, 11, -3, 40}
			};

			for (int i = 0; i < 8; ++i) {
				for (int j = 0; j < 8; ++j) {
					color col = brd.getcol({ i, j });
					if (col == player) {
						value += position_weights[i][j];
					}
					else if (col == op_col(player)) {
						value -= position_weights[i][j];
					}
				}
			}

			// 行动力
			int player_moves = std::popcount(brd.getmoves(player));
			int opponent_moves = std::popcount(brd.getmoves(op_col(player)));
			value += (player_moves - opponent_moves) * 10;

			if (setter == player) {
				return value;
			}
			else {
				return -value;
			}
		}
		template<int BoardSize>
		static float eval_board(const arrbrd_t<BoardSize>& brd, color setter, color player) {
			auto value = moves::calculate_size(brd, setter);
			if (setter == player) {
				return value;
			}
			else {
				return -value;
			}
		}
		template<int BoardSize>
		static float eval_end_game_board(const bitbrd_t<BoardSize>& brd, color player) {
			return brd.countpiece(player) - brd.countpiece(op_col(player)) * 15;
		}
		template<int BoardSize>
		static float eval_end_game_board(const arrbrd_t<BoardSize>& brd, color player) {
			return brd.countpiece(player) - brd.countpiece(op_col(player)) * 15;
		}
	};
}
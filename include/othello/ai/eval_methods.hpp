#pragma once
#include"common.hpp"
namespace bw::othello::ai {
	template<typename value_type = float>
	struct traits_eval {
		using val_t = value_type;
		static const inline value_type position_weights[8][8] = {
				{40, -3, 11, 8, 8, 11, -3, 40},
				{-3, -10, -4, 1, 1, -4, -10, -3},
				{11, -4, 2, 2, 2, 2, -4, 11},
				{8, 1, 2, -3, -3, 2, 1, 8},
				{8, 1, 2, -3, -3, 2, 1, 8},
				{11, -4, 2, 2, 2, 2, -4, 11},
				{-3, -10, -4, 1, 1, -4, -10, -3},
				{40, -3, 11, 8, 8, 11, -3, 40}
		};
		template<int BoardSize>
		value_type eval_board(const bitbrd_t<BoardSize>& brd, color setter, color player) {
			value_type value = 0;

			// 行动力
			int player_moves = std::popcount(brd.getmoves(player));
			int opponent_moves = std::popcount(brd.getmoves(op_col(player)));
			value += (player_moves - opponent_moves) * 5;
			return value;
		}
		template<>
		value_type eval_board<8>(const bitbrd_t<8>& brd, color setter, color player) {
			value_type value = 0;

			// 位置优势
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
			return value;
		}
		template<int BoardSize>
		value_type eval_board(const arrbrd_t<BoardSize>& brd, color setter, color player) {
			value_type value = moves::calculate_size(brd, setter);
			return value;
		}
		template<int BoardSize>
		value_type eval_end_game_board(const bitbrd_t<BoardSize>& brd, color player) {
			value_type value = 0;
			for (int i = 0; i < BoardSize; ++i) {
				for (int j = 0; j < BoardSize; ++j) {
					color col = brd.getcol({ i, j });
					if (col == player) {
						value += position_weights[i][j];
					}
					else if (col == op_col(player)) {
						value -= position_weights[i][j];
					}
				}
			}
			return value + brd.countpiece(player) - brd.countpiece(op_col(player)) * 15;
		}
		template<int BoardSize>
		value_type eval_end_game_board(const arrbrd_t<BoardSize>& brd, color player) {
			return brd.countpiece(player) - brd.countpiece(op_col(player)) * 15;
		}
	};
	template<typename value_type = float>
	struct nn_eval {
		using val_t = value_type;
		template<int BoardSize>
		value_type eval_board(const bitbrd_t<BoardSize>& brd, color setter, color player) {
			value_type value = 0;

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
		value_type eval_board<8>(const bitbrd_t<8>& brd, color setter, color player) {
			value_type value = 0;

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
		value_type eval_board(const arrbrd_t<BoardSize>& brd, color setter, color player) {
			auto value = moves::calculate_size(brd, setter);
			if (setter == player) {
				return value;
			}
			else {
				return -value;
			}
		}
		template<int BoardSize>
		value_type eval_end_game_board(const bitbrd_t<BoardSize>& brd, color player) {
			return brd.countpiece(player) - brd.countpiece(op_col(player)) * 15;
		}
		template<int BoardSize>
		value_type eval_end_game_board(const arrbrd_t<BoardSize>& brd, color player) {
			return brd.countpiece(player) - brd.countpiece(op_col(player)) * 15;
		}
	};
}
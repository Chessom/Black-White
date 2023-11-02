#pragma once
#include"stdafx.h"
#include"othello.gamer.computer.hpp"
namespace bw::othello::ai {
	struct evaluator {
		template<int BoardSize = 8>
		int simple_evaluate(const static_brd<BoardSize>& brd, color c) {
			mvs.update(brd, c);
			return -mvs.size;
		}
		int simple_evaluate_dynamic(const dynamic_brd& brd, color c) {
			switch (brd.size) {
			case 4:return simple_evaluate<4>(static_brd<4>(brd), c);
			case 6:return simple_evaluate<6>(static_brd<6>(brd), c);
			case 8:return simple_evaluate<8>(static_brd<8>(brd), c);
			case 10:return simple_evaluate<10>(static_brd<10>(brd), c);
			case 12:return simple_evaluate<12>(static_brd<12>(brd), c);
			case 14:return simple_evaluate<14>(static_brd<14>(brd), c);
			case 16:return simple_evaluate<16>(static_brd<16>(brd), c);
			}
		}
		template<int BoardSize>
		inline int single_evaluate(const static_brd<BoardSize>& brd, color c) {
			int ret = 0;
			mvs.update(brd, c ^ 1);
			int numcol = brd.countpiece(c), numopcol = brd.countpiece(c ^ 1);
			if (numcol + numopcol == BoardSize * BoardSize) {
				ret += (numcol - numopcol) * 10;//need param

			}
			else {
				ret -= mvs.size * 10;
				auto p=
				ret += (count_corner(brd,c) - count_corner(brd,c ^ 1)) * 10;

				//stability
				//mobility
				//position
			}
			return ret;
		}

		template<int BoardSize>
		int recursive_alphabeta_evaluate(const static_brd<BoardSize>& brd, color c) {

			return 0;
		}
		template<int BoardSize>
		int loop_alphabeta_evaluate(const static_brd<BoardSize>& brd, color c) {

			return 0;
		}
		template<int BoardSize>
		int minmax_evaluate(static_brd<BoardSize>& brd, color c) {

			return 0;
		}

		moves mvs;
	private:
		template<int BoardSize>
		inline int count_corner(const static_brd<BoardSize>& brd, color c) {
			return (brd.getcol({ 0,0 }) == c) + (brd.getcol({ 0,BoardSize - 1 }) == c) + (brd.getcol({ BoardSize - 1,0 }) == c) + (brd.getcol({ BoardSize - 1,BoardSize - 1 }) == c);
		}
	};
	
}
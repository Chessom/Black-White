#pragma once
#include"stdafx.h"
#include"othello/gamer/computer.hpp"
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
			return 0;
		}
		//template<int BoardSize>
		//inline int single_evaluate(const static_brd<BoardSize>& brd, color c) {
		//	int ret = 0;
		//	mvs.update(brd, c ^ 1);
		//	int numcol = brd.countpiece(c), numopcol = brd.countpiece(c ^ 1);
		//	if (numcol + numopcol == BoardSize * BoardSize) {
		//		ret += (numcol - numopcol) * 10;//need para
		//	}
		//	else {
		//		ret -= mvs.size * 10;
		//		auto p=
		//		ret += (count_corner(brd,c) - count_corner(brd,c ^ 1)) * 10;
		//		//stability
		//		//mobility
		//		//position
		//	}
		//	return ret;
		//}
		
		template<int BoardSize>
		int recursive_alphabeta_evaluate(const static_brd<BoardSize>& brd, color setter_color, int depth) {
			mvs.update(brd, setter_color);
			auto brd_t = brd;
			
			if (!mvs.size) {
				for (int i = 0; i < mvs.size; ++i) {
					brd.applymove(mvs.coords[i], setter_color);
					


					brd = brd_t;
				}
			}
			return 0;
		}
		template<int BoardSize>
		int loop_alphabeta_evaluate(const static_brd<BoardSize>& brd, color c) {

			return 0;
		}
		template<int BoardSize>
		int minmax_evaluate(static_brd<BoardSize>& brd, color setter_color, int depth) {
			mvs.update(brd, setter_color);
			auto brd_t = brd;
			int i = 0;
			int max_v = -99999;
			if (!mvs.size) {
				for (int i = 0; i < mvs.size; ++i) {
					brd.applymove(mvs.coords[i], setter_color);
					


					brd = brd_t;
				}
			}
			else {
				moves mvs_op;
				mvs_op.update(brd, setter_color ^ 1);
				if (!mvs.size) {

				}
			}
			return 0;
		}
		/*
		double AlphaBeta(Board board, condition& setter, condition& adversary, const coordinate& chess, int depth, double a, double b) {
	if (depth == 0 || setter.havenoposset() && adversary.havenoposset()) {
		double temp = 0;
		if (setter.ga == computer)
			return temp = value(board, setter, adversary, chess, setter.returncolor(), para, false);
		else
			return temp = value(board, adversary, setter, chess, adversary.returncolor(), para, false);
		if (aishow)
			cout << multistr("	", ::depth - depth + 1) << chess << ":" << temp << endl;
	}
	std::set<int> t;
	if (setter.ga == human) {
		condition compu, huma;
		if (setter.havenoposset()) {
			return value();
		}
		else {
			Board bo = board;
			vector<chessvalue> posn;
			bool presort = setter.nposset() > 4;
			for (auto& node : setter.posn) {
				if (presort)
					posn.push_back(chessvalue(node.chess, prevalue(board, node.chess, setter.returncolor(), setter.ga)));
				else
					posn.push_back(chessvalue(node.chess, 0));
			}
			if (presort)
				sort(posn.begin(), posn.end(), [](chessvalue& c1, chessvalue& c2) {return c1.value < c2.value; });//预搜索排序
			for (auto& node : posn) {
				nodes++;
				board.setchess(node.chess, setter.returncolor(), t);
				compu.update(board, oppocolor(setter.returncolor()), computer); huma.update(board, setter.returncolor(), human);
				b = min(b, value(board, compu, huma, node.chess, huma.returncolor(), para, false) + AlphaBeta(board, compu, huma, node.chess, depth - 1, a, b));
				if (aishow)
					cout << multistr("	", ::depth - depth + 1) << node.chess << ":" << b << endl;
				if (b <= a)
					return b;
				board = bo;
			}
			return b;
		}
	}
	else {
		condition huma, compu;
		if (setter.havenoposset()) {
			return value(board, setter, adversary, chess, adversary.returncolor(), para, false) + AlphaBeta(board, adversary, setter, chess, depth - 1, a, b);
		}
		else {
			//叶节点估值辅助操作
			Board bo = board;
			vector<chessvalue> posn;//到叶节点的路径的集合
			bool presort = setter.nposset() > 4;
			
		for (auto& node : setter.posn) {
			if (presort)
				posn.push_back(chessvalue(node.chess, prevalue(board, node.chess, setter.returncolor(), setter.ga)));
			else
				posn.push_back(chessvalue(node.chess, 0));
		}
		if (presort)
			sort(posn.begin(), posn.end(), [](chessvalue& c1, chessvalue& c2) {return c1.value > c2.value; });//预搜索排序

		for (auto& node : posn)//查看每个叶节点
		{
			nodes++;
			board.setchess(node.chess, setter.returncolor(), t);//试下该棋子以对叶节点进行估值
			huma.update(board, oppocolor(setter.returncolor()), human); compu.update(board, setter.returncolor(), computer);//重新计算
			a = max(a, value(board, compu, huma, node.chess, compu.returncolor(), para, false) + AlphaBeta(board, huma, compu, node.chess, depth - 1, a, b));//递归估值并且找最大
			if (aishow)
				cout << multistr("	", ::depth - depth + 1) << node.chess << ":" << a << endl;
			if (a >= b)
				return a;
			board = bo;
		}
		return a;
	}
}
}*/
		moves mvs;

	private:
		template<int BoardSize>
		inline int count_corner(const static_brd<BoardSize>& brd, color c) {
			return (brd.getcol({ 0,0 }) == c) + (brd.getcol({ 0,BoardSize - 1 }) == c) + (brd.getcol({ BoardSize - 1,0 }) == c) + (brd.getcol({ BoardSize - 1,BoardSize - 1 }) == c);
		}
	};
}
#pragma once
#include"stdafx.h"
#include"core.hpp"
#include"tictactoe.board.hpp"
#include"tictactoe.move.hpp"
namespace bw::tictactoe {
	using namespace bw::core;

	class moves {
	public:
		enum { max_move_num = 9 };
		moves() = default;
		moves(const board& brd, const color& col) {
			update(brd, col);
		}
		void update(const board& brd, const color& col) {
			clear();
			for (int x = 0; x < brd.size; ++x) {
				for (int y = 0; y < brd.size; y++)
				{
					if (brd.checkmove({ x,y }, col)) {
						push_back({ x,y });
					}
				}
			}
		}
		void push_back(const coord& crd) {
			if (size == max_move_num)throw std::range_error("Go beyond the max_move_num!");
			coords[size++] = crd;
		}
		bool empty() const noexcept {
			return size == 0;
		}
		void clear() noexcept {
			size = 0;
		}
		int find(const coord& crd) {
			for (int i = 0; i < size; ++i) {
				if (coords[i] == crd) {
					return i;
				}
			}
			return npos;
		}

		int size = 0;
		static const int npos = -1;
		coord coords[max_move_num] = {};
	};
}
namespace std {
	template<typename CharT>
	struct formatter<bw::tictactoe::moves, CharT> {
		bool number = true;
		constexpr auto parse(auto& context) {
			auto it = context.begin(), end = context.end();
			if (it == end || *it == '}') {
				return it;
			}
			while (*it != '}' && it != end) {
				switch (*it) {
				case 'n':
					number = true;
					break;
				case 'c':
					number = false;
					break;
				case '}':
					break;
				default:
					throw std::format_error("Invalid KeyValue format specifier.");
				}
				++it;
			}
			return it;
		}
		template<typename FormatContext>
		auto format(const bw::tictactoe::moves& mvs, FormatContext& fc) {
			if (mvs.empty())return fc.out();
			if (number) {
				std::format_to(fc.out(), "{}", mvs.coords[0]);
				for (int i = 1; i < mvs.size; ++i) {
					std::format_to(fc.out(), " ({},{})", mvs.coords[i].x, mvs.coords[i].y);
				}
			}
			else {
				std::format_to(fc.out(), "{}{}", char(mvs.coords[0].y + 'A'), char(mvs.coords[0].x + '1'));
				for (int i = 1; i < mvs.size; ++i) {
					std::format_to(fc.out(), " {}{}", char(mvs.coords[i].y + 'A'), char(mvs.coords[i].x + '1'));
				}
			}
			return fc.out();
		};
	};
}
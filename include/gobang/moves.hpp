#pragma once
#include"stdafx.h"
#include"core.hpp"
#include"gobang/board.hpp"
#include"gobang/move.hpp"
namespace bw::gobang {
	using namespace bw::core;
	class moves {
	public:
		moves() = default;
		moves(const board& brd, const color& col) {
			update(brd, col);
		}
		void update(const board& brd, const color& col) {
			clear();
			
		}
		template<int Size>
		static int calculate_size(const board& brd, const color& col) {
			return 1;
		}
		void push_back(const coord& crd) {
			coords.push_back(crd);
			++size;
		}
		bool empty() const noexcept {
			return coords.empty();
		}
		void clear() noexcept {
			coords.clear();
			size = 0;
		}
		int find(const coord& crd) const {
			for (int i = 0; i < coords.size(); ++i) {
				if (coords[i] == crd) {
					return i;
				}
			}
			return npos;
		}
		coord get_crd(int index) const {
			return coords[index];
		}

		template<typename Archive>
		void serialize(Archive& ar) {
			ar(size);
			for (int i = 0; i < size; ++i) {
				ar(coords[i]);
			}
		}
		enum { npos = -1 };
		int size = 0;
		std::vector<coord> coords;
	};
	template<int BoardSize>
	using fast_moves = typename std::conditional<(BoardSize > 8), std::vector<coord>, uint64_t>::type;

	template<int BoardSize>
	bool empty_moves(const fast_moves<BoardSize>& mvs) {
		if constexpr (BoardSize > 8) {
			return mvs.empty();
		}
		else {
			return !mvs;
		}
	};
}
namespace std {
	template<typename CharT>
	struct formatter<bw::gobang::moves, CharT> {
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
		auto format(const bw::gobang::moves& mvs, FormatContext& fc) const {
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
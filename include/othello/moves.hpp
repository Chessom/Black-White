#pragma once
#include"stdafx.h"
#include"core.hpp"
#include"othello/board.hpp"
#include"othello/move.hpp"
namespace bw::othello {
	using namespace bw::core;
namespace v1{
	class moves {
	public:
		moves() = default;
		template<int Size>
		moves(const bitbrd_t<Size>& brd, const color& col) {
			update(brd, col);
		}
		template<mat_brd board>
		moves(const board& brd, const color& col) {
			update(brd, col);
		}
		template<int Size>
		void update(const bitbrd_t<Size>& brd, const color& col) {
			clear();
			constexpr ull mask = (1ull << (Size * Size)) - 1ull;
			ull moves = brd.getmoves(col) & mask;
			bititer iter = 0ull;
			for (; moves; moves = moves & (moves - 1)) {
				iter = moves & (-ll(moves));
				coords[size++] = std::move(bitbrd_t<Size>::bit2crd(iter));
			}
		}
		template<mat_brd board>
		void update(const board& brd, const color& col) {
			clear();
			using drc_t = int;
			using namespace directions;
			color opcol = col ^ 1;
			auto Size = brd.brd_size();
			for (int x = 0; x < Size; ++x) {
				for (int y = 0; y < Size; ++y) {
					coord crd{ x,y };
					if (brd.getcol(crd) == none) {
						bool done = false;
						for (drc_t drc = R; drc <= UR && !done; ++drc) {
							coord iter(crd);
							if (brd.in_board(iter.to_next(drc)) && brd.getcol(iter) == opcol) {
								color c = 0;
								while (brd.in_board(iter.to_next(drc))) {
									c = brd.getcol(iter);
									if (c == col) {
										push_back(crd);
										done = true;
										break;
									}
									else if (c == none) {
										break;
									}
								}
							}
						}
					}
				}
			}
		}
		void push_back(const coord& crd) {
			if (size == max_move_num)throw std::range_error("Go beyond the max_move_num!");
			coords[size++] = crd;
		}
		template<int Size>
		ull to_bitmoves() const {
			static_assert(Size <= 8, "Size must be less than 8.");
			ull rt = 0ull;
			for (int i = 0; i < size; ++i) {
				rt |= (bitbrd_t<Size>::to_bititer(coords[i]));
			}
			return rt;
		}
		bool empty() const noexcept {
			return size == 0;
		}
		void clear() noexcept {
			size = 0;
		}
		int find(const coord& crd) const {
			for (int i = 0; i < size; ++i) {
				if (coords[i] == crd) {
					return i;
				}
			}
			return npos;
		}

		template<typename Archive>
		void serialize(Archive& ar) {
			ar(size);
			for (int i = 0; i < size; ++i) {
				ar(coords[i]);
			}
		}

		int size = 0;
		static const int npos = -1;
		coord coords[max_move_num] = {};
	};
}
inline namespace v2{
	class moves {
	public:
		moves() = default;
		template<int Size>
		moves(const bitbrd_t<Size>& brd, const color& col) {
			update(brd, col);
		}
		template<mat_brd Board>
		moves(const Board& brd, const color& col) {
			update(brd, col);
		}
		template<int Size>
		void update(const bitbrd_t<Size>& brd, const color& col) {
			clear();
			constexpr ull mask = (1ull << (Size * Size)) - 1ull;
			ull moves = brd.getmoves(col) & mask;
			bititer iter = 0ull;
			for (; moves; moves = moves & (moves - 1)) {
				iter = moves & (-ll(moves));
				coords[size++] = std::move(bitbrd_t<Size>::bit2crd(iter));
			}
		}
		template<mat_brd board>
		void update(const board& brd, const color& col) {
			clear();
			using drc_t = int;
			using namespace directions;
			color opcol = col ^ 1;
			auto Size = brd.brd_size();
			for (int x = 0; x < Size; ++x) {
				for (int y = 0; y < Size; ++y) {
					coord crd{ x,y };
					if (brd.getcol(crd) == none) {
						bool done = false;
						for (drc_t drc = R; drc <= UR && !done; ++drc) {
							coord iter(crd);
							if (brd.in_board(iter.to_next(drc)) && brd.getcol(iter) == opcol) {
								color c = 0;
								while (brd.in_board(iter.to_next(drc))) {
									c = brd.getcol(iter);
									if (c == col) {
										push_back(crd);
										done = true;
										break;
									}
									else if (c == none) {
										break;
									}
								}
							}
						}
					}
				}
			}
		}
		void push_back(const coord& crd) {
			coords.push_back(crd);
			++size;
		}
		template<int Size>
		ull to_bitmoves() const {
			static_assert(Size <= 8, "Size must be less than 8.");
			ull rt = 0ull;
			for (int i = 0; i < size; ++i) {
				rt |= (bitbrd_t<Size>::to_bititer(coords[i]));
			}
			return rt;
		}
		bool empty() const noexcept {
			return coords.empty();
		}
		void clear() noexcept {
			coords.clear();
			size = 0;
		}
		int find(const coord& crd) {
			for (int i = 0; i < coords.size(); ++i) {
				if (coords[i] == crd) {
					return i;
				}
			}
			return npos;
		}
		coord get_crd(int index) {
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
};
}
namespace std {
	template<typename CharT>
	struct formatter<bw::othello::moves, CharT> {
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
		auto format(const bw::othello::moves& mvs, FormatContext& fc) {
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
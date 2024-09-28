#pragma once
#include"stdafx.h"
#include"core.hpp"
#include"utility.hpp"
#include"ataxx/move.hpp"
#include"globals.hpp"
namespace bw::ataxx {
	enum { default_size = 7, max_size = 11 };
	using std::vector;
	using namespace core;

	template<int Size, std::integral PosType = int_fast8_t>
	class arrbrd_t;
	template<int Size>
	class bitbrd_t;

	struct dynamic_brd {
		using pos_type = uint_fast8_t;
		template<int Size, std::integral PosType>
		friend class arrbrd_t;
		template<int Size>
		friend class bitbrd_t;
		dynamic_brd() {
			size = default_size;
			mat.resize(size * size);
		}
		dynamic_brd(int Size) {
			size = Size;
			mat.resize(size * size);
		}
		bool in_board(const coord& crd) const {
			return (crd.x < size && crd.y < size && crd.x >= 0 && crd.y >= 0);
		}
		color get_col(const coord& crd)const {
			return mat[crd.x * size + crd.y] - 1;
		}
		void set_col(const coord& crd, color col) {
			mat[crd.x * size + crd.y] = col + 1;
		}
		void initialize() {
			set_col({ size / 2 - 1,size / 2 - 1 }, col1);
			set_col({ size / 2 ,size / 2 }, col1);
			set_col({ size / 2 - 1,size / 2 }, col0);
			set_col({ size / 2 ,size / 2 - 1 }, col0);
		}
		void apply_move(const coord& crd, const color& col) {
			using drc_t = int;
			using namespace directions;
			color opcol = col ^ 1;
			for (drc_t drc = R; drc <= UR; ++drc) {
				coord iter = crd;
				bool flag = false;
				int times = 0;
				if (in_board(iter.to_next(drc)) && get_col(iter) == opcol) {
					color c = 0;
					while (in_board(iter.to_next(drc))) {
						++times;
						c = get_col(iter);
						if (c == col) {
							flag = true;
							break;
						}
						else if (c == none) {
							break;
						}
					}
				}
				if (flag) {
					iter = crd;
					for (int i = 0; i < times; ++i) {
						iter.to_next(drc);
						mat[iter.x * size + iter.y] ^= 0b11;
					}
				}
			}
			set_col(crd, col);
		}
		int countpiece(color col) {
			int cnt = 0, t = col + 1;
			for (int x = 0; x < size; ++x) {
				for (int y = 0; y < size; ++y) {
					if (mat[x * size + y] == t) ++cnt;
				}
			}
			return cnt;
		}
		int count(color col) {
			int cnt = 0, t = col + 1;
			for (int x = 0; x < size; ++x) {
				for (int y = 0; y < size; ++y) {
					if (mat[x * size + y] == t) ++cnt;
				}
			}
			return cnt;
		}
		void resize(int newsize) {
			mat.resize(newsize * newsize);
			size = newsize;
		}
		int brd_size() const {
			return size;
		}
		void clear() {
			for (int i = 0; i < mat.size(); ++i) {
				for (int j = 0; j < mat.size(); ++j) {
					mat[i * size + j] = none;
				}
			}
		}
		template <class Archive>
		void serialize(Archive& ar) {
			ar(mat);
		}
		auto& get_underlying_vector() {
			return mat;
		}
		int size = default_size;
	private:
		vector<pos_type> mat;
	};

	template<int Size>
	class bitbrd_t {
		friend class arrbrd_t<Size>;
		//static_assert(Size <= 8, "Bit board size must be less than 8.");
	public:
		bitbrd_t() { brd[0] = brd[1] = ull(0); };
		bitbrd_t(const ull& brd0, const ull& brd1) { brd[0] = brd0; brd[1] = brd1; };
		bitbrd_t(const arrbrd_t<Size>& abrd) noexcept {
			brd[0] = brd[1] = ull(0);
			for (int x = 0; x < Size; ++x) {
				for (int y = 0; y < Size; ++y) {
					if (abrd.mat[x * Size + y]) {
						if constexpr (bw::is_pow2(Size)) {
							brd[abrd.mat[x * Size + y] >> 1] |= 1ull << ((x << bw::log2(Size)) + y);
						}
						else {
							brd[abrd.mat[x * Size + y] >> 1] |= 1ull << (x * Size + y);
						}
					}
				}
			}
		}
		bitbrd_t(const dynamic_brd& dbrd) {
			brd[0] = brd[1] = ull(0);
			for (int x = 0; x < Size; ++x) {
				for (int y = 0; y < Size; ++y) {
					if (dbrd.mat[x * Size + y]) {
						if constexpr (bw::is_pow2(Size)) {
							brd[dbrd.mat[x * Size + y] >> 1] |= 1ull << ((x << bw::log2(Size)) + y);
						}
						else {
							brd[dbrd.mat[x * Size + y] >> 1] |= 1ull << (x * Size + y);
						}
					}
				}
			}
		}
		static constexpr bititer crd2bit(const coord& crd) {
			if constexpr (bw::is_pow2(Size)) {
				return 1ull << ((crd.x << bw::log2(Size)) + crd.y);
			}
			else {
				return 1ull << (crd.x * Size + crd.y);
			}
		}
		static constexpr coord bit2crd(bititer bitcrd) {
			coord crd;
			int cnt = -1;
			for (; bitcrd; bitcrd >>= 1, ++cnt);
			if constexpr (is_pow2(Size)) {
				crd.x = cnt >> bw::log2(Size);
			}
			else {
				crd.x = cnt / 6;
			}
			if constexpr (Size == 8) {
				crd.y = cnt & 0b111;
			}
			else if constexpr (Size == 4) {
				crd.y = cnt & 0b11;
			}
			else if constexpr (Size == 6) {
				crd.y = cnt % 6;
			}
			return crd;
		}
		static uint64_t shift(uint64_t disks, int dir)
		{
			static const uint64_t MASKS[] = {
				0x7F7F7F7F7F7F7F7FULL, /* Right. */
				0x007F7F7F7F7F7F7FULL, /* Down-right. */
				0xFFFFFFFFFFFFFFFFULL, /* Down. */
				0x00FEFEFEFEFEFEFEULL, /* Down-left. */
				0xFEFEFEFEFEFEFEFEULL, /* Left. */
				0xFEFEFEFEFEFEFE00ULL, /* Up-left. */
				0xFFFFFFFFFFFFFFFFULL, /* Up. */
				0x7F7F7F7F7F7F7F00ULL  /* Up-right. */
			};
			static const uint64_t LSHIFTS[] = {
				0, /* Right. */
				0, /* Down-right. */
				0, /* Down. */
				0, /* Down-left. */
				1, /* Left. */
				9, /* Up-left. */
				8, /* Up. */
				7  /* Up-right. */
			};
			static const uint64_t RSHIFTS[] = {
				1, /* Right. */
				9, /* Down-right. */
				8, /* Down. */
				7, /* Down-left. */
				0, /* Left. */
				0, /* Up-left. */
				0, /* Up. */
				0  /* Up-right. */
			};

			if (dir < 8 / 2) {
				return (disks >> RSHIFTS[dir]) & MASKS[dir];
			}
			else {
				return (disks << LSHIFTS[dir]) & MASKS[dir];
			}
		}
		bool in_board(const coord& crd) const noexcept {
			return (crd.x < Size && crd.y < Size && crd.x >= 0 && crd.y >= 0);
		}
		color get_col(const coord& crd) const noexcept {
			auto ful = brd[0] | brd[1];
			ull index = 1ull;
			if constexpr (bw::is_pow2(Size)) {
				index <<= ((crd.x << bw::log2(Size)) + crd.y);
			}
			else {
				index <<= (crd.x * 6 + crd.y);
			}
			if (ful & index) {
				return bool(brd[1] & index);
			}
			else {
				return none;
			}
		}
		void set_col(const coord& crd, color col) noexcept {
			if constexpr (bw::is_pow2(Size)) {
				auto index = 1ull << ((crd.x << bw::log2(Size)) + crd.y);
				brd[col] |= index;
			}
			else {
				auto index = 1ull << (crd.x * Size + crd.y);
				brd[col] |= index;
			}
		}
		void initialize() noexcept {
			//⚪︎⚫︎
			//⚫︎⚪︎
			set_col({ Size / 2 - 1,Size / 2 - 1 }, col1);
			set_col({ Size / 2 ,Size / 2 }, col1);
			set_col({ Size / 2 - 1,Size / 2 }, col0);
			set_col({ Size / 2 ,Size / 2 - 1 }, col0);
		}
		void apply_move(const coord& crd, const color& col) noexcept {
			apply_move(crd2bit(crd), col);
		}
		void apply_move(const uint64_t& bit_crd, const color& col) noexcept {
			
		}
		ull getmoves(color col) const noexcept {
			
		}
		dynamic_brd to_dynamic() const {
			dynamic_brd brd(Size);
			for (int x = 0; x < Size; ++x) {
				for (int y = 0; y < Size; ++y) {
					brd.set_col({ x,y }, get_col({ x,y }));
				}
			}
			return brd;
		}
		int countpiece(color black_or_white) const noexcept {
			return std::popcount(brd[black_or_white]);
		}
		int count(color col) const noexcept {
			if (col == none) {
				return std::popcount(~(brd[0] | brd[1]));
			}
			else {
				return std::popcount(brd[col]);
			}
		}
		constexpr int brd_size() const {
			return Size;
		}
		void clear() {
			brd[col0] = brd[col1] = 0ull;
		}
		bool operator==(const bitbrd_t<Size>& rhs) const noexcept {
			return brd[0] == rhs.brd[0] && brd[1] == rhs.brd[1];
		}
		template <class Archive>
		void serialize(Archive& ar) {
			ar(brd[0], brd[1]);
		}
	private:
		ull brd[2];
		//brd[col0]:board of col0
		//brd[col1]:board of col1
		//true: brd[col0] & brd[col1] == 0
	};
	using bitbrd = bitbrd_t<8>;

	template<int Size, std::integral PosType>
	class arrbrd_t {
		friend class bitbrd_t<Size>;
	public:
		arrbrd_t() = default;
		arrbrd_t(const bitbrd_t<Size>& brd) noexcept {
			static_assert(Size <= 8, "Bit board size must be less than 8.");
			ull bit1 = brd.brd[col1];
			ull ful = brd.brd[col0] | bit1;
			ull it = 0;
			for (int x = 0; x < Size; ++x) {
				for (int y = 0; y < Size; ++y) {
					mat[x * Size + y] = 0;
					if constexpr (bw::is_pow2(Size)) {
						it = 1ull << ((x << bw::log2(Size)) + y);
					}
					else {
						it = 1ull << (x * Size + y);
					}
					if (ful & it) {
						mat[x * Size + y] = bool(bit1 & it) + 1;
					}
				}
			}
		};
		arrbrd_t(const dynamic_brd& dbrd) {
			for (int x = 0; x < Size; ++x) {
				for (int y = 0; y < Size; ++y) {
					mat[x * Size + y] = dbrd.mat[x * Size + y];
				}
			}
		}
		bool in_board(const coord& crd) const noexcept {
			return crd.x < Size && crd.y < Size && crd.x >= 0 && crd.y >= 0;
		}
		color get_col(const coord& crd) const noexcept {
			return mat[crd.x * Size + crd.y] - 1;
		}
		void set_col(const coord& crd, const color& col) noexcept {
			mat[crd.x * Size + crd.y] = col + 1;
		}
		void apply_move(const coord& crd, const color& col) noexcept {
			using drc_t = int;
			using namespace directions;
			color opcol = col ^ 1;
			for (drc_t drc = R; drc <= UR; ++drc) {
				coord iter = crd;
				bool flag = false;//是否为有效的方向
				int times = 0;
				if (in_board(iter.to_next(drc)) && get_col(iter) == opcol) {//存在相邻反色子
					color c = 0;
					while (in_board(iter.to_next(drc))) {
						++times;
						c = get_col(iter);
						if (c == col) {//反色子后是同色子，则条件达成
							flag = true;
							break;
						}
						else if (c == none) {//出现空位，终止
							break;
						}
						//依然为反色子，继续检查
					}
				}
				if (flag) {
					iter = crd;
					for (int i = 0; i < times; ++i) {
						iter.to_next(drc);
						mat[iter.x * Size + iter.y] ^= 0b11;//翻转棋子
					}
				}
			}
			set_col(crd, col);
		}
		void initialize() noexcept {
			//⚪︎⚫︎
			//⚫︎⚪︎
			set_col({ Size / 2 - 1,Size / 2 - 1 }, col1);
			set_col({ Size / 2 ,Size / 2 }, col1);
			set_col({ Size / 2 - 1,Size / 2 }, col0);
			set_col({ Size / 2 ,Size / 2 - 1 }, col0);
		}
		dynamic_brd to_dynamic() const {
			dynamic_brd brd(Size);
			for (int x = 0; x < Size; ++x) {
				for (int y = 0; y < Size; ++y) {
					brd.set_col({ x,y }, get_col({ x,y }));
				}
			}
			return brd;
		}
		int countpiece(color col) const noexcept {
			int cnt = 0, t = col + 1;
			for (int x = 0; x < Size; ++x) {
				for (int y = 0; y < Size; ++y) {
					if (mat[x * Size + y] == t) ++cnt;
				}
			}
			return cnt;
		}
		int count(color col) const noexcept {
			int cnt = 0, t = col + 1;
			for (int x = 0; x < Size; ++x) {
				for (int y = 0; y < Size; ++y) {
					if (mat[x * Size + y] == t) ++cnt;
				}
			}
			return cnt;
		}
		constexpr int brd_size() const {
			return Size;
		}
		bool operator==(const arrbrd_t<Size>& rhs) const noexcept {
			for (int x = 0; x < Size; x++) {
				for (int y = 0; y < Size; ++y) {
					if (mat[x * Size + y] != rhs.mat[x * Size + y]) {
						return false;
					}
				}
			}
			return true;
		}
		void clear() {
			for (int i = 0; i < Size; ++i) {
				for (int j = 0; j < Size; ++j) {
					mat[i * Size + j] = 0;
				}
			}
		}
		template <class Archive>
		void serialize(Archive& ar) {
			for (int x = 0; x < Size; ++x) {
				for (int y = 0; y < Size; ++y) {
					ar(mat[x * Size + y]);
				}
			}
		}
	private:
		PosType mat[Size * Size] = { 0 };
		//color settings:
		//0:none
		//1:color0---------0b01>>1
		//2:color1---------0b10>>1
	};

	using arrbrd = arrbrd_t<8>;

	template<int Size>
	using static_brd = typename std::conditional<(Size > 8), arrbrd_t<Size>, bitbrd_t<Size>>::type;

	template<typename Brd>
	concept mat_brd = requires(Brd b, const coord & crd, color c) {
		{ b.get_col(crd) } -> std::same_as<color>;
		b.set_col(crd, c);
		{ b.in_board(crd) } -> std::same_as<bool>;
		{ b.brd_size() } -> std::same_as<int>;
	};
};
template<int Size, typename CharT>
struct std::formatter<bw::ataxx::bitbrd_t<Size>, CharT> {
	bool linemark = false;
	bool right = true;
	constexpr auto parse(auto& context) {
		using namespace bw::othello;
		auto it = context.begin(), end = context.end();
		if (it == end || *it == '}') {
			return it;
		}
		while (*it != '}' && it != end) {
			switch (*it) {
			case 's':
				CharMap.current() = symb;
				break;
			case 'n':
				CharMap.current() = numb;
				break;
			case 'm':
				linemark = true;
				break;
			case '>':
				right = true;
				break;
			case '<':
				right = false;
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
	auto format(const bw::othello::bitbrd_t<Size>& brd, FormatContext& fc) const {
		using namespace bw::othello;
		auto it = fc.out();
		for (int x = 0; x < Size; ++x) {
			for (int y = 0; y < Size; ++y) {
				auto str = CharMap[brd.get_col({ x,y })];
				for (auto& c : str) {
					it = c;
				}
			}
			if (linemark) {
				std::format_to(it, "{}", 1 + x);
			}
			it = '\n';
		}
		if (linemark) {
			for (int i = 0; i < Size; ++i) {
				if (right) {
					it = ' ';
					it = 'A' + i;
				}
				else {
					it = 'A' + i;
					it = ' ';
				}
			}
		}
		return it;
	};
};

template<int Size, typename CharT>
struct std::formatter<bw::ataxx::arrbrd_t<Size>, CharT> {
	bool linemark = false;
	bool right = true;
	constexpr auto parse(auto& context) {
		using namespace bw::othello;
		auto it = context.begin(), end = context.end();
		if (it == end || *it == '}') {
			return it;
		}
		while (*it != '}' && it != end) {
			switch (*it) {
			case 's':
				CharMap.current() = symb;
				break;
			case 'n':
				CharMap.current() = numb;
				break;
			case 'm':
				linemark = true;
				break;
			case '>':
				right = true;
				break;
			case '<':
				right = false;
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
	auto format(const bw::othello::arrbrd_t<Size>& brd, FormatContext& fc) const {
		using namespace bw::othello;
		auto it = fc.out();
		for (int x = 0; x < Size; ++x) {
			for (int y = 0; y < Size; ++y) {
				auto str = CharMap[brd.get_col({ x,y })];
				for (auto& c : str) {
					it = c;
				}
			}
			if (linemark) {
				std::format_to(it, "{}", 1 + x);
			}
			it = '\n';
		}
		if (linemark) {
			for (int i = 0; i < Size; ++i) {
				if (right) {
					it = ' ';
					it = 'A' + i;
				}
				else {
					it = 'A' + i;
					it = ' ';
				}
			}
		}
		return it;
	};
};

template<typename CharT>
struct std::formatter<bw::ataxx::dynamic_brd, CharT> {
	bool linemark = false;
	bool right = true;
	constexpr auto parse(auto& context) {
		using namespace bw::othello;
		auto it = context.begin(), end = context.end();
		if (it == end || *it == '}') {
			return it;
		}
		while (*it != '}' && it != end) {
			switch (*it) {
			case 's':
				CharMap.current() = symb;
				break;
			case 'n':
				CharMap.current() = numb;
				break;
			case 'm':
				linemark = true;
				break;
			case '>':
				right = true;
				break;
			case '<':
				right = false;
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
	auto format(const bw::othello::dynamic_brd& brd, FormatContext& fc) const {
		using namespace bw::othello;
		auto it = fc.out();
		for (int x = 0; x < brd.size; ++x) {
			for (int y = 0; y < brd.size; ++y) {
				auto str = CharMap[brd.get_col({ x,y })];
				for (auto& c : str) {
					it = c;
				}
			}
			if (linemark) {
				std::format_to(it, "{}", 1 + x);
			}
			it = '\n';
		}
		if (linemark) {
			for (int i = 0; i < brd.size; ++i) {
				if (right) {
					it = ' ';
					it = 'A' + i;
				}
				else {
					it = 'A' + i;
					it = ' ';
				}
			}
		}
		return it;
	};
};
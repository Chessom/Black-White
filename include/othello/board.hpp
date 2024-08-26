#pragma once
#include"stdafx.h"
#include"core.hpp"
#include"utility.hpp"
#include"othello/move.hpp"
#include"globals.hpp"
#define printbit(brd,size) std::println("{:sm<}",bitbrd_t<size>(brd,0));
namespace bw::othello {
	template<typename T>
	struct TypeWidth {
		constexpr static int width = sizeof(T) * 8;
	};
	template<int Size>
	struct SizetoType {
		constexpr static int Power = Size * Size;
		constexpr static bool Even = ~Size & 1;
		constexpr static bool isVaild = Power <= sizeof(uintmax_t) * 8;
		using type =
			typename std::conditional<(!isVaild), void,
			typename std::conditional<(Power <= 16), uint_fast16_t,
			typename std::conditional<(Power <= 32), uint_fast32_t,
			typename std::conditional<(Power <= 64), uint_fast64_t,
			void>::type>::type>::type>::type;
	};

}

namespace bw::othello {
	enum { default_size = 8, max_size = 16 };
	using std::vector;
	class basic_brd {
	public:
		basic_brd() = default;
		bool in_board(const coord& crd) const {
			return (crd.x < size && crd.y < size && crd.x >= 0 && crd.y >= 0);
		}
		virtual color getcol(const coord& crd) const = 0;
		virtual void setcol(const coord& crd, color col) = 0;
		virtual void initialize() {
			setcol({ size / 2 - 1,size / 2 - 1 }, col1);
			setcol({ size / 2 ,size / 2 }, col1);
			setcol({ size / 2 - 1,size / 2 }, col0);
			setcol({ size / 2 ,size / 2 - 1 }, col0);
		}
		virtual void applymove(const coord& crd, const color& col) = 0;
		virtual int count(color col) = 0;
		virtual void resize(int newsize) = 0;
		virtual ~basic_brd() = default;
		int size = default_size;
	};

	struct dynamic_brd :public basic_brd {
		dynamic_brd() {
			size = default_size;
			mat.resize(size);
			for (auto& v : mat) {
				v.resize(size);
			}
		}
		dynamic_brd(int Size) {
			size = Size;
			mat.resize(size);
			for (auto& v : mat) {
				v.resize(size);
			}
		}
		virtual color getcol(const coord& crd)const override {
			return mat[crd.x][crd.y] - 1;
		}
		virtual void setcol(const coord& crd, color col)override {
			mat[crd.x][crd.y] = col + 1;
		}
		virtual void applymove(const coord& crd, const color& col) override {
			using drc_t = int;
			using namespace directions;
			color opcol = col ^ 1;
			for (drc_t drc = R; drc <= UR; ++drc) {
				coord iter = crd;
				bool flag = false;
				int times = 0;
				if (in_board(iter.to_next(drc)) && getcol(iter) == opcol) {
					color c = 0;
					while (in_board(iter.to_next(drc))) {
						++times;
						c = getcol(iter);
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
						mat[iter.x][iter.y] ^= 0b11;
					}
				}
			}
			setcol(crd, col);
		}
		virtual int countpiece(color col) {
			int cnt = 0, t = col + 1;
			for (int x = 0; x < size; ++x) {
				for (int y = 0; y < size; ++y) {
					if (mat[x][y] == t) ++cnt;
				}
			}
			return cnt;
		}
		virtual int count(color col) override {
			int cnt = 0, t = col + 1;
			for (int x = 0; x < size; ++x) {
				for (int y = 0; y < size; ++y) {
					if (mat[x][y] == t) ++cnt;
				}
			}
			return cnt;
		}
		virtual void resize(int newsize) {
			mat.resize(newsize);
			for (auto& v : mat) {
				v.resize(newsize);
			}
			size = newsize;
		}
		int brd_size() const {
			return mat.size();
		}
		void clear() {
			for (int i = 0; i < mat.size(); ++i) {
				for (int j = 0; j < mat.size(); ++j) {
					mat[i][j] = none;
				}
			}
		}
		template <class Archive>
		void serialize(Archive& ar) {
			ar(mat);
		}
		vector<vector<char>> mat;
	};
	/*class safe_dynamic_brd :public dynamic_brd {
	public:
		safe_dynamic_brd() = default;
		safe_dynamic_brd(int size) :dynamic_brd(size) {}
		safe_dynamic_brd(dynamic_brd Board) :dynamic_brd(std::move(Board)) {}
		virtual color getcol(const coord& crd) override {
			mtx.lock_shared();
			return dynamic_brd::getcol(crd);
			mtx.unlock_shared();
		}
		virtual void setcol(const coord& crd, color col) override {
			mtx.lock();
			dynamic_brd::setcol(crd, col);
			mtx.unlock();
		}
	private:
		std::shared_mutex mtx;
	};*/


	template<int Size, std::integral PosType = char>
	class arrbrd_t;
	template<int Size>
	class bitbrd_t;

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
					if (abrd.mat[x][y]) {
						if constexpr (bw::is_pow2(Size)) {
							brd[abrd.mat[x][y] >> 1] |= 1ull << ((x << bw::log2(Size)) + y);
						}
						else {
							brd[abrd.mat[x][y] >> 1] |= 1ull << (x * Size + y);
						}
					}
				}
			}
		}
		bitbrd_t(const dynamic_brd& dbrd) {
			brd[0] = brd[1] = ull(0);
			for (int x = 0; x < Size; ++x) {
				for (int y = 0; y < Size; ++y) {
					if (dbrd.mat[x][y]) {
						if constexpr (bw::is_pow2(Size)) {
							brd[dbrd.mat[x][y] >> 1] |= 1ull << ((x << bw::log2(Size)) + y);
						}
						else {
							brd[dbrd.mat[x][y] >> 1] |= 1ull << (x * Size + y);
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
		bool in_board(const coord& crd) const noexcept {
			return (crd.x < Size && crd.y < Size && crd.x >= 0 && crd.y >= 0);
		}
		color getcol(const coord& crd) const noexcept {
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
		void setcol(const coord& crd, color col) noexcept {
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
			setcol({ Size / 2 - 1,Size / 2 - 1 }, col1);
			setcol({ Size / 2 ,Size / 2 }, col1);
			setcol({ Size / 2 - 1,Size / 2 }, col0);
			setcol({ Size / 2 ,Size / 2 - 1 }, col0);
		}
		void applymove(const coord& crd, const color& col) noexcept {
			applymove(crd2bit(crd), col);
		}
		void applymove(const uint64_t& bit_crd, const color& col) noexcept {
			ull& brd_blue = brd[col];
			ull& brd_green = brd[col ^ 1];
			ull brd_green_inner;
			ull flip = 0ull;
			ull brd_flip;
			ull brd_green_adj;
			ull it = bit_crd;

			brd_green_inner = brd_green & bw::inner(Size);

			brd_flip = (it >> 1) & brd_green_inner;
			brd_flip |= (brd_flip >> 1) & brd_green_inner;

			if constexpr (Size >= 6) {
				brd_green_adj = brd_green_inner & (brd_green_inner >> 1);
				brd_flip |= (brd_flip >> 2) & brd_green_adj;
			}
			if constexpr (Size == 8) brd_flip |= (brd_flip >> 2) & brd_green_adj;
			if ((brd_flip >> 1) & brd_blue)flip |= brd_flip;

			brd_flip = (it << 1) & brd_green_inner;
			brd_flip |= (brd_flip << 1) & brd_green_inner;

			if constexpr (Size >= 6) {
				brd_green_adj = brd_green_inner & (brd_green_inner << 1);
				brd_flip |= (brd_flip << 2) & brd_green_adj;
			}
			if constexpr (Size == 8) brd_flip |= (brd_flip << 2) & brd_green_adj;
			if ((brd_flip << 1) & brd_blue)flip |= brd_flip;

			brd_flip = (it >> Size) & brd_green;
			brd_flip |= (brd_flip >> Size) & brd_green;

			if constexpr (Size >= 6) {
				brd_green_adj = brd_green & (brd_green >> Size);
				brd_flip |= (brd_flip >> (Size << 1)) & brd_green_adj;
			}
			if constexpr (Size == 8) brd_flip |= (brd_flip >> (Size << 1)) & brd_green_adj;
			if ((brd_flip >> Size) & brd_blue)flip |= brd_flip;

			brd_flip = (it << Size) & brd_green;
			brd_flip |= (brd_flip << Size) & brd_green;

			if constexpr (Size >= 6) {
				brd_green_adj = brd_green & (brd_green << Size);
				brd_flip |= (brd_flip << (Size << 1)) & brd_green_adj;
			}
			if constexpr (Size == 8)brd_flip |= (brd_flip << (Size << 1)) & brd_green_adj;
			if ((brd_flip << Size) & brd_blue)flip |= brd_flip;

			brd_flip = (it >> (Size - 1)) & brd_green_inner;
			brd_flip |= (brd_flip >> (Size - 1)) & brd_green_inner;

			if constexpr (Size >= 6) {
				brd_green_adj = brd_green_inner & (brd_green_inner >> (Size - 1));
				brd_flip |= (brd_flip >> ((Size - 1) << 1)) & brd_green_adj;
			}
			if constexpr (Size == 8)brd_flip |= (brd_flip >> ((Size - 1) << 1)) & brd_green_adj;
			if ((brd_flip >> (Size - 1)) & brd_blue)flip |= brd_flip;

			brd_flip = (it << (Size - 1)) & brd_green_inner;
			brd_flip |= (brd_flip << (Size - 1)) & brd_green_inner;

			if constexpr (Size >= 6) {
				brd_green_adj = brd_green_inner & (brd_green_inner << (Size - 1));
				brd_flip |= (brd_flip << ((Size - 1) << 1)) & brd_green_adj;
			}
			if constexpr (Size == 8)brd_flip |= (brd_flip << ((Size - 1) << 1)) & brd_green_adj;
			if ((brd_flip << (Size - 1)) & brd_blue)flip |= brd_flip;

			brd_flip = (it >> (Size + 1)) & brd_green_inner;
			brd_flip |= (brd_flip >> (Size + 1)) & brd_green_inner;

			if constexpr (Size >= 6) {
				brd_green_adj = brd_green_inner & (brd_green_inner >> (Size + 1));
				brd_flip |= (brd_flip >> ((Size + 1) << 1)) & brd_green_adj;
			}
			if constexpr (Size == 8)brd_flip |= (brd_flip >> ((Size + 1) << 1)) & brd_green_adj;
			if ((brd_flip >> (Size + 1)) & brd_blue)flip |= brd_flip;

			brd_flip = (it << (Size + 1)) & brd_green_inner;
			brd_flip |= (brd_flip << (Size + 1)) & brd_green_inner;

			if constexpr (Size >= 6) {
				brd_green_adj = brd_green_inner & (brd_green_inner << (Size + 1));
				brd_flip |= (brd_flip << ((Size + 1) << 1)) & brd_green_adj;
			}
			if constexpr (Size == 8)brd_flip |= (brd_flip << ((Size + 1) << 1)) & brd_green_adj;
			if ((brd_flip << (Size + 1)) & brd_blue)flip |= brd_flip;

			brd[col] ^= flip;
			brd[col ^ 1] ^= flip;
			brd[col] |= it;
		}
		ull getmoves(color col) const noexcept {
			const ull& brd_blue = brd[col];
			const ull& brd_green = brd[col ^ 1];
			ull moves;
			ull brd_green_inner;
			ull brd_flip;
			ull brd_green_adj;

			brd_green_inner = brd_green & bw::inner(Size);


			brd_flip = (brd_blue >> 1) & brd_green_inner;
			brd_flip |= (brd_flip >> 1) & brd_green_inner;

			if constexpr (Size >= 6) {
				brd_green_adj = brd_green_inner & (brd_green_inner >> 1);
				brd_flip |= (brd_flip >> 2) & brd_green_adj;
			}
			if constexpr (Size == 8) brd_flip |= (brd_flip >> 2) & brd_green_adj;

			moves = brd_flip >> 1;

			brd_flip = (brd_blue << 1) & brd_green_inner;
			brd_flip |= (brd_flip << 1) & brd_green_inner;

			if constexpr (Size >= 6) {
				brd_green_adj = brd_green_inner & (brd_green_inner << 1);
				brd_flip |= (brd_flip << 2) & brd_green_adj;
			}
			if constexpr (Size == 8) brd_flip |= (brd_flip << 2) & brd_green_adj;

			moves |= brd_flip << 1;

			brd_flip = (brd_blue >> Size) & brd_green;
			brd_flip |= (brd_flip >> Size) & brd_green;


			if constexpr (Size >= 6) {
				brd_green_adj = brd_green & (brd_green >> Size);
				brd_flip |= (brd_flip >> (Size << 1)) & brd_green_adj;
			}
			if constexpr (Size == 8) brd_flip |= (brd_flip >> (Size << 1)) & brd_green_adj;


			moves |= brd_flip >> Size;

			brd_flip = (brd_blue << Size) & brd_green;
			brd_flip |= (brd_flip << Size) & brd_green;

			if constexpr (Size >= 6) {
				brd_green_adj = brd_green & (brd_green << Size);
				brd_flip |= (brd_flip << (Size << 1)) & brd_green_adj;
			}
			if constexpr (Size == 8) brd_flip |= (brd_flip << (Size << 1)) & brd_green_adj;


			moves |= brd_flip << Size;

			brd_flip = (brd_blue >> (Size - 1)) & brd_green_inner;
			brd_flip |= (brd_flip >> (Size - 1)) & brd_green_inner;

			if constexpr (Size >= 6) {
				brd_green_adj = brd_green_inner & (brd_green_inner >> (Size - 1));
				brd_flip |= (brd_flip >> ((Size - 1) << 1)) & brd_green_adj;
			}
			if constexpr (Size == 8) brd_flip |= (brd_flip >> ((Size - 1) << 1)) & brd_green_adj;

			moves |= brd_flip >> (Size - 1);

			brd_flip = (brd_blue << (Size - 1)) & brd_green_inner;
			brd_flip |= (brd_flip << (Size - 1)) & brd_green_inner;


			if constexpr (Size >= 6) {
				brd_green_adj = brd_green_inner & (brd_green_inner << (Size - 1));
				brd_flip |= (brd_flip << ((Size - 1) << 1)) & brd_green_adj;
			}
			if constexpr (Size == 8) brd_flip |= (brd_flip << ((Size - 1) << 1)) & brd_green_adj;

			moves |= brd_flip << (Size - 1);

			brd_flip = (brd_blue >> (Size + 1)) & brd_green_inner;
			brd_flip |= (brd_flip >> (Size + 1)) & brd_green_inner;

			if constexpr (Size >= 6) {
				brd_green_adj = brd_green_inner & (brd_green_inner >> (Size + 1));
				brd_flip |= (brd_flip >> ((Size + 1) << 1)) & brd_green_adj;
			}
			if constexpr (Size == 8) brd_flip |= (brd_flip >> ((Size + 1) << 1)) & brd_green_adj;

			moves |= brd_flip >> (Size + 1);

			brd_flip = (brd_blue << (Size + 1)) & brd_green_inner;
			brd_flip |= (brd_flip << (Size + 1)) & brd_green_inner;

			if constexpr (Size >= 6) {
				brd_green_adj = brd_green_inner & (brd_green_inner << (Size + 1));
				brd_flip |= (brd_flip << ((Size + 1) << 1)) & brd_green_adj;
			}
			if constexpr (Size == 8) brd_flip |= (brd_flip << ((Size + 1) << 1)) & brd_green_adj;

			moves |= brd_flip << (Size + 1);

			moves &= ~(brd_blue | brd_green);
			return moves;
		}
		dynamic_brd to_dynamic() const {
			dynamic_brd brd(Size);
			for (int x = 0; x < Size; ++x) {
				for (int y = 0; y < Size; ++y) {
					brd.setcol({ x,y }, getcol({ x,y }));
				}
			}
			return brd;
		}
		int countpiece(color black_or_white) const noexcept {
			return std::popcount(brd[black_or_white]);
		}
		int count(color col) const noexcept{
			if (col == none) {
				return std::popcount(~(brd[0] | brd[1]));
			}
			else {
				return std::popcount(col);
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
					mat[x][y] = 0;
					if constexpr (bw::is_pow2(Size)) {
						it = 1ull << ((x << bw::log2(Size)) + y);
					}
					else {
						it = 1ull << (x * Size + y);
					}
					if (ful & it) {
						mat[x][y] = bool(bit1 & it) + 1;
					}
				}
			}
		};
		arrbrd_t(const dynamic_brd& dbrd) {
			for (int x = 0; x < Size; ++x) {
				for (int y = 0; y < Size; ++y) {
					mat[x][y] = dbrd.mat[x][y];
				}
			}
		}
		bool in_board(const coord& crd) const noexcept {
			return crd.x < Size && crd.y < Size && crd.x >= 0 && crd.y >= 0;
		}
		color getcol(const coord& crd) const noexcept {
			return mat[crd.x][crd.y] - 1;
		}
		void setcol(const coord& crd, const color& col) noexcept {
			mat[crd.x][crd.y] = col + 1;
		}
		void applymove(const coord& crd, const color& col) noexcept {
			using drc_t = int;
			using namespace directions;
			color opcol = col ^ 1;
			for (drc_t drc = R; drc <= UR; ++drc) {
				coord iter = crd;
				bool flag = false;//是否为有效的方向
				int times = 0;
				if (in_board(iter.to_next(drc)) && getcol(iter) == opcol) {//存在相邻反色子
					color c = 0;
					while (in_board(iter.to_next(drc))) {
						++times;
						c = getcol(iter);
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
						mat[iter.x][iter.y] ^= 0b11;//翻转棋子
					}
				}
			}
			setcol(crd, col);
		}
		void initialize() noexcept {
			//⚪︎⚫︎
			//⚫︎⚪︎
			setcol({ Size / 2 - 1,Size / 2 - 1 }, col1);
			setcol({ Size / 2 ,Size / 2 }, col1);
			setcol({ Size / 2 - 1,Size / 2 }, col0);
			setcol({ Size / 2 ,Size / 2 - 1 }, col0);
		}
		dynamic_brd to_dynamic() const {
			dynamic_brd brd(Size);
			for (int x = 0; x < Size; ++x) {
				for (int y = 0; y < Size; ++y) {
					brd.setcol({ x,y }, getcol({ x,y }));
				}
			}
			return brd;
		}
		int countpiece(color col) const noexcept {
			int cnt = 0, t = col + 1;
			for (int x = 0; x < Size; ++x) {
				for (int y = 0; y < Size; ++y) {
					if (mat[x][y] == t) ++cnt;
				}
			}
			return cnt;
		}
		int count(color col) const noexcept {
			int cnt = 0, t = col + 1;
			for (int x = 0; x < Size; ++x) {
				for (int y = 0; y < Size; ++y) {
					if (mat[x][y] == t) ++cnt;
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
					if (mat[x][y] != rhs.mat[x][y]) {
						return false;
					}
				}
			}
			return true;
		}
		void clear() {
			for (int i = 0; i < Size; ++i) {
				for (int j = 0; j < Size; ++j) {
					mat[i][j] = 0;
				}
			}
		}
		template <class Archive>
		void serialize(Archive& ar) {
			for (int x = 0; x < Size; ++x) {
				for (int y = 0; y < Size; ++y) {
					ar(mat[x][y]);
				}
			}
		}
	private:
		PosType mat[Size][Size] = { 0 };
		//color settings:
		//0:none
		//1:color0---------0b01>>1
		//2:color1---------0b10>>1
	};

	using arrbrd = arrbrd_t<8>;

	template<int Size>
	using static_brd = typename std::conditional<(Size > 8), arrbrd_t<Size>, bitbrd_t<Size>>::type;

	template<typename Brd>
	concept mat_brd = requires(Brd b, const coord & crd) {
		{ b.getcol(crd) } -> std::same_as<color>;
		{ b.in_board(crd) }->std::same_as<bool>;
		{ b.brd_size() }->std::same_as<int>;
	};
};
template<int Size, typename CharT>
struct std::formatter<bw::othello::bitbrd_t<Size>, CharT> {
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
				auto str = CharMap[brd.getcol({ x,y })];
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
struct std::formatter<bw::othello::arrbrd_t<Size>, CharT> {
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
				auto str = CharMap[brd.getcol({ x,y })];
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
struct std::formatter<bw::othello::dynamic_brd, CharT> {
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
				auto str = CharMap[brd.getcol({ x,y })];
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
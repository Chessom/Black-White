#pragma once
#include"stdafx.h"
namespace bw::core {
	using ll = long long;
	using ull = unsigned long long;
	using bititer = ull;
	using color = int;
	using mtx_ptr = std::shared_ptr<std::mutex>;
	class metadata {
	public:
		const std::string
			version = "3.0",
			author = "Vemy",
			email = "chessom@foxmail.com",
			license = "MIT";
	};
	namespace gameid {
		enum { unspecific, othello, ataxx, gobang, tictactoe };
	}
	enum { none = -1, col0 = 0, col1 = 1, col2, col3 };//none,color0,color1
	inline constexpr color op_col(const color& c) { return c ^ 1; }
	namespace directions {
		enum { R = 0, RD, D, DL, L, LU, U, UR };	//directions
		//					U
		//					^
		//					|
		//			  L<----o---->R
		//					|
		//					v
		//					D
	}


	template<typename T>
	concept Coordinate = requires(T && coordinate) {
		coordinate.X;
		coordinate.Y;
		requires std::is_integral<decltype(T::X)>::value;
		requires std::is_integral<decltype(T::Y)>::value;
	};
	template<std::integral Int>
	class _coord {
	public:
		using pos_t = Int;
		constexpr _coord() :x(Int(0)), y(Int(0)) {}
		constexpr _coord(const Int& X, const Int& Y) : x(X), y(Y) {}
		template<Coordinate T>
		constexpr _coord(const T& crd) : x(crd.X), y(crd.Y) {}
		constexpr _coord<Int>& to_next(const int& direction) {
			using namespace directions;
			switch (direction) {
			case R:
				++x;
				break;
			case RD:
				++x;
				++y;
				break;
			case D:
				++y;
				break;
			case DL:
				--x;
				++y;
				break;
			case L:
				--x;
				break;
			case LU:
				--x;
				--y;
				break;
			case U:
				--y;
				break;
			case UR:
				++x;
				--y;
				break;
			default:
				std::unreachable();
			}
			return *this;
		}
		constexpr _coord<Int>& to_next_n(const int& direction, int n) {
			using namespace directions;
			switch (direction) {
			case R:
				x += n;
				break;
			case RD:
				x += n;
				y += n;
				break;
			case D:
				y += n;
				break;
			case DL:
				x -= n;
				y += n;
				break;
			case L:
				x -= n;
				break;
			case LU:
				x -= n;
				y -= n;
				break;
			case U:
				y -= n;
				break;
			case UR:
				x += n;
				y -= n;
				break;
			default:
				std::unreachable();
			}
			return *this;
		}
		constexpr _coord<Int> next(const int& direction) {
			auto crd = *this;
			return crd.to_next(direction);
		}
		constexpr _coord<Int> next(const int& direction, int n) {
			auto crd = *this;
			return crd.to_next_n(direction, n);
		}
		constexpr void clear() {
			x = y = 0;
		}
		constexpr _coord& operator=(const _coord&) = default;
		constexpr bool operator==(const _coord& other) const {
			return x == other.x && y == other.y;
		}
		constexpr _coord& operator+=(const _coord& rhs) {
			x += rhs.x;
			y += rhs.y;
			return *this;
		}
		constexpr _coord& operator-=(const _coord& rhs) {
			x -= rhs.x;
			y -= rhs.y;
			return *this;
		}
		template <class Archive>
		void serialize(Archive& ar) {
			ar(x, y);
		}
		Int x, y;
	};

	template<std::integral Int>
	constexpr _coord<Int> operator+(const _coord<Int>& lhs, const _coord<Int>& rhs) {
		_coord<Int> t(lhs);
		t += rhs;
		return t;
	}
	template<std::integral Int>
	constexpr _coord<Int> operator-(const _coord<Int>& lhs, const _coord<Int>& rhs) {
		_coord<Int> t(lhs);
		t -= rhs;
		return t;
	}
	template<std::integral Int>
	std::ostream& operator<<(std::ostream& os, const _coord<Int>& crd) {
		os << "(" << crd.x << "," << crd.y << ")";
		return os;
	}

	using coord = _coord<int>;
	template <std::integral Int>
	using vec = _coord<Int>;
	REFLECTION(coord, x, y);

	template<typename T>
	struct timer_deque_t {
		timer_deque_t(std::shared_ptr<boost::asio::io_context> ctx) :tim(*ctx) {}
		std::deque<T> q;
		boost::asio::steady_timer tim;
	};
	using str_dq = timer_deque_t<std::string>;
	using str_dq_ptr = std::shared_ptr<str_dq>;
};
namespace std {
	template<std::integral Int, typename CharT>
	struct formatter<bw::core::_coord<Int>, CharT> :std::formatter<Int, CharT> {
		template<typename FormatContext>
		auto format(const bw::core::_coord<Int>& crd, FormatContext& fc) const {
			return std::format_to(fc.out(), "({},{})", crd.x, crd.y);
		};
	};
}
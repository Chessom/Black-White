#pragma once
#include"stdafx.h"
#include"core.hpp"
#include"gamer.hpp"
#include"othello.move.hpp"
#include"othello.moves.hpp"
#include"othello.board.hpp"
namespace bw {
	template<typename T>
	class mtxdeque_t {
	public:
		std::deque<T> q;
		std::mutex mtx;
		std::condition_variable conv;
	};
	using mtxdeque = mtxdeque_t<bw::core::coord>;
	template<typename T>
	using mtxdq_ptr = std::shared_ptr<mtxdeque_t<T>>;

	using timdq = core::timer_deque_t<bw::othello::move>;
	using timdq_ptr = std::shared_ptr<timdq>;
}
namespace bw::othello {
	class gamer :public bw::basic_gamer {
	public:
		using context = boost::asio::io_context;
		using context_ptr = std::shared_ptr<context>;
		enum { pass, end, suspended };
		gamer() { gametype = core::gameid::othello;; };
		gamer(core::color Color, int ID = 0, const std::string& Name = "", int GamerType = invalid)
			:basic_gamer(Color, ID, Name, GamerType, core::gameid::othello) {
		};
		virtual boost::cobalt::task<move> getmove(dynamic_brd& brd, std::chrono::seconds limit = std::chrono::seconds(0)) = 0;
		virtual std::string get_name() = 0;
		virtual boost::cobalt::task<void> pass_msg(std::string_view) = 0;
		virtual boost::cobalt::task<void> pass_move(move mv) = 0;
		virtual void cancel() = 0;//cancel getmove
		virtual ~gamer() = default;
		moves mvs;
	};
	using gamer_ptr = std::shared_ptr<gamer>;
}
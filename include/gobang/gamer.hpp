﻿#pragma once
#include"stdafx.h"
#include"core.hpp"
#include"basic_gamer.hpp"
#include"gobang/move.hpp"
#include"gobang/moves.hpp"
#include"gobang/board.hpp"
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

	using timdq = core::timer_deque_t<bw::gobang::move>;
	using timdq_ptr = std::shared_ptr<timdq>;
}
namespace bw::gobang {
	struct gamer :public bw::basic_gamer {
		using context = boost::asio::io_context;
		using context_ptr = std::shared_ptr<context>;
		enum { pass, end, suspended };
		gamer() { gametype = core::gameid::gobang; };
		gamer(core::color Color, int ID = 0, const std::string& Name = "", int GamerType = invalid)
			:basic_gamer(Color, ID, Name, GamerType, core::gameid::gobang) {
		};
		gamer(const basic_gamer& gamer_info) :basic_gamer(gamer_info) {};
		virtual boost::cobalt::task<move> get_move(board& brd, std::chrono::seconds limit = std::chrono::seconds(0)) = 0;
		virtual std::string get_name() = 0;
		virtual boost::cobalt::task<void> pass_msg(std::string) = 0;
		virtual boost::cobalt::task<void> pass_move(move mv) = 0;
		virtual void cancel() = 0;//cancel get_move
		virtual void reset() = 0;//for another round
		virtual ~gamer() {
			spdlog::trace("Othello gamer Destructor");
		}
		detailed_type detailed_gamer_type = detailed_type::invalid;
		moves mvs;
	};
	using gamer_ptr = std::shared_ptr<gamer>;
}
﻿#pragma once
#include"stdafx.h"
#include"core.hpp"
#include"othello.gamer.hpp"
#include"components.h"
namespace bw::othello {
	using namespace std::chrono_literals;
	class human_gamer :public gamer {
	public:
		using timer = boost::asio::steady_timer;
		using timer_ptr = std::shared_ptr<timer>;
		human_gamer() :gamer() { name = global_config->default_name; gamertype = human; }
		human_gamer(color Color, int ID = 0, const std::string& Name = global_config->default_name)
			:gamer(Color, ID, Name, human) {};
		boost::cobalt::task<move> getmove(dynamic_brd& brd, std::chrono::seconds limit = 0s) {
			mvs.update(brd, col);
			move mv;
			if (limit == 0s) {
				pmvdq->tim.expires_at(std::chrono::steady_clock::time_point::max());
			}
			else {
				pmvdq->tim.expires_after(limit);
			}
			while (true) {
				boost::system::error_code ec;
				co_await pmvdq->tim.async_wait(boost::asio::redirect_error(boost::cobalt::use_op, ec));

				while (!pmvdq->q.empty()) {
					mv = pmvdq->q.front();
					pmvdq->q.pop_front();
					if (mv.mvtype == move::mv) {
						if (mvs.find(mv.pos) != moves::npos) {
							co_return mv;
						}
					}
					else {
						co_return mv;
					}
				}
			}
			co_return mv;
		}
		string get_name() { return name; }
		boost::cobalt::task<void> pass_msg(std::string_view message) override {
			ui::msgbox(message);
			co_return;
		}
		boost::cobalt::task<void> pass_move(move mv) {
			if (mv.mvtype == move::str) {
				co_await pass_msg(mv.msg);
			}
			co_return;
		};
		virtual void cancel() {
			pmvdq->q.push_back({ .mvtype = move::quit });
			pmvdq->tim.cancel();
		}
		void set_move_queue(timdq_ptr move_queue) {
			pmvdq = std::move(move_queue);
		}
		timdq_ptr pmvdq;
	};
}
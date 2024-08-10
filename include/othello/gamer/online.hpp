#pragma once
#include"stdafx.h"
#include"othello/gamer.hpp"
#include"tui/components.hpp"
#include"online/basic_user.hpp"
#include"online/basic_online_gamer.hpp"
namespace bw::othello {
	using namespace std::chrono_literals;
	class online_gamer :public gamer, public online::basic_online_gamer {
	public:
		using timer = boost::asio::steady_timer;
		using timer_ptr = std::shared_ptr<timer>;
		online_gamer() :gamer(), basic_online_gamer() { name = "Anonymous"; gamertype = online; }
		online_gamer(basic_gamer_info info) :gamer(info), basic_online_gamer() {}
		online_gamer(color Color, int ID = 0, const std::string& Name = "Anonymous")
			:gamer(Color, ID, Name, online) {};
		boost::cobalt::task<move> getmove(dynamic_brd& brd, std::chrono::seconds limit = 0s) {
			mvs.update(brd, col);
			move mv;
			if (limit == 0s) {
				rd_dq->tim.expires_at(std::chrono::steady_clock::time_point::max());
			}
			else {
				rd_dq->tim.expires_after(limit);
			}
			while (true) {
				boost::system::error_code ec;
				co_await rd_dq->tim.async_wait(boost::asio::redirect_error(boost::cobalt::use_op, ec));

				while (!rd_dq->q.empty()) {
					struct_json::from_json(mv, rd_dq->q.front());
					rd_dq->q.pop_front();
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
		boost::cobalt::task<void> pass_msg(std::string msg) override {
			assert(u_ptr != nullptr);
			u_ptr->deliver(wrap(
				str_msg{
					.content = msg,
					.target_type = str_msg::g,
					.id1 = u_ptr->id,
					.id2 = id,
					.name = u_ptr->name,
				},
				msg_t::str
			));
			co_return;
		}
		boost::cobalt::task<void> pass_move(move mv) override {
			assert(u_ptr != nullptr);
			if (mv.mvtype == move::str) {
				co_await pass_msg(mv.msg);
			}
			else {
				mvstr = "";
				struct_json::to_json(mv, mvstr);
				u_ptr->deliver(wrap(
					game_msg{
						.type = game_msg::move,
						.id = u_ptr->id,
						.movestr = mvstr
					},msg_t::game
				));
			}
			co_return;
		};
		virtual void cancel() {
			std::string cancel_mv;
			struct_json::to_json(move{ .mvtype = move::quit }, cancel_mv);
			rd_dq->q.push_back(cancel_mv);
			rd_dq->tim.cancel();
		}
		std::string mvstr;
	};
}
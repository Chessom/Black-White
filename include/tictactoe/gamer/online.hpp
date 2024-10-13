#pragma once
#include"globals.hpp"
#include"online/basic_online_gamer.hpp"
#include"tictactoe/gamer.hpp"
namespace bw::tictactoe {
	using namespace std::chrono_literals;
	class online_gamer :public gamer, public online::basic_online_gamer {
	public:
		using timer = boost::asio::steady_timer;
		using timer_ptr = std::shared_ptr<timer>;
		online_gamer() :gamer() { name = gettext("Anonymous"); gamertype = online; }
		online_gamer(basic_gamer_info info) :gamer(info) {}
		online_gamer(core::color Color, int ID = 0, const std::string& Name = "Anonymous")
			:gamer(Color, ID, Name, online) {};
		boost::cobalt::task<move> get_move(board& brd, std::chrono::seconds limit = 0s) {
			if (limit == 0s) {
				rd_dq->tim.expires_at(std::chrono::steady_clock::time_point::max());
			}
			else {
				rd_dq->tim.expires_after(limit);
			}
			while (true) {
				boost::system::error_code ec;
				if (rd_dq->q.empty())
					co_await rd_dq->tim.async_wait(boost::asio::redirect_error(boost::cobalt::use_op, ec));
				move mv;
				struct_json::from_json(mv, rd_dq->q.front());
				rd_dq->q.pop_front();
				if (mv.mvtype == move::mv) {
					if (brd.check_move(mv.crd, mv.col)) {
						co_return mv;
					}
				}
				else {
					co_return mv;
				}
			}
			co_return move{};
		}
		string get_name() const { return name; }
		boost::cobalt::task<void> pass_msg(std::string msg) override {
			assert(!u_ptr.expired());
			auto us = u_ptr.lock();
			us->deliver(wrap(
				str_msg{
					.content = msg,
					.target_type = str_msg::g,
					.id1 = us->id,
					.id2 = id,
					.name = us->name,
				},
				msg_t::str
				));
			co_return;
		}
		//To do
		boost::cobalt::task<void> pass_move(move mv) override {
			assert(u_ptr.expired());
			auto us = u_ptr.lock();
			if (us->state == online::user_st::gaming) {
				mvstr = "";
				struct_json::to_json(mv, mvstr);
				us->deliver(wrap(
					game_msg{
						.type = game_msg::move,
						.id = us->id,
						.movestr = mvstr
					}, msg_t::game
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
	protected:
		std::string mvstr;
	};
}
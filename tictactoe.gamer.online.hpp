#pragma once
#include"globals.hpp"
#include"tictactoe.gamer.hpp"
namespace bw::tictactoe {
	using namespace std::chrono_literals;
	struct online_gamer : public gamer {
		using timer = boost::asio::steady_timer;
		using timer_ptr = std::shared_ptr<timer>;
		online_gamer(boost::cobalt::channel<move>& channel) :chan(channel), gamer() { name = global_config->default_name; gamertype = remote; }
		online_gamer(boost::cobalt::channel<move>& channel, core::color Color, int ID = 0, const std::string& Name = global_config->default_name)
			:chan(channel), gamer(Color, ID, Name, remote) {};
		boost::cobalt::task<move> getmove(board& brd, std::chrono::seconds limit = 0s) override {
			move mv;
			while (true) {
				boost::system::error_code ec;
				mv = co_await chan.read();
				if (mv.mvtype == move::mv) {
					if (brd.checkmove(mv.crd, mv.col)) {
						co_return mv;
					}
				}
				else {
					co_return mv;
				}
			}
			co_return mv;
		}
		string get_name() { return name; }
		boost::cobalt::task<void> pass_msg(std::string_view message) override {
			co_return;
		}
		boost::cobalt::task<void> pass_move(move mv) override {
			co_return;
		};
		virtual void cancel() {

		}
		boost::cobalt::channel<move>& chan;
	};
}
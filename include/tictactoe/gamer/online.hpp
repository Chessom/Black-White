#pragma once
#include"globals.hpp"
#include"tictactoe/gamer.hpp"
namespace bw::tictactoe {
	using namespace std::chrono_literals;
	struct online_gamer : public gamer {
		using timer = boost::asio::steady_timer;
		using timer_ptr = std::shared_ptr<timer>;
		using str_channel = boost::cobalt::channel<std::string>;
		using channel_ptr = std::shared_ptr<str_channel>;
		online_gamer(channel_ptr channel) :chan(channel), gamer() { name = global_config->default_name; gamertype = remote; }
		online_gamer(channel_ptr channel, core::color Color, int ID = 0, const std::string& Name = global_config->default_name)
			:chan(channel), gamer(Color, ID, Name, remote) {};
		boost::cobalt::task<move> getmove(board& brd, std::chrono::seconds limit = 0s) override {
			move mv;
			std::string mv_str;
			while (true) {
				boost::system::error_code ec;

				mv_str = co_await chan->read();
				struct_json::from_json(mv, mv_str);
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
		boost::cobalt::task<void> pass_msg(std::string message) override {
			co_return;
		}
		boost::cobalt::task<void> pass_move(move mv) override {
			co_return;
		};
		virtual void cancel() {

		}
		channel_ptr chan;
	};
}
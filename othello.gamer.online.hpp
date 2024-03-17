#include"stdafx.h"
#include"othello.gamer.hpp"
#include"components.h"
namespace bw::othello {
	using namespace std::chrono_literals;
	struct online_gamer : public gamer {
		using timer = boost::asio::steady_timer;
		using timer_ptr = std::shared_ptr<timer>;
		online_gamer(boost::cobalt::channel<move>& channel) :chan(channel), gamer() { name = global_config->default_name; gamertype = remote; }
		online_gamer(boost::cobalt::channel<move>& channel, color Color, int ID = 0, const std::string& Name = global_config->default_name)
			:chan(channel), gamer(Color, ID, Name, remote) {};
		boost::cobalt::task<move> getmove(dynamic_brd& brd, std::chrono::seconds limit = 0s) {
			mvs.update(brd, col);
			move mv;
			while (true) {
				boost::system::error_code ec;
				mv = co_await chan.read();
				if (mv.mvtype == move::mv) {
					if (mvs.find(mv.pos) != moves::npos) {
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
		boost::cobalt::task<void> pass_move(move mv) {
			if (mv.mvtype == move::str) {
				co_await pass_msg(mv.msg);
			}
		};
		virtual void cancel() {
			
		}
		boost::cobalt::channel<move>& chan;
	};
}
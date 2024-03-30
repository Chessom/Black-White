#include"stdafx.h"
#include"othello.gamer.hpp"
#include"components.h"
#include"online.gamer.hpp"
namespace bw::othello {
	using namespace std::chrono_literals;
	struct online_gamer : public gamer {
		using timer = boost::asio::steady_timer;
		using timer_ptr = std::shared_ptr<timer>;
		using move_channel = boost::cobalt::channel<std::string>;
		using move_channel_ptr = std::shared_ptr<move_channel>;
		online_gamer(online::user& User) :gamer(), user(User) {
			name = global_config->default_name;
			gamertype = remote;
			read_ch = std::make_shared<move_channel>();
		}
		online_gamer(online::user& User, color Color, int ID = 0, const std::string& Name = global_config->default_name)
			:user(User), gamer(Color, ID, Name, remote) {
			read_ch = std::make_shared<move_channel>();
		};
		boost::cobalt::task<move> getmove(dynamic_brd& brd, std::chrono::seconds limit = 0s) {
			mvs.update(brd, col);
			move mv;
			while (true) {
				boost::system::error_code ec;
				move_str_buf = co_await read_ch->read();
				struct_json::from_json(mv, move_str_buf);
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
			struct_json::to_json(mv, move_str_buf);
			user.deliver(wrap(
				game_msg{
					.type = game_msg::move,
					.movestr = move_str_buf,
				},
				msg_t::game
			));
		};
		virtual void cancel() {
			
		}
		online::user& user;
		move_channel_ptr read_ch;
	private:
		std::string move_str_buf;
	};
}
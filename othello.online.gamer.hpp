#pragma once
#include"online.basic_gamer.hpp"
#include"othello.gamer.hpp"
#include<boost/cobalt.hpp>
namespace bw::othello::online {
	struct online_gamer: bw::online::basic_online_gamer {
	public:

		boost::cobalt::generator<bw::othello::move> getmove(bw::othello::dynamic_brd& brd){
			//mvs.update(brd, col);
			bw::othello::move mv{ .mvtype = bw::othello::move::invalid };
			game_msg gmmsg;
			while (true) {
				
			}
		}

		std::string get_name(){
			return gmr.name;
		}

		void pass_msg(std::string_view sv){
			pass_move(bw::othello::move{ .mvtype = bw::othello::move::str,.msg = sv.data() });
		}

		void pass_move(bw::othello::move mv){
			using namespace bw::othello;
			game_msg gmmsg;
			struct_json::to_json(mv, gmmsg.movestr);
			gmr.deliver(wrap(gmmsg, msg_t::game));
		}

	};
}
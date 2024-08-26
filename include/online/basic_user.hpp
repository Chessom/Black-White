#pragma once
#include"net/message.hpp"
#include"user_info.hpp"
#include"game.hpp"
namespace bw{
	class basic_Game;
	using basic_Game_ptr = std::shared_ptr<basic_Game>;
}
namespace bw::online {
	
	struct basic_user :user_info {
		virtual void deliver(const message&) = 0;
		basic_game_ptr gm_ptr;
		basic_Game_ptr Game_ptr;
		virtual ~basic_user() = default;
	};
	using basic_user_ptr = std::shared_ptr<basic_user>;
	using basic_weak_user_p = std::weak_ptr<basic_user>;
}
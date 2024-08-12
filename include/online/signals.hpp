#pragma once
#include<boost/signals2.hpp>
#include"net/message.hpp"
namespace bw::online::signals {
	inline boost::signals2::signal<void()>
		refresh_room_info,
		start_game;
	inline boost::signals2::signal<void(int)> end_game;
}
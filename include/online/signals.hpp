#pragma once
#include<boost/signals2.hpp>
namespace bw::online::signals {
	inline boost::signals2::signal<void()>
		refresh_room_info,
		start_game,
		end_game;
}
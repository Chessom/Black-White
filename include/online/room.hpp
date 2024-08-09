#pragma once
#include"stdafx.h"
#include"net/message.hpp"
#include"online/gamer_info.hpp"
namespace bw::online {
	class room_info {
	public:
		room_info() = default;
		room_info(int room_id, int room_owner, std::string room_name, int room_state) 
			:id(room_id), owner(room_owner), name(room_name), state(room_state) {};
		int id = -1, owner = 0;
		std::string name = "Default";
		int state = none;
		int gamersize = 0;
		enum { gaming, end, prepared, none };
		virtual ~room_info() = default;
		enum { outdated, latest };
		int infostate = outdated;
	};
	REFLECTION(room_info, id, owner, name, state, gamersize);
	inline std::string_view state_str[] = { "game","end","preparaed","none" };
	using room_info_ptr = std::shared_ptr<room_info>;
	class room :public room_info {
	public:

	};
	class hall_info {
	public:
		std::string address = "localhost";
		int port = 22222;
		std::vector<room_info> rooms;
		std::set<gamer_info> gamers;
		enum { outdated, latest };
		int infostate = outdated;
	};
	using hall_info_ptr = std::shared_ptr<hall_info>;
}

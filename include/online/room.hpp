#pragma once
#include"stdafx.h"
#include"net/message.hpp"
#include"online/user_info.hpp"
#include"boost/static_string.hpp"
namespace bw::online {
	struct room_info {
	public:
		int id, owner;
		char name[15];
		int state;
		int usersize;
		enum { gaming, end, prepared, none };
		enum { outdated, latest };
		int infostate;
	};
	inline room_info default_room_info() {
		return room_info{
			.id = 0,
			.owner = 0,
			.name = "Default",
			.state = room_info::none,
			.usersize = 0,
			.infostate = room_info::outdated,
		};
		std::is_trivially_copyable<boost::static_string<15>>::value;
	}
	REFLECTION(room_info, id, owner, name, state, usersize);
	//struct room_info {
	//public:
	//	int id = -1, owner = 0;
	//	std::string name = "Default";
	//	int state = none;
	//	int gamersize = 0;
	//	enum { gaming, end, prepared, none };
	//	//virtual ~room_info() = default;
	//	enum { outdated, latest };
	//	int infostate = outdated;
	//};
	
	inline std::string_view state_str[] = { "game","end","preparaed","none" };
	using room_info_ptr = std::shared_ptr<room_info>;
	class hall_info {
	public:
		std::string address = "localhost";
		int port = 22222;
		std::vector<room_info> rooms;
		std::set<user_info> gamers;
		enum { outdated, latest };
		int infostate = outdated;
	};
	using hall_info_ptr = std::shared_ptr<hall_info>;
}

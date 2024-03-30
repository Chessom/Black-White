#pragma once
#include"stdafx.h"
namespace bw {
	enum { success, failed };
	struct control_msg {
		enum { create, update, del, leave, join, none };
		int type = none;
		std::string content;
		int id1, id2;
		int target_type;
		enum { r, g, n };
	};
	REFLECTION(control_msg, type, content, id1, id2, target_type);
	struct str_msg {
		std::string content;
		int target_type = g;
		int id1, id2;
		std::string name;
		enum { r, g, n };
	};
	REFLECTION(str_msg, content, target_type, id1, id2, name);
	struct game_msg {
		enum { prepare, start, move, end };
		int type;
		std::string movestr, board;
	};
	REFLECTION(game_msg, type, movestr, board);
	struct get_msg {
		std::string get_type;
		std::vector<int> ids;
	};
	REFLECTION(get_msg, get_type, ids);
	struct ret_msg {
		int value = failed;
		std::string ret_type;
		std::string ret_str;
	};
	REFLECTION(ret_msg, value, ret_type, ret_str);

	class message {
	public:
		enum { invalid, str, game, control, get, ret };
		message() = default;
		std::string jsonstr;
		int type = invalid;
	};
	REFLECTION(message, jsonstr, type);
	using msg_t = message;
	template<typename T>
	msg_t wrap(T&& obj, int message_type = msg_t::invalid) {
		msg_t temp;
		iguana::to_json(obj, temp.jsonstr);
		temp.type = message_type;
		return temp;
	}
}
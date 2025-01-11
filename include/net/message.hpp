#pragma once
#include<ylt/struct_json/json_writer.h>
#include<ylt/struct_json/json_reader.h>
namespace bw {
	enum { success, failed };
	enum { server_id = 0 };
	struct control_msg {
		enum { create, update, del, leave, join, none };
		int type = none;
		std::string content;
		int id1 = 0, id2 = 0;
		int target_type = n;
		enum { r, g, n };
	};
	YLT_REFL(control_msg, type, content, id1, id2, target_type);
	struct str_msg {
		std::string content;
		int target_type = g;//目标类型，g:单独用户，r:整个房间
		int id1 = 0, id2 = 0;//id1为发送者ID, id2为接收者ID/房间ID
		std::string name;//发送者名字
		enum { r, g, n };
	};
	YLT_REFL(str_msg, content, target_type, id1, id2, name);
	struct game_msg {
		enum { create, prepare, start, move, watch, brd, match, end };
		int type = move;
		int id = 0;
		std::string movestr, board;
		enum { ok, server_end_game, gamer_quit_or_disconnect, unknown };
		//create：movestr代表game的名字（种类），而board则是game的详细信息，是一个json字符串
		//prepare：movestr代表basic_gamer_info（basic_gamer）
		//start: movestr代表game种类，board代表board size
		//move: movestr代表move
		//watch: id:自己的ID ////////////可能的实现：movestr:是密码等验证方式？
		//brd: board：棋盘字符串
		//match: 代表在整个服务器随机匹配，其余与prepare相同
		//end: id代表结束的状态码。
	};
	YLT_REFL(game_msg, type, id, movestr, board);
	struct get_msg {
		std::string get_type;
		std::vector<int> ids;
	};
	YLT_REFL(get_msg, get_type, ids);
	struct notice_msg {
		std::string str;
	};
	YLT_REFL(notice_msg, str);
	struct ret_msg {
		int value = failed;
		std::string ret_type;
		std::string ret_str;
	};
	YLT_REFL(ret_msg, value, ret_type, ret_str);
	

	class message {
	public:
		enum { invalid, str, game, control, get, notice, ret };
		message() = default;
		std::string jsonstr;
		int type = invalid;
	};
	YLT_REFL(message, jsonstr, type);
	using msg_t = message;
	template<typename T>
	msg_t wrap(T&& obj, int message_type = msg_t::invalid) {
		msg_t temp;
		iguana::to_json(obj, temp.jsonstr);
		temp.type = message_type;
		return temp;
	}
	inline bool operator==(const message& m1, const message& m2) {
		return m1.jsonstr == m2.jsonstr && m1.type == m2.type;
	}

}
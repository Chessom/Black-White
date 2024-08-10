#pragma once
#include<ylt/struct_json/json_reader.h>
#include<ylt/struct_json/json_writer.h>
namespace bw::online {
	namespace gamer_st {
		enum { prepared, gaming, free };
	}
	struct user_info {
	public:
		std::string name;
		int state = gamer_st::free;
		int id = -1;
		int authority = anonymous;
		enum { admin, ordinary, anonymous, limited };
		~user_info() = default;
	};
	using gamer_info_ptr = std::shared_ptr<user_info>;
	REFLECTION(user_info, name, id, state, authority);
}
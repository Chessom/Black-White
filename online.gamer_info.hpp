#pragma once
#include"stdafx.h"
namespace bw::online {
	namespace gamer_st {
		enum { prepared, gaming, free };
	}
	class gamer_info {
	public:
		std::string name;
		int state = gamer_st::free;
		int id = -1;
		int authority = anonymous;
		enum { admin, ordinary, anonymous, limited };
	};
	using gamer_info_ptr = std::shared_ptr<gamer_info>;
	REFLECTION(gamer_info, name, id, state, authority);
}
#pragma once
#include"core.hpp"
#include"basic_user.hpp"
namespace bw::online {
	struct basic_online_gamer{
		using context_ptr = std::shared_ptr<boost::asio::io_context>;
		basic_online_gamer() = default;
		basic_online_gamer(core::str_dq_ptr ptr) { set_read_queue(ptr); }
		core::str_dq_ptr rd_dq = nullptr;
		basic_weak_user_p u_ptr;
		void set_read_queue(core::str_dq_ptr read_queue) {
			rd_dq = std::move(read_queue);
		}
		void set_user(basic_user_ptr uptr) {
			u_ptr = uptr;
		}
	};
	using basic_online_gamer_ptr = std::shared_ptr<basic_online_gamer>;
}
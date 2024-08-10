#pragma once
#include"net/message.hpp"
#include"user_info.hpp"
namespace bw::online {
	struct basic_user :user_info {
		virtual void deliver(const message&) = 0;
		virtual ~basic_user() = default;
	};
	using basic_user_ptr = std::shared_ptr<basic_user>;
}
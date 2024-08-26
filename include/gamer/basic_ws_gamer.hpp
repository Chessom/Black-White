#pragma once
#include"boost/beast.hpp"
namespace bw {
	struct basic_http_gamer {
		using context = boost::beast::net::io_context;
		using context_ptr = std::shared_ptr<context>;
		basic_http_gamer(context_ptr Context) :pctx(Context) {}

		context_ptr pctx;
	};
}
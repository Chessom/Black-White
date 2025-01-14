#pragma once
#include<boost/beast.hpp>
#include<boost/cobalt.hpp>
#include<nlohmann/json.hpp>
namespace bw {
	//This gamer get move from a http server
	inline const char* AGENT_STR = R"(Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/100.0.4896.127 Safari/537.36)";
	struct basic_http_gamer {
		using context = boost::beast::net::io_context;
		using context_ptr = std::shared_ptr<context>;
		using server_port = std::pair<std::string, int>;
		basic_http_gamer(context_ptr Context, const std::string& server, int port) 
			:pctx(Context), 
			server_address(server), 
			port(port),
			stream(*Context),
			resolver(*Context)
		{
			using namespace boost::beast;
		}
		void set_server(const std::string& server, int port) {
			server_address = server;
			this->port = port;
		}
		server_port get_server() const {
			return { server_address,port };
		}

		virtual void sync_test_server() {
			namespace beast = boost::beast;
			namespace http = beast::http;
			namespace net = boost::asio;
			using namespace nlohmann;
			using tcp = net::ip::tcp;
			try {
				sync_connect();

				http::request<http::string_body> req{ http::verb::get, "/ping", 11 };
				req.set(http::field::host, server_address);
				req.set(http::field::user_agent, AGENT_STR);
				req.set(http::field::cookie, cookie);

				http::write(stream, req);

				http::response<http::string_body> res_;

				http::read(stream, buffer_, res_);

				stream.close();

				auto& body = res_.body();
				if (auto ck = res_.find(http::field::set_cookie); ck != res_.end()) {
					cookie = ck->value();
				}
				if (body == "pong, bw server!") {
					return;
				}
				throw std::runtime_error("Invalid response");
			}
			catch (...) {
				stream.close();
				throw;
			}
		}

		virtual boost::cobalt::task<void> async_test_server() {
			namespace beast = boost::beast;
			namespace http = beast::http;
			namespace net = boost::asio;
			using namespace nlohmann;
			using tcp = net::ip::tcp;
			net::io_context& ioc = pctx == nullptr ? *(pctx = std::make_shared<context>()) : *pctx;

			try {
				co_await async_connect();

				http::request<http::string_body> req{ http::verb::get, "/ping", 11 };
				req.set(http::field::host, server_address);
				req.set(http::field::user_agent, AGENT_STR);
				req.set(http::field::cookie, cookie);

				stream.expires_after(std::chrono::seconds(30));
				co_await http::async_write(stream, req, boost::cobalt::use_op);

				http::response<http::string_body> res_;

				co_await http::async_read(stream, buffer_, res_, boost::cobalt::use_op);

				stream.close();

				auto body = res_.body();
				if (auto ck = res_.find(http::field::set_cookie); ck != res_.end()) {
					cookie = ck->value();
				}
				json r = json::parse(body);
				if (r["code"].get<int>() == 200 &&
					r["msg"].get<std::string>() == "pong, bw server!") {
					co_return;
				}
				throw std::runtime_error("Invalid response");
			}
			catch (...) {
				stream.close();
				throw;
			}
		}

		boost::cobalt::task<void> async_connect() {
			auto const results = co_await resolver.async_resolve(server_address, std::to_string(port), boost::cobalt::use_op);
			co_await stream.async_connect(results, boost::cobalt::use_op);
		}

		void sync_connect() {
			auto const results = resolver.resolve(server_address, std::to_string(port));
			auto ep = stream.connect(results);
			assert(stream.socket().is_open());
		}

		bool connected() const {
			return stream.socket().is_open();
		}

		void close() {
			if (stream.socket().is_open()) {
				stream.socket().close();
			}
		}

		virtual ~basic_http_gamer() {
			if (stream.socket().is_open()) {
				stream.socket().close();
			}
		};
	protected:
		std::string server_address;
		int port;
		context_ptr pctx;
		boost::beast::flat_buffer buffer_;
		boost::beast::net::ip::tcp::resolver resolver;
		boost::beast::tcp_stream stream;
		std::string cookie = "";
	};
}
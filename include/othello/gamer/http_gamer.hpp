#pragma once
#include"gamer/basic_http_gamer.hpp"
#include"othello/gamer.hpp"
namespace bw::othello {
	struct http_gamer :basic_http_gamer, gamer {
		http_gamer(gamer::context_ptr ctx, core::color Color, const std::string& server = "localhost", int port = 8888)
			:basic_http_gamer(ctx, server, port),
			gamer(Color)
		{
			detailed_gamer_type = detailed_type::http_gamer;
		}
		http_gamer(gamer::context_ptr ctx, const basic_gamer& gamer_info, const std::string& server = "localhost", int port = 8888)
			:basic_http_gamer(ctx, server, port),
			gamer(gamer_info)
		{
			detailed_gamer_type = detailed_type::http_gamer;
		};

		virtual boost::cobalt::task<move> get_move(
			const dynamic_brd& brd, 
			std::chrono::seconds limit = std::chrono::seconds(0)
		) override {
			namespace http = boost::beast::http;
			using namespace nlohmann;
			for (int i = 0; i < 3; ++i) {
				if (stream.socket().is_open()) {
					stream.expires_at(std::chrono::steady_clock::time_point::max());
					mvs.update(brd, col);
					json data;
					data["board_size"] = brd.brd_size();
					data["board_vec"] = dynamic_brd_to_vector(brd);
					data["color"] = col == col0 ? "col0" : "col1";
					data["coefficient"] = json{};

					http::request<http::string_body> req{ http::verb::post, "/get_move/othello", 11 };
					req.set(http::field::host, server_address);
					req.set(http::field::user_agent, AGENT_STR);
					req.set(http::field::cookie, cookie);
					req.body() = data.dump();
					req.prepare_payload();

					co_await http::async_write(stream, req, boost::cobalt::use_op);

					http::response<http::string_body> res_;

					co_await http::async_read(stream, buffer_, res_, boost::cobalt::use_op);

					auto body = res_.body();
					if (auto ck = res_.find(http::field::set_cookie); ck != res_.end()) {
						cookie = ck->value();
					}
					json res_data = json::parse(body);
					if (res_data["code"].get<int>() == 200) {
						move mv;
						struct_json::from_json(mv, res_data["data"].dump());
						if (mvs.find(mv.pos) != moves::npos) {
							co_return mv;
						}
						else {
							co_return{ .mvtype = move::invalid,.pos = mv.pos,.msg = gettext("Invalid move returned by the server!") };
						}
					}
				}
				else {
					co_await async_connect();
				}
			}
		}

		virtual std::string get_name() override {
			return name;
		}

		void refresh_name() {
			namespace http = boost::beast::http;
			using namespace nlohmann;
			if (!connected()) {
				sync_connect();
			}
			http::request<http::empty_body> req{ http::verb::get, "/server_name", 11 };

			stream.expires_after(std::chrono::seconds(30));
			http::write(stream, req);

			http::response<http::string_body> res_;

			http::read(stream, buffer_, res_);

			auto body = res_.body();
			if (auto ck = res_.find(http::field::set_cookie); ck != res_.end()) {
				cookie = ck->value();
			}
			json r = json::parse(body);
			if (r["code"].get<int>() == 200) {
				name = r["data"].get<std::string>();
				name = name.empty() ? "Anonymous" : name;
				return;
			}
			throw std::runtime_error("Invalid server name");
		}

		virtual boost::cobalt::task<void> pass_msg(std::string) override {
			co_return;
		}

		virtual boost::cobalt::task<void> pass_move(move mv) override {
			co_return;
		}

		virtual void cancel() override {
			stream.cancel();
		}

		virtual void reset() override {
			return;
		}

		virtual bool good() const override {
			return is_good;
		}

		std::string& method() { return method_; }

		virtual ~http_gamer() = default;

		bool is_good = false;
	private:
		std::vector<int> dynamic_brd_to_vector(const dynamic_brd& brd) {
			auto& v = brd.get_underlying_vector();
			std::vector<int> ret;
			for (const auto& col : v) {
				ret.push_back(static_cast<int>(col));
			}
			return ret;
		}

		boost::cobalt::task<void> server_name_request() {
			namespace http = boost::beast::http;
			using namespace nlohmann;
			if (connected()) {

				http::request<http::empty_body> req{ http::verb::get, "/server_name", 11 };

				stream.expires_after(std::chrono::seconds(30));
				co_await http::async_write(stream, req, boost::cobalt::use_op);

				http::response<http::string_body> res_;

				co_await http::async_read(stream, buffer_, res_, boost::cobalt::use_op);

				auto& body = res_.body();
				if (auto ck = res_.find(http::field::set_cookie); ck != res_.end()) {
					cookie = ck->value();
				}
				json r = json::parse(body);
				if (r["code"].get<int>() == 200) {
					name = r["data"].get<std::string>();
				}
			}
		}
		std::string method_ = "";
	};
}
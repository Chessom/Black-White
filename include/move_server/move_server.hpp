#pragma once
#include <chrono>
#include "ylt/coro_http/coro_http_server.hpp"
#include <boost/uuid.hpp>
#include "othello/ai.hpp"
#include <nlohmann/json.hpp>

namespace bw::move_server {
	namespace http = coro_http;
	using namespace nlohmann;
	struct person_t {
		void foo(coro_http::coro_http_request&, coro_http::coro_http_response& res) {
			using namespace coro_http;
			res.set_status_and_content(status_type::ok, "ok");
		}
	};

	struct session_data {
		othello::ai::solver solver;
	};

	struct move_server {
		using context = boost::asio::io_context;
		using context_ptr = std::shared_ptr<context>;
		move_server(int thread_num = 1, unsigned short port = 8888) :
			pctx(nullptr),
			thd_num(thread_num),
			port(port),
			server(thread_num, port)
		{
			reply_template["code"] = http::status_type::ok;
			reply_template["msg"] = "";
			reply_template["data"] = json{};
		}
		/*move_server(context_ptr ctx, unsigned short port = 8888) :
			pctx(ctx),
			thd_num(1),
			port(port),
			server(*ctx, port)
		{
			reply_template["code"] = http::status_type::ok;
			reply_template["msg"] = "";
			reply_template["data"] = json{};
		}*/

		std::error_code sync_start() {
			using namespace coro_http;
			using namespace std::chrono_literals;
			server.set_http_handler<GET>(
				"/ping",
				&move_server::ping_handler, *this
			);

			server.set_http_handler<GET>(
				"/server_name",
				&move_server::server_name_handler, *this
			);

			server.set_http_handler<POST>(
				"/get_move/othello",
				&move_server::get_move_othello_handler, *this
			);

			server.set_http_handler<POST>(
				"/echo",
				[this](coro_http_request& req, coro_http_response& resp) {
					auto bnd = req.get_boundary();
					auto body = req.get_body();
					resp.set_status_and_content(http::status_type::ok, std::string(req.get_body()));
				}
			);

			/*server.set_http_handler<POST>(
				"/api/stop",
				[this](coro_http_request& req, coro_http_response& resp) {
					resp.set_status_and_content(http::status_type::ok, "OK");
					server.stop();
				});
			server.set_http_handler<POST, PUT>(
				"/post", [](coro_http_request& req, coro_http_response& resp) {
					auto req_body = req.get_body();
					resp.set_status_and_content(status_type::ok, std::string{ req_body });
				});

			server.set_http_handler<GET>(
				"/headers", [](coro_http_request& req, coro_http_response& resp) {
					auto name = req.get_header_value("name");
					auto age = req.get_header_value("age");
					assert(name == "tom");
					assert(age == "20");
					resp.set_status_and_content(status_type::ok, "ok");
				});

			server.set_http_handler<GET>(
				"/query", [](coro_http_request& req, coro_http_response& resp) {
					auto name = req.get_query_value("name");
					auto age = req.get_query_value("age");
					assert(name == "tom");
					assert(age == "20");
					resp.set_status_and_content(status_type::ok, "ok");
				});

			server.set_http_handler<coro_http::GET, coro_http::POST>(
				"/users/:userid/subscriptions/:subid",
				[](coro_http_request& req, coro_http_response& response) {
					assert(req.params_["userid"] == "ultramarines");
					assert(req.params_["subid"] == "guilliman");
					response.set_status_and_content(status_type::ok, "ok");
				});

			person_t person{};
			server.set_http_handler<GET>("/person", &person_t::foo, person);*/
			return server.sync_start();
		}

		void stop() {
			server.stop();
		}

		context_ptr get_executor() const { return pctx; }

		std::string name() const {
			return "BlackWhiteServer1.0";
		}

		void ping_handler(http::coro_http_request& req, http::coro_http_response& resp) {
			resp.set_status_and_content(http::status_type::ok, "pong, bw server!");
		}

		void server_name_handler(http::coro_http_request& req, http::coro_http_response& resp) {
			json res;
			res["code"] = http::status_type::ok;
			res["data"] = name();
			resp.set_status_and_content(http::status_type::ok, res.dump());
		}

		async_simple::coro::Lazy<void> get_move_othello_handler(http::coro_http_request& req, http::coro_http_response& resp) {
			co_await coro_io::post([&] {
				try
				{
					using solver_ptr = std::shared_ptr<othello::ai::solver>;
					solver_ptr solver = nullptr;
					int brd_size = 8;
					othello::dynamic_brd brd;
					core::color col = core::none;
					othello::ai::ai_option option;

					json req_data = json::parse(req.get_body());

					brd_size = req_data["board_size"].get<int>();
					brd.from_vector(req_data["board_vec"].get<std::vector<int>>());
					col = req_data["color"].get<std::string>() == "col0" ? core::col0 : core::col1;

					if (req_data["coefficient"].empty()) {
						option = othello::ai::ai_option{};
					}
					else {
						struct_json::from_json(option, req_data["coefficient"].dump());
					}

					auto session = req.get_session(true);
					auto optional = session->get_data<solver_ptr>("solver");
					auto optional_size = session->get_data<int>("board_size");

					if (optional.has_value()) {
						solver = optional.value();
					}
					else {
						solver = std::make_shared<othello::ai::solver>(col, brd_size);
						session->set_data("solver", solver);
					}

					if (optional_size.has_value()) {
						brd_size = optional_size.value();
					}
					else {
						session->set_data("board_size", brd_size);
						optional_size = brd_size;
					}

					if (brd_size != optional_size.value() || !(option == solver->option)) {
						solver->reset();
						solver->option = option;
						solver->set_algo();
						req.get_session()->set_data("board_size", brd_size);
					}

					othello::moves mvs(brd, col);
					othello::move mv;

					json resp_data = reply_template;

					if (!mvs.empty()) {
						if (mvs.size == 1) {
							mv.mvtype = othello::move::mv;
							mv.pos = mvs.coords[0];
							mv.c = col;
						}
						else {
							auto pos = solver->best_move(brd, col);
							mv = othello::move{ .mvtype = othello::move::mv, .pos = pos,.c = col };
						}
					}
					else {
						othello::moves mvs2(brd, core::op_col(col));
						if (mvs2.empty()) {
							mv = othello::move{ .mvtype = othello::move::quit };
							resp_data["msg"] = "Game End";
						}
						else {
							mv = othello::move{ .mvtype = othello::move::pass };
							resp_data["msg"] = "Gamer Pass";
						}
					}
					std::string mvjson;
					struct_json::to_json(mv, mvjson);
					resp_data["data"] = json::parse(mvjson);
					resp.set_status_and_content(http::status_type::ok, resp_data.dump());
				}
				catch (const nlohmann::json::exception& e) {
					auto js = reply_template;
					js["code"] = http::status_type::bad_request;
					js["msg"] = e.what();
					resp.set_status_and_content(http::status_type::bad_request, js.dump());
				}
				catch (const std::runtime_error& e) {
					auto js = reply_template;
					js["code"] = http::status_type::bad_request;
					js["msg"] = e.what();
					resp.set_status_and_content(http::status_type::bad_request, js.dump());
				}
				catch (const std::exception& e) {
					auto js = reply_template;
					js["code"] = http::status_type::internal_server_error;
					js["msg"] = e.what();
					resp.set_status_and_content(http::status_type::internal_server_error, js.dump());
				}
			});
		}

	private:
		context_ptr pctx;
		int thd_num, port;
		coro_http::coro_http_server server;
		json reply_template;
		//std::unordered_map<std::string, session_data> data_map;
	};
}

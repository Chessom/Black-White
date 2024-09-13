#pragma once
#include"stdafx.h"
#include"core.hpp"
#include"othello/gamer.hpp"
#include"tui/components.hpp"
namespace bw::othello {

	/*struct online_game_settings {
		int board_size;
		basic_gamer first, second;
	};
	REFLECTION(online_game_settings, board_size, first, second);*/
	
	using namespace std::chrono_literals;
	class remote_tcp_gamer :public gamer{
	public:
		remote_tcp_gamer(
			context_ptr ContextPtr,
			core::color Color,
			int ID = 0,
			const std::string& Name = gettext("Remote tcp gamer"))
			:gamer(Color, ID, Name, remote),
			context_ptr(ContextPtr) 
		{
			rdq = std::make_shared<timdq>(context_ptr);
			wtq = std::make_shared<timdq>(context_ptr);
			wtq->tim.expires_at(std::chrono::steady_clock::time_point::max());
			socket_ptr = std::make_shared<boost::asio::ip::tcp::socket>(*context_ptr);
			detailed_gamer_type = detailed_type::remote_tcp_gamer;
		};
		using acceptor = boost::asio::ip::tcp::acceptor;
		using context = boost::asio::io_context;
		using socket = boost::asio::ip::tcp::socket;
		using endpoint = boost::asio::ip::tcp::endpoint;
		using mvdq = std::deque<move>;
		using move_handler = std::function<void(const move& mv)>;
		using move_handler_ptr = std::shared_ptr<move_handler>;
		inline string gbk2utf8(std::string_view s) {
			return boost::locale::conv::to_utf<char>(s.data(), "gbk");
		}
		boost::cobalt::task<move> getmove(dynamic_brd& brd, std::chrono::seconds limit = 0s) override {
			mvs.update(brd, col);
			move mv{ .mvtype = move::invalid };
			
			if (limit == 0s) {
				rdq->tim.expires_at(std::chrono::steady_clock::time_point::max());
			}
			else {
				rdq->tim.expires_after(limit);
			}
			while (true) {
				boost::system::error_code ec;
				co_await rdq->tim.async_wait(boost::asio::redirect_error(boost::cobalt::use_op, ec));
				
				while (!rdq->q.empty()) {
					mv = rdq->q.front();
					rdq->q.pop_front();
					switch (mv.mvtype) {
					case move::mv:
						if (mvs.find(mv.pos) != moves::npos) {
							co_return mv;
						}
						break;
					case move::start:
						break;
					default:
						co_return mv;
					}
				}
			}
			co_return mv;
		}
		move consume_one_move() {
			move mv;
			return mv;
		}
		void start() {
			boost::cobalt::spawn(*context_ptr, co_read(), boost::asio::detached);
			boost::cobalt::spawn(*context_ptr, co_write(), boost::asio::detached);
		}
		void insert_move(const move& mv, timdq_ptr dq) {
			dq->q.push_back(mv);
			dq->tim.cancel_one();
		}
		void deliver(const move& mv) {
			insert_move(mv, wtq);
		}
		boost::cobalt::task<void> co_read() {
			using namespace boost::asio;
			move mv;
			std::string jsonstr, sizestr;
			boost::system::error_code ec;
			while (socket_ptr->is_open())
			{
				try
				{
					jsonstr = "";
					sizestr = "";
					co_await async_read(*socket_ptr, dynamic_buffer(sizestr, 5), boost::cobalt::use_op);
					co_await async_read(*socket_ptr, dynamic_buffer(jsonstr, stoi(sizestr)), boost::cobalt::use_op);
					struct_json::from_json(mv, jsonstr);
					if (handle_move_ != nullptr) {
						(*handle_move_)(mv);
					}
					else {
						insert_move(mv, rdq);
					}
				}
				catch (const std::exception& ec)
				{
					printf("\a");
					ui::msgbox(gbk2utf8(ec.what()));
					insert_move(move{ .mvtype = move::str,.msg = format("FILE:{}\n LINE:{}\n msg:{}",__FILE__,__LINE__,gbk2utf8(ec.what())) }, rdq);
					co_return;
				}
			}
		}
		boost::cobalt::task<void> co_write() {
			using namespace boost::asio;
			boost::system::error_code ec;
			std::string jsonstr;
			socket_ptr->wait(socket::wait_write);
			while (true) {
				if (wtq->q.empty()) {
					boost::system::error_code ec;
					co_await wtq->tim.async_wait(redirect_error(boost::cobalt::use_op, ec));
				}
				while (socket_ptr->is_open() && wtq->q.size()) {
					try
					{
						jsonstr = "";
						move mv = wtq->q.front();
						struct_json::to_json(mv, jsonstr);
						jsonstr = std::format("{:<5}{}", jsonstr.size(), jsonstr);
						co_await async_write(*socket_ptr, dynamic_buffer(jsonstr, jsonstr.size()), boost::cobalt::use_op);
						wtq->q.pop_front();
					}
					catch (const std::exception& ec)
					{
						ui::msgbox(std::format("Send Error:{}", gbk2utf8(ec.what())));
						insert_move({ .mvtype = move::str,.msg = format("FILE:{}\n LINE:{}\n msg:{}",__FILE__,__LINE__,gbk2utf8(ec.what())) }, rdq);
						co_return;
					}
				}
			}
		}
		std::tuple<endpoint, boost::system::error_code> 
		make_endpoint(std::string address, int port) {
			using namespace boost::asio::ip;
			tcp::resolver res(*context_ptr);
			boost::system::error_code ec;
			const auto& eps = res.resolve(tcp::v4(), address, std::to_string(port), ec);
			if (ec) {
				ui::msgbox(gbk2utf8(ec.what()));
				return { {},ec };
			}
			const auto& ep = (*eps.begin()).endpoint();
			auto ipstr = ep.address().to_string();
			auto port_n = ep.port();
			//ui::msgbox(std::format("resolve result:\nIP:{},port:{}", ipstr, port_n));
			return std::make_tuple(ep, ec);
		}
		bool connect(std::string address, int port) {
			using namespace boost::asio::ip;
			IP = address;
			this->port = port;
			socket_ptr = std::make_shared<socket>(*context_ptr);
			auto [ep, ec] = make_endpoint(IP, port);
			if (ec) {
				socket_ptr->close();
				return false;
			}
			else {
				socket_ptr->connect(ep, ec);
			}
			if (ec) {
				socket_ptr->close();
				ui::msgbox(gbk2utf8(ec.what()));
				return false;
			}
			return true;
		}
		bool connected() const {
			return socket_ptr != nullptr && socket_ptr->is_open();
		}
		string get_name() { return name; }
		
		boost::cobalt::task<void> pass_msg(std::string message) override{
			co_await pass_move(move{ .mvtype = move::str,.msg = message });
		}
		boost::cobalt::task<void> pass_move(move mv) {
			if (mv.mvtype == move::quit) {
				socket_ptr->close();
				rdq->tim.cancel();
				wtq->tim.cancel();
			}
			else {
				wtq->q.push_back(mv);
				wtq->tim.cancel_one();
			}
			co_return;
		}
		virtual void cancel() override {
			rdq->q.push_back({ .mvtype = move::quit });
			rdq->tim.cancel();
		}
		void register_handle_move(move_handler_ptr ptr) {
			handle_move_ = ptr;
		}
		void clear_handler() { handle_move_ = nullptr; }
		void post(std::function<void()> task) const { context_ptr->post(task); }
		virtual bool good()const override { return context_ptr != nullptr && connected(); }
		virtual void reset() override {}
		timdq_ptr rdq, wtq;
		std::shared_ptr<acceptor> acceptor_ptr = nullptr;
		std::shared_ptr<socket> socket_ptr = nullptr;
		std::shared_ptr<context> context_ptr = nullptr;
		std::shared_ptr<std::jthread> thread_ptr = nullptr;
		std::string IP = "127.0.0.1";
		unsigned int port = 22222;
		virtual ~remote_tcp_gamer() {
			if (socket_ptr->is_open()) {
				//socket_ptr->shutdown(ip::tcp::socket::shutdown_receive);
				socket_ptr->close();
			}
			rdq->tim.cancel();
			wtq->tim.cancel();
		}
	private:
		move_handler_ptr handle_move_ = nullptr;
	};
}
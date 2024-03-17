#pragma once
#include"stdafx.h"
#include"message.hpp"
#include"online.room.hpp"
#include"components.h"
namespace bw::online {
	class gamer :public gamer_info {
	public:
		using socket_t = boost::asio::ip::tcp::socket;
		using socket_ptr = std::shared_ptr<socket_t>;
		using endpoint = boost::asio::ip::tcp::endpoint;
		using context = boost::asio::io_context;
		gamer() {
			context_ptr = std::make_shared<context>();
			socket = std::make_shared<socket_t>(*context_ptr);
			timer_ = std::make_shared<boost::asio::steady_timer>(*context_ptr);
			timer_->expires_at(std::chrono::steady_clock::time_point::max());
			mutex_ = std::make_shared<std::mutex>();
			conv = std::make_shared<std::condition_variable>();
		};
		inline std::string gbk2utf8(std::string_view s) {
			return boost::locale::conv::to_utf<char>(s.data(), "gbk");
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
			return std::make_tuple(ep, ec);
		}
		bool connect(std::string_view Address, int Port) {
			using namespace boost::asio::ip;
			hall.address = Address;
			hall.port = Port;
			socket = std::make_shared<socket_t>(*context_ptr);
			auto [ep, ec] = make_endpoint(hall.address, hall.port);
			if (ec) {
				ui::msgbox(std::format("没有找到服务器，请检查地址或您的网络连接 Error code:{}", ec.value()));
				return false;
			}
			else {
				socket->connect(ep, ec);
			}
			if (ec) {
				ui::msgbox(std::format("服务器响应异常，请检查地址或您的网络连接 Error code:{}", ec.value()));
				return false;
			}
			return true;
		}
		inline bool connected() const {
			return socket != nullptr && socket->is_open();
		}
		auto get_executor() { return context_ptr; }
		void deliver(const message& msg)
		{
			write_msg_queue.push_back(msg);
			timer_->cancel_one();
		}
		void safe_deliver(const message& msg) {
			context_ptr->post([this, &msg] {deliver(msg); });
		}
		void start()
		{
			/*boost::asio::co_spawn(*context_ptr, [this] {return reader(); }, boost::asio::detached);
			boost::asio::co_spawn(*context_ptr, [this] {return writer(); }, boost::asio::detached);*/

			boost::cobalt::spawn(*context_ptr, reader(), boost::asio::detached);
			boost::cobalt::spawn(*context_ptr, writer(), boost::asio::detached);
			thread_ptr = std::make_shared<std::jthread>([this] {context_ptr->run(); });
		}
		inline void handle_msg(const message& msg);//Implement below
		void login() {
			std::string jsonstr;
			iguana::to_json(static_cast<gamer_info>(*this), jsonstr);
			boost::system::error_code ec;
			socket->wait(socket_t::wait_write, ec);
			if (!ec) {
				deliver(wrap(control_msg{
				.type = control_msg::create,
				.content = jsonstr,
				.target_type = control_msg::g,
					}, msg_t::control));
				infostate = outdated;
			}
		}
		void try_update_info(const gamer_info& info) {
			std::string infostr;
			iguana::to_json(info, infostr);
			deliver(wrap(control_msg{
				.type = control_msg::update,
				.content = infostr,
				.id1 = id,
				.target_type = control_msg::g
				}, msg_t::control));
			infostate = outdated;
		}
		void try_update_name(std::string_view namesv) {
			gamer_info info = *this;
			info.name = namesv;
			try_update_info(info);
		}
		void try_join_room(int roomID) {
			if (roomID > 0 && roomID < hall.rooms.size()) {
				deliver(wrap(control_msg{
					.type = control_msg::join,
					.id1 = id,
					.id2 = roomID,
					},
					msg_t::control
				));
				infostate = outdated;
			}
		}
		void get(std::string gettype, std::vector<int> ids = {}) {
			if (gettype == "room_info") {
				deliver(wrap(
					get_msg{
						.get_type = gettype,
						.ids = std::move(ids)
					},
					msg_t::get
				));
			}
			else if (gettype == "gamer_info") {
				deliver(wrap(
					get_msg{
						.get_type = gettype,
						.ids = std::move(ids)
					},
					msg_t::get
				));
			}
			else if (gettype == "notices") {
				deliver(wrap(
					get_msg{
						.get_type = gettype,
					},
					msg_t::get
				));
			}
			else {

			}
			return;
		}
		void try_leave() {
			deliver(wrap(
				control_msg{
					.type = control_msg::leave,
					.id1 = id,
					.target_type = control_msg::g
				},
				msg_t::control
			));
			infostate = outdated;
		}
		void broadcast(const std::string& str) {
			if ((roomid == 0 && authority == admin) || roomid > 0) {
				deliver(wrap(
					str_msg{
						.content = str,
						.target_type = str_msg::r,
						.id1 = id,
						.id2 = roomid,
						.name = name
					},
					msg_t::str
				));
			}
		}
		void refresh_screen() const {
			if (screen_ptr) {
				screen_ptr->PostEvent(ftxui::Event::Custom);
			}
			else [[unlikely]]{
				throw std::runtime_error("Fatal error: Invalid screen!");
			}
		}

		void stop() {
			if (socket->is_open()) {
				socket->close();
			}
			if (timer_ != nullptr) {
				timer_->cancel();
			}
			if (!context_ptr->stopped()) {
				context_ptr->stop();
			}
		}
		~gamer() {
			if (socket->is_open()) {
				socket->close();
			}
			if (!context_ptr->stopped()) {
				context_ptr->stop();
			}
		}
		hall_info hall;
		int roomid = 0;
		std::vector<std::string> notices;
		std::shared_ptr<std::condition_variable> conv;
		const int sizelen = 5;
		ftxui::ScreenInteractive* screen_ptr = nullptr;
		enum { outdated, updated };
		int infostate = outdated;
		std::deque<str_msg> chat_msg_queue;
	private:
		boost::cobalt::task<void> reader()
		{
			using namespace boost::asio;
			std::string read_str, sizestr;
			msg_t msg;
			while (true) {
				try {
					co_await async_read(*socket, dynamic_buffer(sizestr, sizelen), boost::cobalt::use_op);
					co_await async_read(*socket, dynamic_buffer(read_str, std::stoi(sizestr)), boost::cobalt::use_op);
				}
				catch (const std::exception& e) {
					std::println("Error:{}", e.what());
					stop();
					co_return;
				}
				try {
					struct_json::from_json(msg, read_str);
				}
				catch (const std::exception& e)
				{
					ui::msgbox(std::format("Data structure damaged. msg:{}", gbk2utf8(e.what())));
					sizestr = "";
					read_str = "";
					continue;
				}
				try {
					handle_msg(msg);
				}
				catch (const std::exception& e)
				{
					ui::msgbox(std::format("Failed to process data. msg:{}", gbk2utf8(e.what())));
				}
				sizestr = "";
				read_str = "";
			}
		}
		boost::cobalt::task<void> writer()
		{
			using namespace boost::asio;
			using namespace std::string_literals;
			try
			{
				std::string write_str;
				while (socket->is_open())
				{
					if (write_msg_queue.empty())
					{
						boost::system::error_code ec;
						co_await timer_->async_wait(redirect_error(boost::cobalt::use_op, ec));
					}
					else
					{
						write_str = "";
						iguana::to_json(write_msg_queue.front(), write_str);
						co_await boost::asio::async_write(*socket, boost::asio::buffer(std::format("{0:<{1}}", write_str.size(), sizelen), sizelen), boost::cobalt::use_op);
						co_await boost::asio::async_write(*socket, boost::asio::dynamic_buffer(write_str, write_str.size()), boost::cobalt::use_op);
						write_msg_queue.pop_front();
					}
				}
			}
			catch (std::exception& e)
			{
				//std::print(stderr, "{}", e.what());
				ui::msgbox("Failed to send ... Error: "s + e.what());
				conv->notify_all();
				stop();
			}
		}
		
		socket_ptr socket;
		
		std::shared_ptr<context> context_ptr;
		std::shared_ptr<std::jthread> thread_ptr;
		std::shared_ptr<boost::asio::steady_timer> timer_;
		std::shared_ptr<std::mutex> mutex_;
		std::deque<message> write_msg_queue;
	};
	inline void gamer::handle_msg(const message& msg){
		int type = msg.type;
		if (type == msg_t::invalid)
			throw std::runtime_error("Invalid Control Message");
		if (type == msg_t::control) {
			control_msg con_m;
			struct_json::from_json(con_m, msg.jsonstr);
			if (con_m.type == control_msg::create) {
				if (con_m.target_type == control_msg::g) {
					gamer_info newinfo;
					struct_json::from_json(newinfo, con_m.content);
					gamer_info& self = *this;
					self = newinfo;
					assert(newinfo.id == con_m.id1);
					infostate = updated;
				}
			}
			else if (con_m.type == control_msg::update) {
				if (con_m.target_type == control_msg::g) {
					gamer_info newinfo;
					struct_json::from_json(newinfo, con_m.content);
					gamer_info& self = *this;
					self = newinfo;
					assert(newinfo.id == con_m.id1);
					infostate = updated;
					refresh_screen();
				}
				else if (con_m.target_type == control_msg::r) {
					if (con_m.id2 == 0) {
						
					}
					else {

					}
				}
				else {

				}
			}
			else if (con_m.type == control_msg::del) {
				if (con_m.target_type == control_msg::r) {

				}
				else if (con_m.target_type == control_msg::g) {
					
				}
				else {

				}
			}
			else if (con_m.type == control_msg::join) {
				roomid = con_m.id2;
				screen_ptr->PostEvent(ftxui::Event::Special("RefreshChat"));
			}
			else if (con_m.type == control_msg::leave) {
				if (roomid > 0) {
					roomid = 0;
					chat_msg_queue.clear();
				}
				else {
					roomid = -1;
				}
				screen_ptr->PostEvent(ftxui::Event::Special("RefreshRoomInfo"));
				chat_msg_queue.clear();
			}
			else if (con_m.type == control_msg::none) {

			}
			else {

			}
		}
		else if (type == msg_t::str) {
			str_msg strmsg;
			struct_json::from_json(strmsg, msg.jsonstr);
			if (strmsg.target_type == str_msg::g) {
				if (strmsg.id1 == 0) {
					if (strmsg.id2 == -1) {
						notices.push_back(strmsg.content);
						refresh_screen();
					}
					else {
						
					}
				}
				else {
					
				}
			}
			else {
				chat_msg_queue.push_back(std::move(strmsg));
				screen_ptr->PostEvent(ftxui::Event::Special("AddChat"));
				refresh_screen();
			}
		}
		else if (type == msg_t::game) {

		}
		else if (type == msg_t::ret) {
			ret_msg rmsg;
			struct_json::from_json(rmsg, msg.jsonstr);
			if (rmsg.ret_type == "room_info") {
				std::vector<room_info> infos;
				struct_json::from_json(infos, rmsg.ret_str);
				roomid = infos.front().id;
				hall.rooms.clear();
				for (int i = 1; i < infos.size(); ++i) {
					hall.rooms.emplace_back(infos[i]);
				}
				infostate = updated;
				hall.infostate = hall_info::updated;
				screen_ptr->PostEvent(ftxui::Event::Special("RefreshRoomInfo"));
				refresh_screen();
			}
		}
		else {

		}
	}
};

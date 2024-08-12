#pragma once
#include"stdafx.h"
#include"game.hpp"
#include"net/message.hpp"
#include"online/room.hpp"
#include"safe_vector.hpp"
#include"tui/components.hpp"
#include"basic_user.hpp"
#include"basic_online_gamer.hpp"
#include"boost/container/flat_map.hpp"
namespace bw::online {
	class user :public basic_user, public std::enable_shared_from_this<user> {
	public:
		using socket_t = boost::asio::ip::tcp::socket;
		using socket_ptr = std::shared_ptr<socket_t>;
		using endpoint = boost::asio::ip::tcp::endpoint;
		using context = boost::asio::io_context;
		using str_channel = boost::cobalt::channel<std::string>;
		using channel_ptr = std::shared_ptr<str_channel>;
		using filter_func = std::function<void(const message&)>;
		user() :context_ptr(std::make_shared<context>()), socket(*context_ptr), timer_(*context_ptr),  chat_mutex(0), _gen(0, INT_MAX) {
			timer_.expires_at(std::chrono::steady_clock::time_point::max());
			stop_flag.clear();
			crt_room_info.store(default_room_info());
			search_rinfo_res.store(default_room_info());
		};
		user(std::shared_ptr<context> io) :context_ptr(std::move(io)), socket(*io), timer_(*io), chat_mutex(0), _gen(0, INT_MAX) {
			timer_.expires_at(std::chrono::steady_clock::time_point::max());
			stop_flag.clear();
			crt_room_info.store(default_room_info());
			search_rinfo_res.store(default_room_info());
		}
		std::string gbk2utf8(std::string_view s) {
			return boost::locale::conv::to_utf<char>(s.data(), "gbk");
		}
		int rand() {
			return _gen(_rnd);
		}
		std::string rand_str() {
			return std::to_string(rand());
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
			socket = socket_t(*context_ptr);
			auto [ep, ec] = make_endpoint(hall.address, hall.port);
			if (ec) {
				socket.close();
				ui::msgbox(std::format("{}:{}",gettext("Server not found. Please check the address or your network connection. Error code:"), ec.value()));
				return false;
			}
			else {
				socket.connect(ep, ec);
			}
			if (ec) {
				socket.close();
				ui::msgbox(std::format("{}:{}", gettext("The server response is abnormal. Please check the address or your network connection. Error code:"), ec.value()));
				return false;
			}
			return true;
		}
		inline bool connected() const {
			return socket.is_open();
		}
		bool in_room() const {
			return crt_room_info.load().id != 0;
		}
		auto get_executor() { return context_ptr; }

		void post(std::function<void()> handler) {
			context_ptr->post(std::move(handler));
		}

		void deliver(const message& msg) override {
			write_msg_queue.push_back(msg);
			timer_.cancel_one();
		}
		void safe_deliver(const message& msg) {
			context_ptr->post([this, &msg] {deliver(msg); });
		}

		void start() {
			boost::cobalt::spawn(*context_ptr, cobalt_reader(), boost::asio::detached);
			boost::cobalt::spawn(*context_ptr, cobalt_writer(), boost::asio::detached);
			/*boost::asio::co_spawn(*context_ptr, asio_reader(), boost::asio::detached);
			boost::asio::co_spawn(*context_ptr, asio_writer(), boost::asio::detached);*/
			chat_mutex.release();
			thread_ptr = std::make_shared<std::jthread>(
				[this] {
					context_ptr->run(); 
					spdlog::info("user thread stopped.");
				});
		}
		inline void handle_msg(const message& msg);//Implement below
		void login() {
			std::string jsonstr;
			iguana::to_json(static_cast<user_info>(*this), jsonstr);
			boost::system::error_code ec;
			socket.wait(socket_t::wait_write, ec);
			if (!ec) {
				infostate = outdated;
				deliver(wrap(
					control_msg{
						.type = control_msg::create,
						.content = jsonstr,
						.target_type = control_msg::g,
					},
					msg_t::control
				));
			}
		}
		void try_update_info(const user_info& info) {
			infostate = outdated;
			std::string infostr;
			struct_json::to_json(info, infostr);
			deliver(wrap(control_msg{
				.type = control_msg::update,
				.content = infostr,
				.id1 = id,
				.target_type = control_msg::g
				}, msg_t::control));
		}
		void try_update_name(std::string new_name) {
			user_info info = *this;
			info.name = std::move(new_name);
			try_update_info(info);
		}
		void try_join_room(int roomID) {
			if (roomID > 0) {
				infostate = outdated;
				deliver(wrap(control_msg{
					.type = control_msg::join,
					.id1 = id,
					.id2 = roomID,
					},
					msg_t::control
				));
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
			else if (gettype == "current_room_info") {
				deliver(wrap(
					get_msg{
						.get_type = gettype,
					},
					msg_t::get
				));
			}
			else if (gettype == "search_room_info") {
				deliver(wrap(
					get_msg{
						.get_type = gettype,
						.ids = std::move(ids),
					},
					msg_t::get
				));
			}
			else if (gettype == "user_info") {
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
			infostate = outdated;
			deliver(wrap(
				control_msg{
					.type = control_msg::leave,
					.id1 = id,
					.target_type = control_msg::g
				},
				msg_t::control
			));
		}
		void broadcast(std::string str) {
			auto rinfo = crt_room_info.load();
			if ((rinfo.id == 0 && authority == admin) || rinfo.id > 0) {
				deliver(wrap(
					str_msg{
						.content = str,
						.target_type = str_msg::r,
						.id1 = id,
						.id2 = rinfo.id,
						.name = name
					},
					msg_t::str
				));
			}
		}
		void refresh_screen() const {
			if (scr_ptr && scr_ptr->good()) {
				scr_ptr->refresh();
			}
		}
		//void add_filter(const std::string& tag, filter_func pred) {
		//	filters[tag] = (std::move(pred));
		//}
		//void del_filter(const string& tag) {
		//	filters.erase(tag);
		//}
		//void clear_filter() {
		//	filters.clear();
		//}
		void stop() {
			if (socket.is_open()) {
				socket.close();
			}
			timer_.cancel();
			if (!context_ptr->stopped()) {
				context_ptr->stop();
			}
		}

		~user() {
			if (socket.is_open()) {
				socket.close();
			}
			timer_.cancel();
			if (context_ptr && !context_ptr->stopped()) {
				context_ptr->stop();
			}
			signals::end_game.disconnect_all_slots();
			signals::start_game.disconnect_all_slots();
			spdlog::info("user_destructed");
		};
		
		//boost::signals2::signal<void()> refresh_room_info, search_roon_info;

		using ID = int;
		
		hall_info hall;

		std::atomic_flag stop_flag;

		thread_safe::vector<std::string> notices;

		const int sizelen = 5;

		bw::components::ftxui_screen_ptr scr_ptr = nullptr;

		enum { outdated, latest };
		int infostate = outdated;

		std::deque<str_msg> chat_msg_queue;
		std::binary_semaphore chat_mutex;

		//std::unordered_map<std::string, filter_func> filters;
		boost::signals2::signal<void(const message&)> read_msg;
		boost::signals2::signal<void(int)> prepare_watch_game;
		boost::signals2::signal<void()> show_room_chats;

		std::string game_type, game_info_str;
		using chan_gamer_ptr = std::pair<core::str_dq_ptr, basic_gamer_ptr>;
		std::unordered_map<ID, chan_gamer_ptr> chan_gamer_map;
		std::atomic<room_info> crt_room_info, search_rinfo_res;
	private:
		
		boost::cobalt::task<void> cobalt_reader();
		boost::cobalt::task<void> cobalt_writer();

		boost::cobalt::task<void> heart_beat() {//unused
			boost::asio::steady_timer t(*context_ptr);
			while (socket.is_open()) {
				using namespace std;
				t.expires_after(2min);
				co_await t.async_wait(boost::cobalt::use_op);
			}
		}

		void handle_game_msg(const message&);
		
		std::random_device _rnd;
		std::uniform_int_distribution<int> _gen;
		std::shared_ptr<context> context_ptr;
		socket_t socket;
		std::shared_ptr<std::jthread> thread_ptr;
		boost::asio::steady_timer timer_;
		std::deque<message> write_msg_queue;
	};
	inline void user::handle_game_msg(const message& msg){

	}
	inline void user::handle_msg(const message& msg){
		int type = msg.type;
		if (type == msg_t::invalid)
			throw std::runtime_error("Invalid Control Message");
		if (type == msg_t::control) {
			control_msg con_m;
			struct_json::from_json(con_m, msg.jsonstr);
			if (con_m.type == control_msg::create) {
				if (con_m.target_type == control_msg::g) {
					user_info newinfo;
					struct_json::from_json(newinfo, con_m.content);
					user_info& self = *this;
					self = newinfo;
					assert(newinfo.id == con_m.id1);
					infostate = latest;
				}
			}
			else if (con_m.type == control_msg::update) {
				if (con_m.target_type == control_msg::g) {//修改user信息
					user_info newinfo;
					struct_json::from_json(newinfo, con_m.content);
					user_info& self = *this;
					self = newinfo;
					assert(newinfo.id == con_m.id1);
					infostate = latest;
				}
				else if (con_m.target_type == control_msg::r) {//修改room信息
					if (con_m.id2 == 0) {
						
					}
					else {

					}
				}
				else {

				}
			}
			else if (con_m.type == control_msg::del) {//删除
				if (con_m.target_type == control_msg::r) {//删除room

				}
				else if (con_m.target_type == control_msg::g) {//删除user
					
				}
				else {

				}
			}
			else if (con_m.type == control_msg::join) {//加入room
				room_info rinfo;
				struct_json::from_json(rinfo, con_m.content);
				crt_room_info.store(rinfo);
				infostate = latest;
				scr_ptr->post_event("RefreshChat");
				show_room_chats();
			}
			else if (con_m.type == control_msg::leave) {//离开room
				auto rinfo = crt_room_info.load();
				if (rinfo.id > 0) {
					crt_room_info.store(default_room_info());
					chat_mutex.acquire();
					chat_msg_queue.clear();
					chat_mutex.release();
				}
				else {
					crt_room_info.store(default_room_info());
				}
				//scr_ptr->post_event("RefreshRoomInfo");
				get("current_room_info");
				chat_mutex.acquire();
				chat_msg_queue.clear();
				chat_mutex.release();
			}
			else if (con_m.type == control_msg::none) {

			}
			else {

			}
			refresh_screen();
		}
		else if (type == msg_t::str) {
			str_msg strmsg;
			struct_json::from_json(strmsg, msg.jsonstr);
			if (strmsg.target_type == str_msg::g) {
				
			}
			else {
				chat_mutex.acquire();
				chat_msg_queue.push_back(std::move(strmsg));
				chat_mutex.release();
				scr_ptr->post_event("AddChat");
			}
			refresh_screen();
		}
		else if (type == msg_t::game) {
			game_msg gmmsg;
			struct_json::from_json(gmmsg, msg.jsonstr);
			switch (gmmsg.type) {
			case game_msg::create://弃用
				game_type = gmmsg.movestr;
				game_info_str = gmmsg.board;
				break;
			case game_msg::watch:
				state = user_st::watching;
				infostate = latest;
				break;
			case game_msg::prepare://根据发过来的user_info，生成gamer,加入gamers。
			{
				basic_gamer_info gamer_info;
				struct_json::from_json(gamer_info, gmmsg.movestr);
				if (Game_ptr == nullptr) {
					if (state == user_st::watching) {
						prepare_watch_game(gamer_info.gametype);
					}
					else {
						spdlog::error("null Game component.");
						ui::msgbox(gettext("Error: Invalid Game"));
						return;
					}
				}
				if (gmmsg.id == id) {
					gamer_info.gamertype = basic_gamer::human;
					Game_ptr->join(Game_ptr->gamer_from_info(gamer_info));
				}
				else {
					basic_gamer_ptr gmr_ptr = Game_ptr->gamer_from_info(gamer_info);
					basic_online_gamer_ptr ol_gamer =
						std::dynamic_pointer_cast<basic_online_gamer>(gmr_ptr);
					ol_gamer->set_read_queue(std::make_shared<core::str_dq>(context_ptr));
					ol_gamer->set_user(shared_from_this());
					chan_gamer_map[gmr_ptr->id] = { ol_gamer->rd_dq,gmr_ptr };
					Game_ptr->join(gmr_ptr);
				}
			}
				break;
			case game_msg::start://根据prepare的gamers生成game和Game，并且启动
			{
				std::stringstream ss(gmmsg.board);
				std::string game_name, brd_sizestr;
				ss >> game_name >> brd_sizestr;
				Game_ptr->set_board_size(std::stoi(brd_sizestr));
				signals::start_game();
				if (state != user_st::watching) {
					state = user_st::gaming;
				}
			}
				break;
			case game_msg::move:
			{
				if (gmmsg.id != id) {
					auto& chan = chan_gamer_map.at(gmmsg.id);
					chan.first->q.push_back(gmmsg.movestr);
					chan.first->tim.cancel();
				}
			}
				break;
			case game_msg::end:
				chan_gamer_map.clear();
				if(gm_ptr)
					gm_ptr->end_game();
				break;
			default:
				;
			}
		}
		else if (type == msg_t::notice) {
			notice_msg nmsg;
			struct_json::from_json(nmsg, msg.jsonstr);
			notices.push_back(nmsg.str);
			if (scr_ptr && scr_ptr->good()) {
				scr_ptr->refresh();
			}
		}
		else if (type == msg_t::ret) {
			ret_msg rmsg;
			struct_json::from_json(rmsg, msg.jsonstr);
			if (rmsg.ret_type == "room_info") {
				std::vector<room_info> infos;
				struct_json::from_json(infos, rmsg.ret_str);
				crt_room_info.store(infos.front());
				hall.rooms.clear();
				for (int i = 1; i < infos.size(); ++i) {
					hall.rooms.emplace_back(infos[i]);
				}
				infostate = latest;
				hall.infostate = hall_info::latest;
				scr_ptr->post_event("RefreshRoomInfo");
				refresh_screen();
			}
			else if (rmsg.ret_type == "current_room_info") {
				if (rmsg.value == success) {
					std::vector<room_info> infos;
					struct_json::from_json(infos, rmsg.ret_str);
					crt_room_info.store(infos.front());
				}
				else {
					crt_room_info.store(default_room_info());
				}
				infostate = latest;
				refresh_screen();
			}
			else if (rmsg.ret_type == "search_room_info") {
				if (rmsg.value == success) {
					std::vector<room_info> infos;
					struct_json::from_json(infos, rmsg.ret_str);
					search_rinfo_res.store(infos.front());
				}
				else {
					search_rinfo_res.store(default_room_info());
					ui::msgbox(gettext("Room not found"));
				}
				infostate = latest;
				refresh_screen();
			}
			else if (rmsg.ret_type == "user_info") {
				std::vector<basic_gamer> infos;
				
			}
			else if (rmsg.ret_type == "notices") {
				std::vector<message> msgs;
				struct_json::from_json(msgs, rmsg.ret_str);
				std::vector<std::string> notices;
				notice_msg notice;
				for (auto& msg : msgs) {
					struct_json::from_json(notice, msg.jsonstr);
					notices.push_back(notice.str);
				}
				this->notices.assign(notices.begin(), notices.end());
				refresh_screen();
			}
			else if (rmsg.ret_type == "join_room_failed"){
				ui::msgbox(gettext("Room not found"));
			}
			else if (rmsg.ret_type == "operation_failed") {
				ui::msgbox(gettext("Server message: ") + rmsg.ret_str);
			}
			else if (rmsg.ret_type == "match_failed"){
				ui::msgbox(gettext("You can only match in a room"));
			}
		}
		else {

		}
	}
	using user_ptr = std::shared_ptr<user>;
#pragma optimize("", off)
	boost::cobalt::task<void> user::cobalt_reader()
	{
		std::shared_ptr<user> self = shared_from_this();
		using namespace boost::asio;
		std::string read_str, read_size_str;
		msg_t msg;
		while (true) {
			try {
				co_await async_read(self->socket, dynamic_buffer(read_size_str, self->sizelen), boost::cobalt::use_op);
				co_await async_read(self->socket, dynamic_buffer(read_str, std::stoi(read_size_str)), boost::cobalt::use_op);
			}
			catch (const std::exception& e) {
				if (!stop_flag.test()) {
					ui::msgbox(fmt::format("{}:{}", gettext("Error"), gbk2utf8(e.what())));
					self->stop();
				}
				co_return;
			}
			try {
				struct_json::from_json(msg, read_str);
			}
			catch (const std::exception& e)
			{
				ui::msgbox(fmt::format("{}:{}", gettext("Data structure damaged. msg"), gbk2utf8(e.what())));
				spdlog::error("msg:{} error:{}", msg.jsonstr, e.what());
				read_size_str = "";
				read_str = "";
				continue;
			}

			try {
				read_msg(msg);
			}
			catch (const std::exception& e)
			{
				ui::msgbox(std::format("{}:{}", gettext("Bad Filter. msg"), gbk2utf8(e.what())));
				spdlog::error("msg:{} error:{}", msg.jsonstr, e.what());
			}

			try {
				self->handle_msg(msg);
			}
			catch (const std::exception& e)
			{
				ui::msgbox(std::format("{}:{}", gettext("Failed to process data. msg"), gbk2utf8(e.what())));
				spdlog::error("msg:{} error:{}", msg.jsonstr, e.what());
			}
			read_size_str = "";
			read_str = "";
		}
	}
	boost::cobalt::task<void> user::cobalt_writer()
	{
		std::shared_ptr<user> self = shared_from_this();
		using namespace boost::asio;
		using namespace std::string_literals;
		try
		{
			std::string write_str, write_size_str;
			while (self->socket.is_open())
			{
				if (self->write_msg_queue.empty())
				{
					boost::system::error_code ec;
					co_await self->timer_.async_wait(redirect_error(boost::cobalt::use_op, ec));
				}
				else
				{
					write_str = "";
					write_size_str = "";
					struct_json::to_json(self->write_msg_queue.front(), write_str);
					/*size_str = std::to_string(write_str.size());
					size_str = size_str + std::string(sizelen - size_str.size(), ' ');
					size_str = std::format("{0:<{1}}", write_str.size(), sizelen);*/
					write_size_str = std::format("{:<{}}", write_str.size(), self->sizelen);
					co_await boost::asio::async_write(self->socket, boost::asio::dynamic_buffer(write_size_str, self->sizelen), boost::cobalt::use_op);
					co_await boost::asio::async_write(self->socket, boost::asio::dynamic_buffer(write_str, write_str.size()), boost::cobalt::use_op);
					self->write_msg_queue.pop_front();
				}
			}
		}
		catch (std::exception& e)
		{
			ui::msgbox(std::string(gettext("Failed to send ... Error: ")) + e.what());
			self->stop();
		}
	}
#pragma optimize("", on)
};
//boost::asio::awaitable<void> asio_reader(socket_ptr socket)
		//{
		//	using namespace boost::asio;
		//	std::string read_str, read_size_str;
		//	msg_t msg;
		//	while (true) {
		//		try {
		//			co_await async_read(*socket, dynamic_buffer(read_size_str, sizelen), boost::asio::use_awaitable);
		//			co_await async_read(*socket, dynamic_buffer(read_str, std::stoi(read_size_str)), boost::asio::use_awaitable);
		//		}
		//		catch (const std::exception& e) {
		//			ui::msgbox(fmt::format("{}:{}", gettext("Error"), gbk2utf8(e.what())));
		//			stop();
		//			co_return;
		//		}
		//		try {
		//			struct_json::from_json(msg, read_str);
		//		}
		//		catch (const std::exception& e)
		//		{
		//			ui::msgbox(fmt::format("{}:{}",gettext("Data structure damaged. msg"), gbk2utf8(e.what())));
		//			read_size_str = "";
		//			read_str = "";
		//			continue;
		//		}
		//		try {
		//			for (auto& [_, p] : filters) {
		//				p(msg);
		//			}
		//		}
		//		catch (const std::exception& e)
		//		{
		//			ui::msgbox(std::format("{}:{}", gettext("Bad Filter. msg"), gbk2utf8(e.what())));
		//		}
		//		try {
		//			handle_msg(msg);
		//		}
		//		catch (const std::exception& e)
		//		{
		//			ui::msgbox(std::format("{}:{}",gettext("Failed to process data. msg"), gbk2utf8(e.what())));
		//		}
		//		read_size_str = "";
		//		read_str = "";
		//	}
		//}
		//boost::asio::awaitable<void> asio_writer(socket_ptr socket)
		//{
		//	using namespace boost::asio;
		//	using namespace std::string_literals;
		//	
		//	try
		//	{
		//		while (socket.is_open())
		//		{
		//			if (write_msg_queue.empty())
		//			{
		//				boost::system::error_code ec;
		//				co_await timer_->async_wait(redirect_error(boost::asio::use_awaitable, ec));
		//			}
		//			else
		//			{
		//				write_str = "";
		//				write_size_str = "";
		//				struct_json::to_json(write_msg_queue.front(), write_str);
		//				/*size_str = std::to_string(write_str.size());
		//				size_str = size_str + std::string(sizelen - size_str.size(), ' ');*/
		//				//size_str = std::format("{0:<{1}}", write_str.size(), sizelen);
		//				write_size_str = fmt::format("{:<{}}", write_str.size(), sizelen);
		//				co_await boost::asio::async_write(*socket, boost::asio::dynamic_buffer(write_size_str, sizelen), boost::asio::use_awaitable);
		//				co_await boost::asio::async_write(*socket, boost::asio::dynamic_buffer(write_str, write_str.size()), boost::asio::use_awaitable);
		//				write_msg_queue.pop_front();
		//			}
		//		}
		//	}
		//	catch (std::exception& e)
		//	{
		//		ui::msgbox(std::string(gettext("Failed to send ... Error: ")) + e.what());
		//		stop();
		//	}
		//}
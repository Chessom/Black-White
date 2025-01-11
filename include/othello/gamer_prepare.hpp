#pragma once
#include"tui/game/gamer_prepare.hpp"
#include"othello/gamer/computer.hpp"
#include"othello/gamer/remote_tcp.hpp"
#include"othello/gamer/lua_gamer.hpp"
#include"othello/gamer/python_gamer.hpp"
#include"othello/gamer/http_gamer.hpp"
#include"tui/text_editor.hpp"
#include"tui/components.hpp"
namespace bw::components {
	template<>
	ftxui::Component GamerSetting<othello::lua_gamer>(basic_gamer_ptr _p) {
		using namespace ftxui;
		std::shared_ptr<std::string> error_str = std::make_shared<std::string>("");
		othello::lua_gamer_ptr gptr = std::dynamic_pointer_cast<othello::lua_gamer>(_p);
		{
			std::ifstream fin;
			fin.open(gptr->default_script_path(), std::ios::in);
			if (fin.is_open()) {
				std::stringstream ss;
				ss << fin.rdbuf();
				gptr->script = ss.str();
			}
			fin.close();
		}
		TextEditorOption option = TextEditorOption::Default();
		option.line_number_renderer = [](int number) {
			return text(std::to_string(number)) | color(Color::Cyan);
		};
		Component editor = TextEditor(&gptr->script, gettext("Code Lua here"), option) | flex | borderRounded;
		Component error_msg = Renderer([error_str] {
			if (error_str->empty()) {
				return window(text(gettext("Error Output")), text(gettext("No errors")) | color(Color::Green));
			}
			else {
				return window(text(gettext("Error Output")), ui::autolines(*error_str) | color(Color::RedLight));
			}
		});
		Component Buttons = Container::Vertical({
			Button(gettext("Save Script"),[gptr, error_str] {
				std::ofstream fout;
				fout.open(gptr->default_script_path(), std::ios::out);
				if (fout.is_open()) {
					fout << gptr->script;
				}
				else {
					*error_str = gettext("Failed to save script to ") + gptr->default_script_path();
				}
				fout.close();
				},ButtonOption::Animated(Color::GreenLight)) | hcenter,
			Button(gettext("Test Script"),
				[gptr, error_str] {
				try {
					gptr->test();
					error_str->clear();
				}
				catch (const sol::error& e) {
					*error_str = e.what();
					return;
				}
				catch (const std::exception& e) {
					*error_str = e.what();
					return;
				}
				},ButtonOption::Animated(Color::Blue)) | hcenter,
			Button(gettext("Load Script"),[gptr,error_str] {
				std::ifstream fin;
				fin.open(gptr->default_script_path(), std::ios::in);
				if (!fin) {
					ui::msgbox(gettext("Failed to open script file: ") + gptr->default_script_path());
					fin.close();
					return;
				}
				std::stringstream ss;
				ss << fin.rdbuf();
				gptr->script = ss.str();
				fin.close();
			},ButtonOption::Animated()) | hcenter,
			Container::Vertical({
				ui::TextComp(gettext("File name:"),hcenter) | hcenter,
				Input(&gptr->script_filename,gettext("enter path")) | hcenter
				}) | border,
			Container::Vertical({
				ui::TextComp(gettext("get move function name:"),hcenter) | hcenter,
				Input(&gptr->getmove_func_signature,gettext("enter function signature")) | hcenter
				}) | border
		});
		return ResizableSplit(ResizableSplitOption{
			.main = Container::Vertical({error_msg | flex ,ui::ToCom(separator()),Buttons | hcenter }),
			.back = editor,
			.direction = Direction::Right,
			.main_size = 15
		});
	}

	template<>
	ftxui::Component GamerSetting<othello::python_gamer>(basic_gamer_ptr _p) {
		using namespace ftxui;
		namespace py = pybind11;
		std::shared_ptr<std::string> error_str = std::make_shared<std::string>("");
		othello::python_gamer_ptr gptr = std::dynamic_pointer_cast<othello::python_gamer>(_p);
		auto test_op = [gptr, error_str] {
			py::gil_scoped_acquire acquire;
			try {
				gptr->test();
				error_str->clear();
			}
			catch (const pybind11::error_already_set& e) {
				*error_str = e.what();
				return;
			}
			catch (const std::exception& e) {
				*error_str = e.what();
				return;
			}; 
		};
		{
			std::ifstream fin;
			fin.open(gptr->default_script_path(), std::ios::in);
			if (fin.is_open()) {
				std::stringstream ss;
				ss << fin.rdbuf();
				gptr->script = ss.str();
				test_op();
			}
			fin.close();
		}
		
		TextEditorOption option = TextEditorOption::Default();
		option.line_number_renderer = [](int number) {
			return text(std::to_string(number)) | color(Color::Cyan);
			};
		Component editor = TextEditor(&gptr->script, gettext("Code Python here"), option) | flex | borderRounded;
		Component error_msg = Renderer([error_str] {
			if (error_str->empty()) {
				return window(text(gettext("Error Output")), text(gettext("No errors")) | color(Color::Green));
			}
			else {
				return window(text(gettext("Error Output")), ui::autolines(*error_str) | color(Color::RedLight));
			}
			});
		Component Buttons = Container::Vertical({
			Button(gettext("Save Script"),[gptr, error_str] {
				std::ofstream fout;
				fout.open(gptr->default_script_path(), std::ios::out);
				if (fout.is_open()) {
					fout << gptr->script;
				}
				else {
					*error_str = gettext("Failed to save script to ") + gptr->default_script_path();
				}
				fout.close();
				},ButtonOption::Animated(Color::GreenLight)) | hcenter,
			Button(gettext("Test Script"),
				test_op
				,ButtonOption::Animated(Color::Blue)) | hcenter,
			Button(gettext("Load Script"),[gptr,error_str] {
				std::ifstream fin;
				fin.open(gptr->default_script_path(), std::ios::in);
				if (!fin) {
					ui::msgbox(gettext("Failed to open script file: ") + gptr->default_script_path());
					fin.close();
					return;
				}
				std::stringstream ss;
				ss << fin.rdbuf();
				gptr->script = ss.str();
				fin.close();
			},ButtonOption::Animated()) | hcenter,
			Container::Vertical({
				ui::TextComp(gettext("File name:"),hcenter) | hcenter,
				Input(&gptr->script_filename,gettext("enter path")) | hcenter
				}) | border,
			Container::Vertical({
				ui::TextComp(gettext("get_move function name:"),hcenter) | hcenter,
				Input(&gptr->getmove_func_signature,gettext("enter function signature")) | hcenter
				}) | border
			});
		return ResizableSplit(ResizableSplitOption{
			.main = Container::Vertical({error_msg | flex ,ui::ToCom(separator()),Buttons | hcenter }),
			.back = editor,
			.direction = Direction::Right,
			.main_size = 15
			});
	}
	
	template<>
	ftxui::Component GamerSetting<othello::remote_tcp_gamer>(basic_gamer_ptr _p) {
		using namespace ftxui;
		struct Impl :ftxui::ComponentBase{
		public:
			inline std::string gbk2utf8(std::string_view s) {
				return boost::locale::conv::to_utf<char>(s.data(), "gbk");
			}
			Impl(basic_gamer_ptr p) :
				remote_ptr(std::dynamic_pointer_cast<othello::remote_tcp_gamer>(p)),
				runningtim(*(remote_ptr->context_ptr))
			{
				auto toggle = Toggle(&vec, &selected_mode);
				
				auto active_connect = Container::Vertical({
					Container::Horizontal({
					Renderer([] {
						return vbox(text(gettext("Address:")) | border,text(gettext("Port:")) | border);
						}),
					Container::Vertical({
						Input(&IP, "localhost") | size(WIDTH,GREATER_THAN,16) | border,
						Input(&port, "22222") | size(WIDTH,GREATER_THAN,16) | border
						}),
					}) | center,
					Container::Horizontal({
					Button(gettext("Connect"),[this] {
						if (running) {
							ui::msgbox(gettext("Accepting Now"));
							ret = false;
							return;
						}
						try {
							if (remote_ptr->connected()) {
								remote_ptr->socket_ptr->close();
							}
							remote_ptr->connect(IP, std::stoi(port));
							prog = 1.0;
							ret = true;
						}
						catch (const std::exception& e) {
							ui::msgbox(boost::locale::conv::to_utf<char>(e.what(),"gbk"));
							ret = false;
						}
						},ButtonOption::Animated())
					}) | center ,
					Renderer([this] {return text(std::format("{}:[{}]",gettext("State"),ret ? gettext("Connected") : gettext("Disconnected"))) | center | border; })
					});
				auto co_ac = [this]()->boost::cobalt::task<void> {
					running = true;
					boost::system::error_code ec;
					*(remote_ptr->socket_ptr) = std::move(co_await remote_ptr->acceptor_ptr->async_accept(boost::asio::redirect_error(boost::cobalt::use_op, ec)));
					if (ec) {
						ret = false;
						running = false;
						runningtim.cancel();
						remote_ptr->socket_ptr = nullptr;
						co_return;
					}

					auto ep = remote_ptr->socket_ptr->remote_endpoint();
					remote_IP = ep.address().to_string();
					remote_port = std::to_string(ep.port());
					ret = true;
					running = false;
					ScreenInteractive::Active()->PostEvent(Event::Custom);
					runningtim.cancel();
					co_return;
					};

				runningtim.expires_from_now(boost::posix_time::hours(24));
				runningtim.async_wait([](boost::system::error_code) {});

				auto wait_for_connect = Container::Vertical({
					Container::Horizontal({
							Renderer([] {return text(gettext("Listening port:")) | border; }),
							Input(&port_listen, "22222") | size(WIDTH,GREATER_THAN,16) | border
						}) | center,
					Renderer([this] {
						return
						hbox(
							vbox(
								text(gettext("remote Address:")) | center | border,
								text(gettext("remote Port:")) | center | border
							),
							vbox(
								text(remote_IP) | size(WIDTH, GREATER_THAN, 16) | center | border,
								text(remote_port) | size(WIDTH, GREATER_THAN, 16) | center | border
							)
						) | center;
					}),
					Renderer([] {return separator(); }),
					Container::Horizontal({
						Button(gettext("Start Accept"),[this, co_ac] {
							if (!running) {
								try
								{
									boost::asio::ip::tcp::endpoint ep(boost::asio::ip::tcp::v4(), std::stoi(port_listen));
									remote_ptr->acceptor_ptr = std::make_shared<boost::asio::ip::tcp::acceptor>(*remote_ptr->context_ptr, ep);
									boost::cobalt::spawn(*(remote_ptr->context_ptr), co_ac(), boost::asio::detached);
								}
								catch (const std::exception& e)
								{
									ui::msgbox(gbk2utf8(e.what()));
								}
							}
							},ButtonOption::Animated()),
						Button(gettext("Stop Accept"),[this] {
							if (running) {
								remote_ptr->context_ptr->post([&] {remote_ptr->acceptor_ptr->cancel(); });
								ret = false;
							}
							else {
								ui::msgbox(gettext("Not Accepting"));
							}
							},ButtonOption::Animated()),
						}) | center,
					Renderer([this] {return text(fmt::format("{}:[{}]",gettext("State"),running ? gettext("Accepting") : (ret ? gettext("Connected") : gettext("Stopped")))) | center | border; })
					});

				auto tab = Container::Tab({ active_connect,wait_for_connect }, &selected_mode);
				auto container = Container::Vertical({
					toggle,
					tab,
					});
				auto layout = Container::Vertical({
					Renderer(container, [toggle, tab] {
					return vbox({
							   toggle->Render() | hcenter,
							   separator(),
							   tab->Render(),
						}) |
						border;
					}),
					Renderer([] {return text(gettext("WARNING: This mode has been deprecated!")) | ftxui::color(Color::Red); }) | center,
					Renderer([] {return ui::autolines(gettext(
						"The purpose of this mode is to provide users\n"
						"with a customized interface for the game.\n"
						"If you want to go online, please go to \n"
						"\"Home->Online\"\n"
						"For more information about this mode, please go to\n"
						"\"Home->More\"")
						, center | ftxui::color(Color::BlueLight)); }) | center
					});

				remote_ptr->thread_ptr = std::make_shared<std::jthread>([this] {remote_ptr->context_ptr->run(); });

				Add(layout | center | ui::EnableMessageBox());
			}
			Element Render() override {
				return children_[0]->Render();
			}
			bool OnEvent(Event e) override {
				return ChildAt(0)->OnEvent(e);
			}
			~Impl() {
				if (remote_ptr->acceptor_ptr) {
					remote_ptr->acceptor_ptr->close();
				}
				runningtim.cancel();
				if (!remote_ptr->context_ptr->stopped()) {
					remote_ptr->context_ptr->stop();
				}
				remote_ptr->thread_ptr->join();
				remote_ptr->context_ptr->restart();
				assert(!remote_ptr->context_ptr->stopped());
				if (remote_ptr->connected()) {
					remote_ptr->start();
				}
			}
		private:
			std::shared_ptr<othello::remote_tcp_gamer> remote_ptr;
			boost::asio::deadline_timer runningtim;
			std::string IP = "localhost", port = "22222";
			std::vector<std::string> vec = { gettext("Actively Connect"),gettext("Wait for Connection") };
			std::string port_listen = "22222", remote_port = gettext("None"), remote_IP = gettext("None");
			int selected_mode = 0;
			bool running = false;
			float prog = 0;
			bool ret = false;
		};
		return Make<Impl>(_p);
	}

	template<>
	ftxui::Component GamerSetting<othello::computer_gamer_ai>(basic_gamer_ptr _p) {
		using namespace ftxui;
		struct Impl :ftxui::ComponentBase {
			Impl(basic_gamer_ptr ptr){
				gptr = std::dynamic_pointer_cast<othello::computer_gamer_ai>(ptr);
				auto& option = gptr->e.option;
				s_mtd_select = std::to_underlying(option.s_mtd);
				e_mtd_select = std::to_underlying(option.e_mtd);
				device_select = std::to_underlying(option.device);
				threads_str = std::to_string(option.threads);
				srch_dp_str = std::to_string(option.search_depth);
				mcts_simulation_str = std::to_string(option.mcts_opt.simulations);
				time_limit_str = std::to_string(option.time_limit);
				expf_str = std::to_string(option.mcts_opt.explore_factor);

				auto default_comp = GamerSetting<void>(ptr);
				auto m_option = MenuOption();
				auto s_mtd_menu = Menu(&search_method_list, &s_mtd_select, m_option);
				auto e_mtd_menu = Menu(&eval_method_list, &e_mtd_select, m_option);
				auto device_menu = Menu(&device_list, &device_select, m_option);
				auto menus = Container::Horizontal({
					s_mtd_menu,
					e_mtd_menu,
					device_menu
				});
				auto menu_renderer = Renderer(menus, 
					[s_mtd_menu, e_mtd_menu, device_menu] {
						return 
						hbox({
							vbox({
								hcenter(bold(text(gettext("Search method")))),
								separator(),
								s_mtd_menu->Render(),
							}),
							separator(),
							vbox({
								hcenter(bold(text(gettext("Evaluate method")))),
								separator(),
								e_mtd_menu->Render(),
							}),
							separator(),
							vbox({
								hcenter(bold(text(gettext("Device")))),
								separator(),
								device_menu->Render(),
							}),
						}) | border;
					}
				);
				auto thd_input = Input(&threads_str, gettext("Enter threads number")) | ui::NumberOnly();
				auto threads_input = Renderer(thd_input, [thd_input] {
					return hbox(text(gettext("Thread number:")) | vcenter, thd_input->Render() | vcenter);
				});

				auto srch_dq_input = Input(&srch_dp_str, gettext("Enter search depth")) | ui::NumberOnly();
				auto search_depth_input = Renderer(srch_dq_input, [srch_dq_input] {
					return hbox(text(gettext("Search depth:")) | vcenter, srch_dq_input->Render() | vcenter);
				});
				auto alphabeta_opt = Container::Vertical({
					threads_input,
					search_depth_input,
					});

				auto simu_input = Input(&mcts_simulation_str, gettext("Enter simulate times")) | ui::NumberOnly();
				auto simulations_input = Renderer(simu_input, [simu_input] {
					return hbox(text(gettext("Simulation times:")) | vcenter, simu_input->Render() | vcenter);
					});

				auto tim_lim_input = Input(&time_limit_str, gettext("Enter microseconds")) | ui::NumberOnly();
				auto time_limit_input = Renderer(tim_lim_input, [tim_lim_input] {
					return hbox(text(gettext("Time limit(microseconds):")) | vcenter, tim_lim_input->Render() | vcenter);
					});

				auto expf_input = Input(&expf_str, gettext("Enter explore factor")) | ui::FloatOnly();
				auto explore_factor_input = Renderer(expf_input, [expf_input] {
					return hbox(text(gettext("explore factor:")) | vcenter, expf_input->Render() | vcenter);
					});
				auto mcts_opt = Container::Vertical({
					simulations_input,
					time_limit_input,
					explore_factor_input
					});

				auto menu_tabs = Container::Tab({
					alphabeta_opt,
					mcts_opt,
					},&s_mtd_select);
				auto algo_detailed_opt = Renderer(menu_tabs, [menu_tabs]{
					return vbox(
						text(gettext("Algorithm Advanced Option")),
						separator(),
						menu_tabs->Render()
					) | borderRounded;
					});
				auto layout = Container::Vertical({
					default_comp | hcenter,
					menu_renderer | hcenter,
					algo_detailed_opt| hcenter,
					});
				Add(layout);
			}
			~Impl() {
				auto& option = gptr->e.option;
				option.s_mtd = othello::ai::ai_option::search_method(s_mtd_select);
				option.e_mtd = othello::ai::ai_option::eval_method(e_mtd_select);
				option.device = othello::ai::ai_option::device_type(device_select);
				option.threads = threads_str.empty() ? 1 : std::stoi(threads_str);
				option.search_depth = srch_dp_str.empty() ? 5 : std::stoi(srch_dp_str);
				option.mcts_opt.simulations = mcts_simulation_str.empty() ? 0 : std::stoi(mcts_simulation_str);
				option.time_limit = time_limit_str.empty() ? 0 : std::stoi(time_limit_str);
				option.mcts_opt.explore_factor = std::stof(expf_str);
				gptr->e.set_algo();
			}
		private:
			std::shared_ptr<othello::computer_gamer_ai> gptr = nullptr;
			std::string threads_str;
			std::string srch_dp_str;
			std::string mcts_simulation_str;
			std::string time_limit_str;
			std::string expf_str;
			int s_mtd_select;
			std::vector<string> search_method_list = {
				"alphabeta",
				"mcts",
				"minmax"
			};
			int e_mtd_select;
			std::vector<string> eval_method_list = {
				"traits",
				"pattern",
				"nn"
			};
			int device_select;
			std::vector<string> device_list = {
				"CPU",
				"GPU"
			};
		};
		return ftxui::Make<Impl>(_p);
	}

	template<>
	ftxui::Component GamerSetting<othello::http_gamer>(basic_gamer_ptr _p) {
		using namespace ftxui;
		struct Impl :ftxui::ComponentBase {
		public:
			inline std::string gbk2utf8(std::string_view s) {
				return boost::locale::conv::to_utf<char>(s.data(), "gbk");
			}
			Impl(basic_gamer_ptr p) :
				http_ptr(std::dynamic_pointer_cast<othello::http_gamer>(p))
			{
				auto test_connect = Container::Vertical({
					Container::Horizontal({
					Renderer([] {
						return vbox(text(gettext("Server Name:")) | border ,text(gettext("Address:")) | border,text(gettext("Port:")) | border);
						}),
					Container::Vertical({
						Renderer([this] {
							return text(http_ptr->get_name().empty()
								? gettext("Anonymous")
								: http_ptr->get_name())
								| underlined | border;
							}),
						Input(&IP, "localhost") | size(WIDTH,GREATER_THAN,16) | border,
						Input(&port, "8888") | size(WIDTH,GREATER_THAN,16) | border
						}),
					}) | center,
					Container::Horizontal({
					Button(gettext("Test Server"),[this] {
						try {
							if (http_ptr->connected()) {
								http_ptr->close();
							}
							http_ptr->set_server(IP, std::stoi(port));
							http_ptr->sync_test_server();
							http_ptr->refresh_name();
							ret = true;
							http_ptr->is_good = true;
						}
						catch (const std::exception& e) {
							ui::msgbox(boost::locale::conv::to_utf<char>(e.what(),"gbk"));
							ret = false;
							http_ptr->is_good = false;
						}
						},ButtonOption::Animated())
					}) | center,
					Renderer([this] {return text(std::format("{}:[{}]",gettext("State"),ret ? gettext("Good") : gettext("Bad"))) | center | border; })
					});
				Add(test_connect | center | ui::EnableMessageBox());
			}
			Element Render() override {
				return children_[0]->Render();
			}
			bool OnEvent(Event e) override {
				return ChildAt(0)->OnEvent(e);
			}
		private:
			std::shared_ptr<othello::http_gamer> http_ptr;
			std::string IP = "localhost", port = "22222";
			bool ret = false;
		};
		return Make<Impl>(_p);
	}
}
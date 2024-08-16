#pragma once
#include"tui/game/gamer_prepare.hpp"
#include"othello/gamer/computer.hpp"
#include"othello/gamer/remote_tcp.hpp"
#include"othello/gamer/lua_gamer.hpp"
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
			fin.open("othello.lua", std::ios::in);
			if (fin.is_open()) {
				gptr->script.clear();
				fin >> gptr->script;
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
				gptr->script.clear();
				fin >> gptr->script;
				fin.close();
			},ButtonOption::Animated()) | hcenter,
			ui::TextComp(gettext("File name\n\"othello.lua\""),hcenter) | border | hcenter
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
}
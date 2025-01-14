#pragma once
#include"globals.hpp"
#include"components.hpp"
#include"utility.hpp"
#include"move_server/move_server.hpp"
namespace bw::components {
	struct MoveServerPageImpl {
		ftxui::Component Page() {
			using namespace ftxui;
			Component layout = Container::Vertical({
				Container::Horizontal({
					ui::TextComp(gettext("Port:")) | underlined | vcenter,
					Input(&port,InputOption::Spacious()) | vcenter | ui::NumberOnly() | ui::ShortThan(&port,5)
					}),
				Container::Horizontal({
					ui::TextComp(gettext("Thread Number:")) | underlined | vcenter,
					Input(&thd_num,InputOption::Spacious()) | vcenter | ui::NumberOnly()
					}),
				Button(gettext("Start Server"),[this] {
					ScreenInteractive::Active()->Post(
						ftxui::ScreenInteractive::Active()->WithRestoredIO([this] {
							try {
								std::unique_ptr<move_server::move_server> server_ptr = nullptr;
								server_ptr = std::make_unique<move_server::move_server>(std::stoi(thd_num),std::stoi(port));
								server_ptr->sync_start();
							}
							catch (const std::exception& e) {
								spdlog::error(e.what());
							}
						})
					); 
					},
					ButtonOption::Animated()
				)
			});
			return layout;
		}
		std::string port = "8888";
		std::string thd_num = "1";
	};
	inline ftxui::Component MoveServerComp() {
		struct Impl :ftxui::ComponentBase {
			Impl() {
				Add(impl.Page());
			}
			MoveServerPageImpl impl;
		};
		return ftxui::Make<Impl>();
	}
}
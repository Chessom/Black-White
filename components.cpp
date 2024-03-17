#include"stdafx.h"
#include"components.h"
namespace bw::ui {
	using namespace core;
	void msgbox(std::string_view s) {
		MessageBoxString = s;
		show_modal = true;
		MessageBoxComp->TakeFocus();
	}
	void premsgbox()
	{
		show_modal = true;
		MessageBoxComp->TakeFocus();
	}
	ftxui::Component MessageBoxComponent() {
		using namespace ftxui;
		auto component = Button("Quit", [] {show_modal = false; }, ButtonOption::Animated());
		component |= Renderer([&](Element inner) {
			return vbox({
				text(""),
				separator(),
				text(MessageBoxString) | center,
				inner | center,
				})
				| borderRounded | center |clear_under;
			});
		return Container::Vertical({ component });
	}
	ftxui::ComponentDecorator EnableMessageBox() {
		using namespace ftxui;
		return Modal(MessageBoxComp = MessageBoxComponent(), &show_modal);
	}
	ftxui::Component TextComp(std::string_view str) {
		using namespace ftxui;
		return Renderer([&str]() {return ftxui::text(str.data()); });
	}
}
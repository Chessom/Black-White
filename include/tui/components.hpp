#pragma once
#include"stdafx.h"
#include"core.hpp"
#include"globals.hpp"
#include"bwlocale.hpp"
namespace bw::ui {
	using namespace core;
	struct auto_close_modal {
		auto_close_modal() { _show_modal = false; }
		~auto_close_modal() { _show_modal = false; }
	};
	inline void msgbox(std::string s) {
		MessageBoxString = s;
		_show_modal = true;
		MessageBoxComp->TakeFocus();
	}
	inline void msgbox(std::string s, std::function<void()> call_back) {
		MessageBoxString = s;
		_show_modal = true;
		_msgbox_call_back = std::move(call_back);
		MessageBoxComp->TakeFocus();
	}
	inline void msgbox(std::string s, ftxui::Components buts) {
		using namespace ftxui;
		MessageBoxString = s;
		MessageBoxComp->DetachAllChildren();
		Component box = Container::Horizontal(buts) | Renderer([](Element inner) {
			return vbox({
				text(""),
				separator(),
				text(MessageBoxString) | center,
				inner | center,
				})
				| borderRounded | center | clear_under;
			});
		MessageBoxComp->Add(box);
		_show_modal = true;
		MessageBoxComp->TakeFocus();
	}
	inline void msgbox(ftxui::Component box) {
		MessageBoxComp->DetachAllChildren();
		MessageBoxComp->Add(box);
		_show_modal = true;
		MessageBoxComp->TakeFocus();
	}
	inline void premsgbox()
	{
		_show_modal = true;
		MessageBoxComp->TakeFocus();
	}
	inline ftxui::Component MessageBoxComponent() {
		using namespace ftxui;
		auto component = Button(gettext("OK"), [] {_show_modal = false; _msgbox_call_back(); }, ButtonOption::Animated());
		component |= Renderer([](Element inner) {
			return vbox({
				text(""),
				separator(),
				text(MessageBoxString) | center,
				inner | center,
				})
				| borderRounded | center | clear_under;
			});
		return Container::Vertical({ component });
	}
	inline ftxui::Component StackedMsgBoxComp() {
		using namespace ftxui;
		auto component = Container::Vertical({
			Renderer([] {return text(MessageBoxString) | center; }),
			Button(std::format("{:^8}",gettext("OK")), [] {_show_modal = false; }, ButtonOption::Animated()) | center
			});
		auto Dim = Terminal::Size();
		auto window = Window({
			.inner = component,
			.title = gettext("Tip"),
			.left = Dim.dimx / 2 - 20,
			.top = Dim.dimy / 2 - 5,
			.width = 40,
			.height = 10,
			.resize_left = true,
			.resize_right = true,
			.resize_top = false,
			.resize_down = false,
			});
		return Modal(Renderer([] {return emptyElement(); }), window, &_show_modal);
	}
	inline ftxui::ComponentDecorator EnableMessageBox() {
		using namespace ftxui;
		return Modal(MessageBoxComp = MessageBoxComponent(), &_show_modal);
		/*return [](Component comp) {
			return Container::Stacked({ MessageBoxComp = StackedMsgBoxComp(),std::move(comp) });
		};*/
	}
	inline ftxui::Component TextComp(std::string str) {
		using namespace ftxui;
		return Renderer([str]() {return ftxui::text(str.data()); });
	}
	inline ftxui::Element autolines(const std::string& str, ftxui::ElementDecorator style = ftxui::nothing) {
		using namespace ftxui;
		Elements es;
		int pre = 0;
		for (int i = 0; i < str.size(); ++i) {
			if (str[i] == '\n') {
				es.push_back(paragraphAlignJustify(str.substr(pre, i - pre)) | style);
				pre = i + 1;
			}
		}
		if (pre != str.size()) {
			es.push_back(paragraphAlignJustify(str.substr(pre, str.size() - pre)) | style);
		}
		return vbox(es);
	}
	inline ftxui::Component Focusable() {
		struct Impl : public ftxui::ComponentBase {
			bool Focusable() const override { return true; }
		};
		return ftxui::Make<Impl>();
	}
}
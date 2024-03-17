#pragma once
#include"stdafx.h"
#include"core.hpp"
#include"globals.hpp"
#include"bwlocale.hpp"
namespace bw::ui {
	using namespace core;
	struct auto_close_modal {
		auto_close_modal() { show_modal = false; }
		~auto_close_modal() { show_modal = false; }
	};
	extern void msgbox(std::string_view s);
	extern void premsgbox();
	extern ftxui::Component MessageBoxComponent();
	extern ftxui::ComponentDecorator EnableMessageBox();
	extern ftxui::Component TextComp(std::string_view);
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
	class Scrollable :ftxui::ComponentBase {
	public:
		Scrollable(ftxui::Components& Components) {
			
		}
		
	};
}
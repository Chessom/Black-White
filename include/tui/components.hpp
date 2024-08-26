#pragma once
#include"stdafx.h"
#include"core.hpp"
#include"globals.hpp"
#include"bwlocale.hpp"
namespace bw::ui {
	using namespace core;
	struct auto_close_modal {
		auto_close_modal() { 
			_show_modal = false; 
			MessageBoxComp->DetachAllChildren();
		}
		~auto_close_modal() { 
			_show_modal = false;
			MessageBoxComp->DetachAllChildren();
		}
	};
	inline ftxui::Component ToCom(ftxui::Element e) {
		return ftxui::Renderer([e] {return e; });
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
	inline ftxui::Component TextComp(std::string str, ftxui::ElementDecorator style = ftxui::nothing) {
		using namespace ftxui;
		return Renderer([str, style]() {return autolines(str.data(), style); });
	}
	inline ftxui::Component MakeOKButton() {
		return ftxui::Button(gettext("OK"), [] {_show_modal = false; _msgbox_call_back(); }, ftxui::ButtonOption::Animated());
	}
	inline ftxui::Component MessageBoxComponent() {
		using namespace ftxui;
		auto component = MakeOKButton();
		component |= Renderer([](Element inner) {
			return vbox({
				text(""),
				separator(),
				autolines(MessageBoxString,center) | center,
				inner | center,
				}) | borderRounded | center ;
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
		return Maybe(window, &_show_modal);
	}
	inline ftxui::ComponentDecorator EnableMessageBox() {
		using namespace ftxui;
		MessageBoxComp = MessageBoxComponent();
		return Modal(MessageBoxComp | bgcolor(Color::Black), &_show_modal);
	}
	inline ftxui::ComponentDecorator EnableStackedMessageBox() {
		using namespace ftxui;
		return [](Component comp) {
			return Container::Stacked({ MessageBoxComp = StackedMsgBoxComp(),std::move(comp) });
		};
	}
	inline ftxui::Component Focusable() {
		struct Impl : public ftxui::ComponentBase {
			bool Focusable() const override { return true; }
		};
		return ftxui::Make<Impl>();
	}
	inline void msgbox(std::string s) {
		MessageBoxComp->DetachAllChildren();
		MessageBoxComp->Add(MessageBoxComponent());
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
				autolines(MessageBoxString,center) | center,
				inner | center,
				})
				| borderRounded | center;
			});
		MessageBoxComp->Add(box);
		_show_modal = true;
		MessageBoxComp->TakeFocus();
	}
	inline void msgbox(ftxui::Component box) {
		using namespace ftxui;
		MessageBoxComp->DetachAllChildren();
		MessageBoxComp->Add(box);
		_show_modal = true;
		box->TakeFocus();
	}
	inline void premsgbox()
	{
		_show_modal = true;
		MessageBoxComp->TakeFocus();
	}
	class NodeDecorator : public ftxui::Node {
	public:
		explicit NodeDecorator(ftxui::Element child) : Node(unpack(std::move(child))) {}
		void ComputeRequirement() override {
			Node::ComputeRequirement();
			requirement_ = children_[0]->requirement();
		}
		void SetBox(ftxui::Box box) override {
			Node::SetBox(box);
			children_[0]->SetBox(box);
		}
	};
	class ResizeDecorator : public NodeDecorator {
	public:
		ResizeDecorator(ftxui::Element child,
			bool resize_left,
			bool resize_right,
			bool resize_top,
			bool resize_down,
			ftxui::Color color)
			: NodeDecorator(std::move(child)),
			color_(color),
			resize_left_(resize_left),
			resize_right_(resize_right),
			resize_top_(resize_top),
			resize_down_(resize_down) {}

		void Render(ftxui::Screen& screen) override {
			NodeDecorator::Render(screen);

			if (resize_left_) {
				for (int y = box_.y_min; y <= box_.y_max; ++y) {
					auto& cell = screen.PixelAt(box_.x_min, y);
					cell.foreground_color = color_;
					cell.automerge = false;
				}
			}
			if (resize_right_) {
				for (int y = box_.y_min; y <= box_.y_max; ++y) {
					auto& cell = screen.PixelAt(box_.x_max, y);
					cell.foreground_color = color_;
					cell.automerge = false;
				}
			}
			if (resize_top_) {
				for (int x = box_.x_min; x <= box_.x_max; ++x) {
					auto& cell = screen.PixelAt(x, box_.y_min);
					cell.foreground_color = color_;
					cell.automerge = false;
				}
			}
			if (resize_down_) {
				for (int x = box_.x_min; x <= box_.x_max; ++x) {
					auto& cell = screen.PixelAt(x, box_.y_max);
					cell.foreground_color = color_;
					cell.automerge = false;
				}
			}
		}

		ftxui::Color color_;
		const bool resize_left_;
		const bool resize_right_;
		const bool resize_top_;
		const bool resize_down_;
	};
	inline ftxui::Element AlwaysActiveRenderState(const ftxui::WindowRenderState& state) {
		using namespace ftxui;
		ftxui::Element element = state.inner;

		element = window(ftxui::text(state.title), element);
		element |= ftxui::clear_under;

		const Color color = Color::Red;

		element = std::make_shared<ResizeDecorator>(  //
			element,                                  //
			state.hover_left,                         //
			state.hover_right,                        //
			state.hover_top,                          //
			state.hover_down,                         //
			color                                     //
		);

		return element;
	}
}
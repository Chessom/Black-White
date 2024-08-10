#pragma once
#include"tui/screen.hpp"
#include"bwlocale.hpp"
namespace bw::components {
	class ftxui_screen :public screen {
	public:
		ftxui_screen(ftxui::ScreenInteractive* Screen) :_initial(Screen) {}
		virtual bool good() override {
			return _initial == current();
		}
		virtual void post_event(const std::string& s) override {
			if (good())
				_initial->PostEvent(ftxui::Event::Special(s));
			else
				throw std::runtime_error("Invalid screen!");
		}
		virtual void post(std::function<void()> task) override {
			if (good())
				_initial->Post(task);
			else
				throw std::runtime_error("Invalid screen!");
		}
		virtual void refresh() override {
			if (good())
				_initial->PostEvent(ftxui::Event::Custom);
			else
				throw std::runtime_error("Invalid screen!");
		}
		virtual void exit() override {
			if (good())
				_initial->Exit();
		}
		static ftxui::ScreenInteractive* current() {
			return ftxui::ScreenInteractive::Active();
		}
	private:
		ftxui::ScreenInteractive* _initial = nullptr;
		
	};
	using ftxui_screen_ptr = std::shared_ptr<ftxui_screen>;
}
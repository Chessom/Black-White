#pragma once
#include"stdafx.h"
namespace bw::components {
	class screen {
	public:
		virtual bool good() = 0;
		virtual void post_event(const std::string&) = 0;
		virtual void post(std::function<void()>) = 0;
		virtual void refresh() = 0;
		virtual void exit() = 0;
	};

	using screen_ptr = std::shared_ptr<screen>;

	struct empty_screen :screen {
		virtual bool good() { return true; }
		virtual void post_event(const std::string&) {};
		virtual void post(std::function<void()>) {};
		virtual void refresh() {};
		virtual void exit() {};
	};
}
#pragma once
#include"stdafx.h"
#include"core.hpp"
#include<ftxui/dom/elements.hpp>
#include<ftxui/component/component.hpp>
#include<ftxui/component/screen_interactive.hpp>
namespace bw::ui {
	using namespace core;
	using ftxui::text;
	using ftxui::Event;
	using ftxui::Mouse;
	/*class BoardBase : public ftxui::ComponentBase {
	public:
		BoardBase() = default;
		virtual ftxui::Element Render() { return text("Blank board"); }
		virtual bool OnEvent(Event e) {	return false;}
		bool OnMouseEvent(Event e) {}
	protected:
		bool mouse_hover = false;
		ftxui::Box box;
	};*/
}
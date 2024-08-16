#pragma once
#include"gamer.hpp"
#include"tui/components.hpp"
namespace bw::components {
	inline ftxui::Component HumanGamerSetting(basic_gamer_ptr ptr) {
		using namespace ftxui;
		auto gamer_type = Renderer(
			[] {
				return hbox(text(gettext("Gamer Type:")) | borderRounded, text(gettext("Human")) | borderRounded);
			}
		);
		auto gamer_name = Container::Horizontal({
			ui::TextComp(gettext("Gamer name:")) | borderRounded | center ,
			Input(&ptr->name, gettext("Enter name"), InputOption::Spacious()) | borderRounded | center
		});
		return Container::Vertical({
			gamer_type | center,
			gamer_name | center
		});
	}
	template<typename Gamer>
	ftxui::Component GamerSetting(basic_gamer_ptr ptr) {
		using namespace ftxui;
		auto gamer_name = Container::Horizontal({
			ui::TextComp(gettext("Gamer name:")) | borderRounded | center,
			Input(&ptr->name, gettext("Enter name"), InputOption::Spacious()) | borderRounded | center
			});
		return gamer_name | center;
	}
}
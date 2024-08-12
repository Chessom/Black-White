#pragma once
#include"config.hpp"
#include"globals.hpp"
#include"components.hpp"
namespace bw::components {
	struct SettingsPage {
		SettingsPage() {
			conf = *bw::global_config;
		}
		
		void start() {
			using namespace ftxui;
			auto screen = ScreenInteractive::Fullscreen();
            std::vector<std::string> language_entries = {
                gettext("Chinese"),
                gettext("English"),
            };

            int language_selected = 0;

            Component language_toggle = Toggle(&language_entries, &language_selected);

            Component name_input = Input(&conf.default_name, gettext("Enter the name")) | underlined;

            Component address_input = Input(&conf.default_address, gettext("Enter the address")) | underlined;

            std::string port_string = std::to_string(conf.port);
            Component port_number_input = Input(&port_string, gettext("Enter the port number")) | underlined | CatchEvent([&](Event event) {return event.is_character() && !std::isdigit(event.character()[0]);}) | CatchEvent([&](Event event) {return event.is_character() && port_string.size() > 5;});

			Component locale_path_input = Input(&conf.locale_path, gettext("Path of the language files")) | underlined;


            auto save_handler = [this, &port_string, &language_selected] {
                if (language_selected == 0) {
                    conf.locale_id = "zh_CN";
                }
                else if (language_selected == 1) {
                    conf.locale_id = "en_US";
                }
                conf.port = std::stoi(port_string);
				/*std::locale::global(locale_gen(conf.locale_id + ".utf8"));
				std::cout.imbue(std::locale());*/
                };
			auto ConfirmButton = Button(gettext("Temporary Save"),
                save_handler,
                ButtonOption::Animated(Color::GreenLight)
            );

			auto SaveButton = Button(gettext("Save to local"),
				[this, save_handler] {
					save_handler();
					if (global_config->config_t == config::type::binary) {
						conf.dump_binary_file(config::bin_config_path);
					}
					else
					{
						std::string config_json;
						struct_json::to_json(conf, config_json);
						std::ofstream fout(config::json_config_path, std::ios::out);
						fout << config_json;
						fout.close();
					}
				},
				ButtonOption::Animated(Color::Blue)
			);
			auto CancelButton = Button(gettext("Cancel"),
				[this, &screen, &port_string, &language_selected ] {
					conf = *global_config;
					port_string = std::to_string(conf.port);
					if (conf.locale_id == "zh_CN") {
						language_selected = 0;
					}
					else if (conf.locale_id == "en_US") {
						language_selected = 1;
					}
				},
				ButtonOption::Animated(Color::YellowLight)
			);
			auto ExitButton = Button(gettext("Quit"), screen.ExitLoopClosure(), ButtonOption::Animated(Color::RedLight));
            
            auto container = Container::Vertical({
                language_toggle,
                name_input,
                address_input,
                port_number_input,
				locale_path_input,
                });

			auto renderer = Renderer(container, [&] {
				return vbox({
					hbox(text("· " + gettext("Language (Changes take effect after restart): ")), filler(),language_toggle->Render()) | xflex ,
					hbox(text("· " + gettext("Default display name: ")),filler(),name_input->Render() | align_right) | xflex,
					hbox(text("· " + gettext("Default server address: ")),filler(),address_input->Render() | align_right) | xflex,
					hbox(text("· " + gettext("Default port number: ")),filler(),port_number_input->Render() | align_right) | xflex,
					hbox(text("· " + gettext("Locale files path: ")),filler(),locale_path_input->Render() | align_right) | xflex,
					});
				}) | xflex | size(WIDTH, LESS_THAN, 80) | border;
			
			auto layout = Container::Vertical({
				renderer | center,
				Renderer([] {return filler(); }),
				Container::Horizontal({
					ConfirmButton | vcenter ,
					ui::TextComp("  "),
					SaveButton | vcenter ,
					ui::TextComp("  "),
					CancelButton | vcenter,
					ui::TextComp("  "),
					ExitButton | vcenter
					}) | center,
				ui::TextComp(gettext(
					"\"Temporary save\" means that changes will only \n"
					"take effect until exiting the program, \n"
					"while \"Save to local\" means that \n"
					"changes will be written to the configuration file \n"
					"and will take effect at a later startup."
				),center | color(Color::BlueLight)) | center
			});

            screen.Loop(layout);
		}
		bw::config conf;
		~SettingsPage() {
			*bw::global_config = conf;
		}
	};
}
#pragma once
#include"stdafx.h"
#include"core.hpp"
#include"basic_gamer.hpp"
#include"game.hpp"
#include"online/basic_user.hpp"
namespace bw {
	class basic_Game {
	public:
		using context_ptr = std::shared_ptr<boost::asio::io_context>;

		basic_Game() { pctx = std::make_shared<boost::asio::io_context>(); }
		basic_Game(context_ptr context) :pctx(std::move(context)) {};

		inline std::string censtr(const std::string& str, int width) {
			return std::format("{0:^{1}}", str, width);
		}
		inline std::string gbk2utf8(std::string_view s) {
			return boost::locale::conv::to_utf<char>(s.data(), "gbk");
		}

		virtual ftxui::Component OnlineGamePage(ftxui::ScreenInteractive& screen, basic_game_ptr gm_ptr) { return nullptr; };
		virtual ftxui::Component GamePage(ftxui::ScreenInteractive& screen) = 0;
		virtual ftxui::Component OnlinePrepareCom(bw::online::basic_user_ptr) { return nullptr; };
		virtual void set_board_size(int size) = 0;
		virtual basic_gamer_ptr gamer_from_info(basic_gamer_info) = 0;
		virtual basic_game_ptr generate_game(ftxui::ScreenInteractive&) = 0;
		virtual void join(basic_gamer_ptr) = 0;
		virtual ~basic_Game() {
			spdlog::trace("Basic Game Destructor");
		}
		std::shared_ptr<boost::asio::io_context> pctx;
		bool another_round = false;
	};
}
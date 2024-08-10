#pragma once
#include"stdafx.h"
#include"core.hpp"
#include"gamer.hpp"
#include"game.hpp"
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

		//virtual ftxui::Component GamePreparePage(ftxui::ScreenInteractive& screen, basic_game_ptr gm_ptr) = 0;
		virtual ftxui::Component GamePage(ftxui::ScreenInteractive& screen, basic_game_ptr gm_ptr) = 0;
		virtual ftxui::Component OnlinePrepareCom(std::function<void()> match_op) {
			return nullptr;
		};
		virtual void set_board_size(int size) = 0;
		virtual basic_gamer_ptr gamer_from_info(basic_gamer_info) = 0;
		virtual basic_game_ptr generate_game(ftxui::ScreenInteractive&) = 0;
		virtual void join(basic_gamer_ptr) = 0;
		virtual ~basic_Game() = default;
		std::shared_ptr<boost::asio::io_context> pctx;
	};
	using basic_Game_ptr = std::shared_ptr<basic_Game>;
}
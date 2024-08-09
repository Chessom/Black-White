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
		virtual ftxui::Component OnlinePrepareCom(std::function<void()> match_op) { return {}; };
		virtual ~basic_Game() = default;
		virtual void join(basic_gamer_ptr) = 0;//
		std::shared_ptr<boost::asio::io_context> pctx;
	};
	using basic_Game_ptr = std::shared_ptr<basic_Game>;
}
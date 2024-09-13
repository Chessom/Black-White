#pragma once
#include"stdafx.h"
namespace bw {
	struct bwtimer {
		using clock_type = std::chrono::high_resolution_clock;
		bwtimer() {
			prev = clock.now();
		}
		std::chrono::nanoseconds elasped() {
			auto now = clock.now();
			auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now - prev);
			prev = now;
			return ns;
		}
		void restart() {
			prev = clock.now();
		}
	private:
		clock_type::time_point prev;
		clock_type clock;
	};
}
#pragma once
#include"stdafx.h"
#include"core.hpp"
#include"env.hpp"
#include"utility.hpp"
#include"othello/board.hpp"
#include"othello/moves.hpp"
#define BSIZE 8
#define inc(i,n) for(int i=0;i<n;i++)
namespace std {
    ostream& operator<<(ostream& os, const char8_t* str) {
        os << reinterpret_cast<const char*>(str);
        return os;
    }
    ostream& operator<<(ostream& os, u8string_view strvw) {
        os << reinterpret_cast<const char*>(strvw.data());
        return os;
    }
    ostream& operator<<(ostream& os, const u8string& str) {
        os << reinterpret_cast<const char*>(str.c_str());
        return os;
    }
}
namespace bw::tools {
    using namespace std;
    using namespace core;
    using brd = bitset<64>;
    inline bool in_board(const coord& crd) {
        return crd.x < 8 && crd.y < 8 && crd.x >= 0 && crd.y >= 0;
    }
    /*void gen_crosses(){
        ofstream fout("C:\\Users\\vemy\\desktop\\data.txt");
        ull brd;
        coord iter;
        using namespace directions;
        fout << std::hex << "{";
        for (int x = 0; x < 8; ++x) {
            for (int y = 0; y < 8; ++y) {
                iter = { x,y };
                brd = 0ull;
                brd |= iter.to_bititer();
                for (int drc = R; drc <= UR; ++drc) {
                    iter = { x,y };
                    while (in_board(iter.to_next(drc))) {
                        brd |= iter.to_bititer();
                    }
                }
                fout << "0x" << brd << ",";
            }
            fout << "\n";
        }
        fout << "}";
    }
    void test_crosses() {
        int x = 0, y = 0;
        while (x != -1) {
            cin >> x >> y;
            println("{:sm<}", bitbrd(crosses[x][y], 0ull));
        }
    }*/
    coord fromstr(const string& s) {
        return coord(s[1] - '1', s[0] - 'A');
    }
    bool is_vaild_movestr(const string& s) {
        return s[0] >= 'A' && s[0] <= 'Z' && s[1] >= '0' && s[1] <= '9';
    }
    /*void test_applymove() {
        ull bm, wm;
        cin >> std::hex >> bm >> wm;
        bitbrd brd(bm, wm);
        arrbrd abrd(brd);
        while (true) {
            moves mvs(brd, col0);
            print("{:sm<}\n{:sm<}\n{:c}\n", brd, abrd, mvs);
            string s;
            cin >> s;
            brd.applymove(fromstr(s), col0);
            abrd.applymove(fromstr(s), col0);
        }
    }*/
    void cls() {
        std::system("cls");
    }
	void print_empty_brd() {
		std::cout << "╔"; 
        for (int i = 1; i < BSIZE; i++) 
            std::cout << "═══╦";
		std::cout << "═══╗\n";
        for (int i = 0; i<8; i++) {
			std::cout << "║";
            for (int j = 0; j<8; j++) {
				/*if (game[pos(i, j)] == !game.col) std::cout<<"○";
				else if (game[pos(i, j)] == game.col) std::cout<<"●";
				else */
				std::cout << " ○";
				std::cout << "║";
			}
			std::cout << "\n";
            if (i < BSIZE - 1) {
                std::cout << "╠"; 
                for (int j = 0; j < BSIZE - 1; j++) 
                    std::cout << "═══╬";
                std::cout << "═══╣";
            }
            else {
                std::cout << "╚"; 
                for (int j = 0; j < BSIZE - 1; j++) 
                    std::cout << "═══╩";
                std::cout << "═══╝";
            }
			std::cout << "\n";
		}
	}
}
#undef BSIZE
#undef inc
#pragma once
#include"stdafx.h"
#define NAME(variable) (#variable)
namespace bw {
	template<std::integral Int>
	inline constexpr bool is_odd(const Int& num) { return num & Int(1); }
	inline consteval bool is_pow2(int x) {
		return ((x - 1) & x) == 0;
	}
	inline consteval int log2(int x) {
		if (x == 1)return 0;
		if (x == 2)return 1;
		if (x == 4)return 2;
		if (x == 8)return 3;
		if (x == 16)return 4;
		if (x == 32)return 5;
		if (x == 64)return 6;
		if (x == 128)return 7;
		if (x == 256)return 8;
	}
	inline consteval unsigned long long inner(int Size) {
		if (Size == 4)return 0x6666ull;
		if (Size == 6)return 0x79E79E79Eull;
		if (Size == 8)return 0x7E7E7E7E7E7E7E7Eull;
	}
    inline std::string xml_format(const std::string& input, const std::string& levelstr = "    ") {
        auto getLevelStr = [levelstr](int level) {
            std::string levelStr = "";
            for (int i = 0; i < level; i++)
            {
                levelStr += levelstr;
            }
            return levelStr;
            };

        std::string result;

        enum OpenStatus { Open, Close };
        OpenStatus beforeIsOpen = Open;
        OpenStatus afterIsOpen = Open;

        int openCnt = 0;
        int closeCnt = 0;
        int level = 0;

        for (std::string::size_type index = 0; index < input.size(); index++)
        {
            char c = input[index];

            if (level > 0 && '\r\n' == input[input.size() - 1])
            {
                result += getLevelStr(level);
            }

            switch (c)
            {
            case '<':
                // xml头时
                if (input[index + 1] == '?') {
                    result = result + c;
                    continue;
                }

                // 开<a>
                if (input[index + 1] != '/') {
                    openCnt++;
                    afterIsOpen = Open;
                }
                else {
                    // 关 </a>
                    afterIsOpen = Close;
                }

                // 前开，后关不换行，无缩进
                if (beforeIsOpen == Open && afterIsOpen == Close) {

                }
                else {
                    // 去除后面空格
                    result.erase(result.find_last_not_of(" ") + 1);

                    // 第一行不缩进
                    if (index > 0) {
                        // 换行
                        result += "\n";

                        level = openCnt - closeCnt - 1;
                        result += getLevelStr(level);
                    }
                }

                result = result + c;

                // 保留状态放在最下面
                if (input[index + 1] != '/') {

                }
                else {
                    // 关 </a>
                    closeCnt++;
                }
                beforeIsOpen = afterIsOpen;
                break;
            case '>':
                // xml头时
                if (input[index - 1] == '?') {
                    result = result + c;
                    continue;
                }
                // 关<a/>
                if (input[index - 1] == '/') {
                    closeCnt++;
                    afterIsOpen = Close;
                    beforeIsOpen = afterIsOpen;
                }

                result = result + c;
                break;
            default:
                // 回车键
                if (c == '\r' || c == '\n') {
                }
                else {
                    result += c;
                }
                break;
            }

        }

        return result;
    }
    inline std::string json_format(const std::string& json, const std::string& levelstr = "    ")
    {
        auto getLevelStr = [str = levelstr](int level) {
            std::string levelStr;
            for (int i = 0; i < level; i++)
            {
                levelStr += str;
            }
            return levelStr;
        };
        std::string result;
        int level = 0;
        for (std::string::size_type index = 0; index < json.size(); index++)
        {
            char c = json[index];

            if (level > 0 && '\n' == json[json.size() - 1])
            {
                result += getLevelStr(level);
            }

            switch (c)
            {
            case '{':
            case '[':
                result = result + c + "\n";
                level++;
                result += getLevelStr(level);
                break;
            case ',':
                result = result + c + "\n";
                result += getLevelStr(level);
                break;
            case '}':
            case ']':
                result += "\n";
                level--;
                result += getLevelStr(level);
                result += c;
                break;
            case ':':
                result += c;
                result += " ";
                break;
            default:
                result += c;
                break;
            }
        }
        return result;
    }
}

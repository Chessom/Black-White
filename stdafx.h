#pragma once
#include<cereal/access.hpp>
#include<cereal/archives/adapters.hpp>
#include<cereal/archives/binary.hpp>
#include<cereal/archives/portable_binary.hpp>
#include<cereal/archives/json.hpp>
#include<cereal/archives/xml.hpp>
#include<cereal/types/array.hpp>
#include<cereal/types/atomic.hpp>
#include<cereal/types/base_class.hpp>
#include<cereal/types/bitset.hpp>
#include<cereal/types/chrono.hpp>
#include<cereal/types/common.hpp>
#include<cereal/types/complex.hpp>
#include<cereal/types/concepts/pair_associative_container.hpp>
#include<cereal/types/deque.hpp>
#include<cereal/types/forward_list.hpp>
#include<cereal/types/functional.hpp>
#include<cereal/types/list.hpp>
#include<cereal/types/stack.hpp>
#include<cereal/types/string.hpp>
#include<cereal/types/tuple.hpp>
#include<cereal/types/unordered_map.hpp>
#include<cereal/types/unordered_set.hpp>
#include<cereal/types/utility.hpp>
#include<cereal/types/variant.hpp>
#include<cereal/types/vector.hpp>
#include<cereal/version.hpp>

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fenv.h>
#include <float.h>
#include <inttypes.h>
#include <limits.h>
#include <locale.h>
#include <math.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <uchar.h>
#include <wchar.h>
#include <wctype.h>


#pragma warning(push)
#pragma warning(disable : 5244) // '#include <meow>' in the purview of module 'std' appears erroneous.

// "C++ library headers" [tab:headers.cpp]
#include <algorithm>
#if _HAS_STATIC_RTTI
#include <any>
#endif // _HAS_STATIC_RTTI
#include <array>
#include <atomic>
#include <barrier>
#include <bit>
#include <bitset>
#include <charconv>
#include <chrono>
#include <codecvt>
#include <compare>
#include <complex>
#include <concepts>
#include <condition_variable>
#include <coroutine>
#include <deque>
#include <exception>
#include <execution>
#include <expected>
#include <filesystem>
#include <format>
#include <forward_list>
#include <fstream>
#include <functional>
#include <future>
#include <initializer_list>
#include <iomanip>
#include <ios>
#include <iosfwd>
#include <iostream>
#include <istream>
#include <iterator>
#include <latch>
#include <limits>
#include <list>
#include <locale>
#include <map>
#include <memory>
#include <memory_resource>
#include <mutex>
#include <new>
#include <numbers>
#include <numeric>
#include <optional>
#include <ostream>
#include <print>
#include <queue>
#include <random>
#include <ranges>
#include <ratio>
#include <regex>
#include <scoped_allocator>
#include <semaphore>
#include <set>
#include <shared_mutex>
#include <source_location>
#include <span>
#include <spanstream>
#include <sstream>
#include <stack>
#include <stacktrace>
#include <stdexcept>
#include <stdfloat>
#include <stop_token>
#include <streambuf>
#include <string>
#include <string_view>
#include <strstream>
#include <syncstream>
#include <system_error>
#include <thread>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <valarray>
#include <variant>
#include <vector>
#include <version>

// "C++ headers for C library facilities" [tab:headers.cpp.c]
#include <cassert>
#include <cctype>
#include <cerrno>
#include <cfenv>
#include <cfloat>
#include <cinttypes>
#include <climits>
#include <clocale>
#include <cmath>
#include <csetjmp>
#include <csignal>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cuchar>
#include <cwchar>
#include <cwctype>

#pragma warning(pop)

#include<fmt/core.h>
#include<fmt/xchar.h>
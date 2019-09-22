#pragma once

#include <variant>
#include <string>
#include <functional>

#include <logger/logger.h>
#include <wpr/Readers.hpp>
#include <wpr/Containers.hpp>
#include <wpr/Details.hpp>

namespace wpr::test {

struct Result {
    struct UnexpectedResult {
        bool expected_is_success;
        std::string expected;
        bool result_is_success;
        std::string result;
    };

    struct UnexpectedCursor {
        std::size_t expected;
        std::size_t result;
    };

    std::string message;
    std::string input;
    std::optional<std::variant<UnexpectedResult, UnexpectedCursor>> unexpected;  
};

template<typename P, bool isSuccess, typename E, typename R = BoundedReader>
Result run_test(std::string const& message, std::size_t expected_cursor, std::string const& input, E expected) {
    static_assert(details::is_parser_v<P, R>, "Unfortunetly, P is not a parser...");

    R reader(input);
    auto res = P::parse(reader);

    Result test_result;
    test_result.message = message;
    test_result.input = input;

    if (res.is_success() == isSuccess) {
        auto const& value = [&] () { if constexpr (isSuccess || !P::can_fail) return res.success(); else return res.error(); }();
        constexpr bool E_in_sum = [] {
            if constexpr (details::is_same_HK_type_v<std::decay_t<decltype(value)>, Sum>)
                if constexpr (details::is_in_v<std::decay_t<E>, details::list_from_t<Sum, std::decay_t<decltype(value)>>>)
                    return true;

            return false;
        }();

        if constexpr (E_in_sum) {
            if (std::get<std::decay_t<E>>(value) == expected) {
                if (res.cursor() != expected_cursor) {
                    Result::UnexpectedCursor info;
                    info.expected = expected_cursor;
                    info.result = res.cursor();
                    test_result.unexpected = std::move(info);
                }
                return test_result;
            }
        } else {
            if (value == expected) {
                if (res.cursor() != expected_cursor) {
                    Result::UnexpectedCursor info;
                    info.expected = expected_cursor;
                    info.result = res.cursor();
                    test_result.unexpected = std::move(info);
                }
                return test_result;
            }
        }
    }

    Result::UnexpectedResult info;
    info.expected_is_success = isSuccess;
    info.result_is_success = res.is_success();
    info.expected = describe(expected);
    if (res.is_success()) {
        info.result = describe(res.success());
    } else {
        if constexpr (P::can_fail) {
            info.result = describe(res.error());
        }
    }
    test_result.unexpected = std::move(info);

    return test_result;
}

struct Input {
    std::string input;
    std::size_t cursor;
};

template<bool isSuccess, typename R>
struct SpecificCase {
    R res;
    std::vector<Input> inputs = {};
};

static inline constexpr bool successful = true;
static inline constexpr bool failure = false;

using Case = std::function<std::vector<Result>(std::string const&)>;

template<typename P>
struct Parser {
    std::string message;
    std::vector<Case> cases = {};
};

template<typename M, typename P>
auto it(M const& message, P) { return Parser<std::decay_t<P>>{ std::string{ message }}; }

template<typename R>
auto should_be(R&& res) { return SpecificCase<successful, R>{ std::forward<R>(res), {} }; }

template<typename E>
auto should_fail(E&& err) { return SpecificCase<failure, E>{ std::forward<E>(err), {} }; }

template<typename M>
auto on(M&& m, std::intmax_t cursor) { 
    Input input{ std::string{ std::forward<M>(m) }, 0 }; 
    input.cursor = static_cast<std::size_t>(
        cursor < 0 ? 
            static_cast<std::intmax_t>(input.input.size()) + cursor 
        :   cursor); 
    return input; 
}

template<typename M>
auto on(M&& m) { 
    Input input{ std::string{ std::forward<M>(m) }, 0 }; 
    input.cursor = input.input.size(); 
    return input; 
}

template<bool isSuccess, typename R>
auto operator >> (SpecificCase<isSuccess, R> s, Input o) {
    s.inputs.emplace_back(std::move(o));
    return s;
}

template<typename P, bool isSuccess, typename R>
Case make_case(SpecificCase<isSuccess, R> c) {
    return [specific_case = std::move(c)] (std::string const& message) {
        std::vector<Result> results;
        for(auto const& i : specific_case.inputs) {
            auto result = run_test<P, isSuccess>(message, i.cursor, i.input, specific_case.res);
            results.emplace_back(std::move(result));
        }

        return results;
    };
}

template<typename P, bool isSuccess, typename R>
Parser<P> operator > (Parser<P> i, SpecificCase<isSuccess, R> s) {
    i.cases.emplace_back(make_case<P>(std::move(s)));
    return i;
}

struct Tester {
    std::size_t count{ 0 };
    std::size_t success{ 0 };

    bool report() const {
        if (count == success) {
            ws::module::successln(success, "/", count, " tests passed!");
        } else {
            ws::module::errorln(success, "/", count, " tests passed!");
        }

        return count == success;
    }
};

template<typename P>
Tester& operator += (Tester& tester, Parser<P> const& p) {
    ws::module::println(ws::module::style::bold, "[", p.message, "]", ws::module::style::reset);
    
    std::size_t count{ 0 };
    std::size_t success{ 0 };

    for(auto c : p.cases) {
        auto results = c(p.message);

        for(auto res : results) {
            ++count;

            if (!res.unexpected) {
                ++success;
            } else {
                ws::module::print(ws::module::tabs(1), ws::module::style::bold, ws::module::colour::fg::cyan, "[", res.input, "]", ws::module::style::reset);

                if (auto* value = std::get_if<Result::UnexpectedResult>(&*res.unexpected)) {
                    auto expected_color = value->expected_is_success ? ws::module::colour::fg::green : ws::module::colour::fg::red;
                    auto result_color = value->result_is_success ? ws::module::colour::fg::green : ws::module::colour::fg::red;

                    ws::module::println(
                        ", expected ", ws::module::style::bold, expected_color, "[", value->expected, "]", ws::module::style::reset,
                        " but got ", ws::module::style::bold, result_color, "[", value->result, "]", ws::module::style::reset);
                } else {
                    auto& cursor = std::get<Result::UnexpectedCursor>(*res.unexpected);
                    ws::module::println(
                        ", expected cursor to be ", ws::module::style::bold, ws::module::colour::fg::magenta, cursor.expected, ws::module::style::reset,
                        " but got ", ws::module::style::bold, ws::module::colour::fg::magenta, cursor.result, ws::module::style::reset);
                }
            }
        }
    }

    ws::module::println(ws::module::style::bold, count == success ? ws::module::colour::fg::green : ws::module::colour::fg::red, "[", success, "/", count, "]", ws::module::style::reset);

    tester.count += count;
    tester.success += success;

    return tester;
}


}
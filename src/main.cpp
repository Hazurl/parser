#include <iostream>
#include <cctype>
#include <optional>
#include <sstream>
#include <variant>
#include <type_traits>
#include <experimental/type_traits>
#include <cassert>

#include <logger/logger.h>
#include <json.hpp>
#include <wpr/Parsers.hpp>
#include <wpr/Containers.hpp>
#include <wpr/Details.hpp>

namespace wpre = wpr::error;
namespace wprd = wpr::details;

struct BoundedReader {

    BoundedReader(char const* begin_, char const* end_) 
        : begin{ begin_                          }, end{ end_               }, index { 0 } {}
    template<std::size_t N>
    BoundedReader(char const (&str)[N]) 
        : begin{ str                             }, end{ str + N - 1        }, index { 0 } {}
    BoundedReader(std::string const& str) 
        : begin{ str.empty() ? nullptr : &str[0] }, end{ begin + str.size() }, index { 0 } {}
    BoundedReader(std::string_view const& str) 
        : begin{ str.empty() ? nullptr : &str[0] }, end{ begin + str.size() }, index { 0 } {}

    static BoundedReader copy_move(BoundedReader r, std::size_t advance) {
        r.index += advance;
        return r;
    }

    static BoundedReader from_cursor(BoundedReader r, std::size_t cursor) {
        r.index = cursor;
        return r;
    }

    std::size_t cursor() const {
        return index;
    }

    bool is_end(std::size_t i = 0) const {
        return begin + index + i >= end;
    }

    char const& operator[](std::size_t i) const {
        assert(!is_end(i));
        return begin[index + i];
    }

    char peek() const {
        return operator[](0);
    }


    BoundedReader& operator++() {
        ++index;
        return *this;
    }

private:

    char const* begin{ nullptr };
    char const* end{ nullptr };
    std::size_t index{ 0 };

};
/*
struct StringReader {
    std::string_view str;
    std::size_t cursor{ 0 };

    bool empty() const {
        return cursor >= str.size();
    }

    char get() const {
        return str.at(cursor);
    }

    void next() {
        ++cursor;
    }

    char peek() const {
        return get();
    }

    StringReader copy_with_cursor(std::size_t c) const {
        return {
            str,
            c
        };
    }
};*/

template<typename...>
struct debug_t;

template<typename R, typename P, std::size_t N>
void test(char const* message, char const (&input)[N], R expected, P) {
    static_assert(wprd::is_parser_v<P, BoundedReader>, "Unfortunetly, P is not a parser...");

    BoundedReader reader(input);
    auto res = P::parse(reader);

    if (res.is_error()) {
        ws::module::errorln(
            ws::module::style::bold, "[", message, "]", ws::module::style::reset, 
            " with ", ws::module::style::bold, ws::module::colour::fg::cyan, "[", input, "]", ws::module::style::reset, 
            ", expected ", ws::module::style::bold, ws::module::colour::fg::green, "[", wpr::describe(expected), "]", ws::module::style::reset,
            " but got ", ws::module::style::bold, ws::module::colour::fg::red, "[", wpr::describe(res), "]", ws::module::style::reset);
        return;
    }

    auto& value = res.success(); 
    if (!(value == expected)) {
        ws::module::errorln(
            ws::module::style::bold, "[", message, "]", ws::module::style::reset, 
            " with ", ws::module::style::bold, ws::module::colour::fg::cyan, "[", input, "]", ws::module::style::reset,
            ", expected ", ws::module::style::bold, ws::module::colour::fg::green, "[", wpr::describe(expected), "]", ws::module::style::reset,
            " but got ", ws::module::style::bold, ws::module::colour::fg::green, "[", wpr::describe(value), "]", ws::module::style::reset);
        return;
    }

    ws::module::successln(
        ws::module::style::bold, "[", message, "]", ws::module::style::reset, 
        " with ", ws::module::style::bold, ws::module::colour::fg::cyan, "[", input, "]", ws::module::style::reset, " parsed successfully");
}

template<typename E, typename P, std::size_t N>
void test_err(char const* message, char const (&input)[N], E error, P) {
    static_assert(wprd::is_parser_v<P, BoundedReader>, "Unfortunetly, P is not a parser...");

    BoundedReader reader(input);
    auto res = P::parse(reader);

    if (res.is_success()) {
        ws::module::errorln(
            ws::module::style::bold, "[", message, "]", ws::module::style::reset, 
            " with ", ws::module::style::bold, ws::module::colour::fg::cyan, "[", input, "]", ws::module::style::reset, 
            ", expected ", ws::module::style::bold, ws::module::colour::fg::red, "[", wpr::describe(error), "]", ws::module::style::reset, " but",
            " got ", ws::module::style::bold, ws::module::colour::fg::green, "[", wpr::describe(res.success()), "]", ws::module::style::reset);
        return;
    }

    if constexpr (P::can_fail) {
        auto& value = res.template error(); 

        if constexpr (wprd::is_same_HK_type_v<wprd::error_of_t<wprd::result_type_t<P>>, wpr::Sum>) {
            if (res.template is_error() && !std::holds_alternative<E>(res.error())) {
                ws::module::errorln(
                    ws::module::style::bold, "[", message, "]", ws::module::style::reset, "]", 
                    " with ", ws::module::style::bold, ws::module::colour::fg::cyan, "[", input, "]", ws::module::style::reset, 
                    ", expected ", ws::module::style::bold, ws::module::colour::fg::red, "[", wpr::describe(error), "]", ws::module::style::reset, " but",
                    " got ", ws::module::style::bold, ws::module::colour::fg::red, "[", wpr::describe(res), "]", ws::module::style::reset);
                return;
            }

            if (!(std::get<E>(value) == error)) {
                ws::module::errorln(
                    ws::module::style::bold, "[", message, "]", ws::module::style::reset, 
                    " with ", ws::module::style::bold, ws::module::colour::fg::cyan, "[", input, "]", ws::module::style::reset, 
                    ", expected ", ws::module::style::bold, ws::module::colour::fg::red, "[", wpr::describe(error), "]", ws::module::style::reset, " but",
                    " got ", ws::module::style::bold, ws::module::colour::fg::red, "[", wpr::describe(value), "]", ws::module::style::reset);
                return;
            }
        } else {
            if (!(value == error)) {
                ws::module::errorln(
                    ws::module::style::bold, "[", message, "]", ws::module::style::reset, 
                    " with ", ws::module::style::bold, ws::module::colour::fg::cyan, "[", input, "]", ws::module::style::reset, 
                    ", expected ", ws::module::style::bold, ws::module::colour::fg::red, "[", wpr::describe(error), "]", ws::module::style::reset, " but",
                    " got ", ws::module::style::bold, ws::module::colour::fg::red, "[", wpr::describe(value), "]", ws::module::style::reset);
                return;
            }
        }


        ws::module::successln(
            ws::module::style::bold, "[", message, "]", ws::module::style::reset, 
            " with ", ws::module::style::bold, ws::module::colour::fg::cyan, "[", input, "]", ws::module::style::reset, " successfully failed");
    }
}

/*

    These are all manual tests to test the parser validator
    Uncomment each of them to see if the assertion fail with the correct message
 */

/*
struct GoodParser : wpr::Parser<GoodParser, char, wpre::EndOfFile> {
    template<typename R> static wpr::Result<char, wpre::EndOfFile> parse(R reader) { return wpr::success(reader.cursor, '0'); }
};
static_assert(wprd::is_parser_valid_v<BoundedReader, GoodParser>);
*/
/*
//    static_assert(parser_details::false_v<T, Es...>, "The parse method's return type should be a Result");
struct ReturnTypeNotResult : wpr::Parser<ReturnTypeNotResult, char, wpre::EndOfFile> {
    template<typename R> static char parse(R reader) { return '0'; }
};
static_assert(wprd::is_parser_valid_v<BoundedReader, ReturnTypeNotResult>);
*/
/*
//    static_assert(parser_details::false_v<T>, "The parse method's return type is a Result with the wrong success type");
struct ResultSuccessWrong : wpr::Parser<ResultSuccessWrong, char, wpre::EndOfFile> {
    template<typename R> static wpr::Result<int, wpre::EndOfFile> parse(R reader) { return wpr::success(reader.cursor, 1337); }
};
static_assert(wprd::is_parser_valid_v<BoundedReader, ResultSuccessWrong>);
*/
/*
//    static_assert(parser_details::false_v<Es...>, "The parse method's return type is a Result with the wrong error list");
struct ResultErrorsWrong : wpr::Parser<ResultErrorsWrong, char, wpre::EndOfFile> {
    template<typename R> static wpr::Result<char, wpre::EndOfFile, int> parse(R reader) { return wpr::success(reader.cursor, '0'); }
};
static_assert(wprd::is_parser_valid_v<BoundedReader, ResultErrorsWrong>);
*/
/*
//    static_assert(parser_details::false_v<T, Es...>, "The parse method's return type is a Result with the wrong success type and error list");
struct ResultSuccessAndErrorsWrong : wpr::Parser<ResultSuccessAndErrorsWrong, char, wpre::EndOfFile> {
    template<typename R> static wpr::Result<int, wpre::EndOfFile, int> parse(R reader) { return wpr::success(reader.cursor, 1337); }
};
static_assert(wprd::is_parser_valid_v<BoundedReader, ResultSuccessAndErrorsWrong>);
*/
/*
//    static_assert(parser_details::false_v<R>, "The parse method should be static");
struct ParseNotStatic : wpr::Parser<ParseNotStatic, char, wpre::EndOfFile> {
    template<typename R> wpr::Result<char, wpre::EndOfFile> parse(R reader) { return wpr::success(reader.cursor, '0'); }
};
static_assert(wprd::is_parser_valid_v<BoundedReader, ParseNotStatic>);
*/
/*
//    static_assert(parser_details::false_v<P>, "The parse member is detected but doesn't seems like a static method, makes sure it has the signature: \ntemplate<typename Reader>\nstatic Result<T, Es...> parse(Reader reader)");
struct ParseNotFunction : wpr::Parser<ParseNotFunction, char, wpre::EndOfFile> {
    template<typename R> R parse; 
};
static_assert(wprd::is_parser_valid_v<BoundedReader, ParseNotFunction>);
*/
/*
//    static_assert(parser_details::false_v<R>, "The parse method should only have one argument, the reader (taken by value)");
struct ParseWithMoreArg : wpr::Parser<ParseWithMoreArg, char, wpre::EndOfFile> {
    template<typename R> static wpr::Result<char, wpre::EndOfFile> parse(R reader, int) { return wpr::success(reader.cursor, '0'); }
};
static_assert(wprd::is_parser_valid_v<BoundedReader, ParseWithMoreArg>);
*/
/*
//    static_assert(parser_details::false_v<R>, "The parse method isn't detected, makes sure it has the signature: \ntemplate<typename Reader>\nstatic Result<T, Es...> parse(Reader reader)");
struct Typo : wpr::Parser<Typo, char, wpre::EndOfFile> {
    template<typename R> static wpr::Result<char, wpre::EndOfFile> parse_(R reader) { return wpr::success(reader.cursor, '0'); }
};
static_assert(wprd::is_parser_valid_v<BoundedReader, Typo>);
*/
/*
//    static_assert(parser_details::false_v<P>, "The type you wants to validate doesn't inherit from Parser, to be more exact, the parser_type alias isn't a parser");
struct ParserTypeNotParser : BoundedReader {
    using parser_type = BoundedReader;
    template<typename R> static wpr::Result<char, wpre::EndOfFile> parse(R reader) { return wpr::success(reader.cursor, '0'); }
};
static_assert(wprd::is_parser_valid_v<BoundedReader, ParserTypeNotParser>);
*/
/*
//    static_assert(parser_details::false_v<P>, "The type you wants to validate doesn't inherit from Parser");
struct NoBaseParser {
    template<typename R> static wpr::Result<char, wpre::EndOfFile> parse(R reader) { return wpr::success(reader.cursor, '0'); }
};
static_assert(wprd::is_parser_valid_v<BoundedReader, NoBaseParser>);
*/

int char_to_int(char c) { return static_cast<int>(c); }
int digit_to_int(char c) { return c - '0'; }
char default_maybe(wpr::Maybe<char>&& m) { return m.value_or(' '); };
bool is_digit(char c) { return c >= '0' && c <= '9'; }
char EOF_to_space(wpre::EndOfFile) { return ' '; };
std::string select_middle(wpr::Product<char, std::string, char>&& p) {
    return std::get<1>(p);
}
int string_to_int(std::string&& str) {
    int i{ 0 };
    for(auto c : str) {
        i *= 10;
        i += digit_to_int(c);
    }
    return i;
}

int main() {
    test(
        "Simple Next",
        "something",
        's',
        wpr::nextc
    );

    test_err(
        "Next with EOF",
        "",
        wpre::EndOfFile{},
        wpr::nextc
    );

    test(
        "Optional Next",
        "something",
        wpr::Maybe<char>{ 's' },
        wpr::opt<wpr::nextc>
    );

    test(
        "Optional Next with EOF",
        "",
        wpr::Maybe<char>{},
        wpr::opt<wpr::nextc>
    );

    test(
        "Double Next",
        "something",
        wpr::Product('s', 'o'),
        wpr::seq<wpr::nextc, wpr::nextc>
    );

    test_err(
        "Seq, first error",
        "",
        wpre::EndOfFile{},
        wpr::seq<wpr::nextc, wpr::nextc>
    );

    test_err(
        "Seq, second error",
        "s",
        wpre::EndOfFile{},
        wpr::seq<wpr::nextc, wpr::nextc>
    );

    test(
        "Match maybe 's'",
        "something",
        wpr::Maybe<char>('s'),
        wpr::opt<wpr::ch<'s'>>
    );

    test(
        "Match maybe 's' on EOF",
        "",
        wpr::Maybe<char>(std::nullopt),
        wpr::opt<wpr::ch<'s'>>
    );

    test_err(
        "No Match 's'",
        "a",
        wpre::NotMatching<'s'>{},
        wpr::ch<'s'>
    );

    test(
        "Simple ch",
        "something",
        's',
        wpr::ch<'s'>
    );

    test_err(
        "ch on EOF",
        "",
        wpre::EndOfFile{},
        wpr::ch<'s'>
    );

    test_err(
        "ch not matching",
        "something",
        wpre::NotMatching<'x'>{},
        wpr::ch<'x'>
    );

    test(
        "Match 's' after 'a' failed",
        "something",
        wpr::Sum<char>('s'),
        wpr::first<wpr::ch<'a'>, wpr::ch<'s'>>
    );

    test(
        "Match 's' before 'a' failed",
        "something",
        wpr::Sum<char>('s'),
        wpr::first<wpr::ch<'s'>, wpr::ch<'a'>>
    );

    test(
        "Match 's' before `nextc` works",
        "something",
        wpr::Sum<char>('s'),
        wpr::first<wpr::ch<'s'>, wpr::nextc>
    );

    test_err(
        "No Match 's' and 'a'",
        "foo",
        wpr::Product<wpr::Sum<wpre::NotMatching<'s'>, wpre::EndOfFile>, wpr::Sum<wpre::NotMatching<'a'>, wpre::EndOfFile>>(),
        wpr::first<wpr::ch<'s'>, wpr::ch<'a'>>
    );

    test(
        "Simple Next with transformer",
        "something",
        char_to_int('s'),
        wpr::nextc [ wpr::map<char_to_int> ]
    );

    test_err(
        "Next with transformer on EOF",
        "",
        wpre::EndOfFile{},
        wpr::nextc [ wpr::map<char_to_int> ]
    );

    test(
        "Match 's'",
        "something",
        's',
        wpr::opt<wpr::ch<'s'>> [ wpr::map<default_maybe> ]
    );

    test(
        "many Next",
        "something",
        std::string{ "something" },
        wpr::many<wpr::nextc, std::basic_string>
    );

    test(
        "repeat at least 5 times Next",
        "something",
        std::string{ "something" },
        wpr::repeat<wpr::nextc, 5, wpr::open_maximum, std::basic_string>
    );

    test(
        "repeat between 5 and 6 times Next",
        "something",
        std::string{ "someth" },
        wpr::repeat<wpr::nextc, 5, 6, std::basic_string>
    );

    test(
        "Exactly 3 times Next",
        "something",
        std::string{ "som" },
        wpr::exact<wpr::nextc, 3, std::basic_string>
    );

    test_err(
        "Some Next on EOF",
        "",
        wpre::Expected<wpr::NextC>{},
        wpr::some<wpr::nextc>
    );

    test_err(
        "repeat at least 5 times Next on 4 characters",
        "some",
        wpre::Expected<wpr::NextC>{},
        wpr::repeat<wpr::nextc, 5, wpr::open_maximum, std::basic_string>
    );

    test(
        "Next which satisfy is_digit",
        "123",
        '1',
        wpr::nextc [ wpr::filter<is_digit> ]
    );

    test_err(
        "Next which does not satisfy is_digit",
        "something",
        wpre::PredicateFailure{},
        wpr::nextc [ wpr::filter<is_digit> ]
    );

    test_err(
        "Next with satisfy `is_digit` on EOF",
        "",
        wpre::EndOfFile{},
        wpr::nextc [ wpr::filter<is_digit> ]
    );

    test(
        "Next which satisfy is_digit and transform to an int",
        "123",
        1,
        wpr::nextc [ wpr::filter<is_digit> ] [ wpr::map<digit_to_int> ]
    );

    test(
        "Next on EOF with handle",
        "",
        ' ',
        wpr::nextc [ wpr::handler<EOF_to_space> ]
    );

    test(
        "Next on EOF with default",
        "",
        ' ',
        wpr::nextc [ wpr::or_else<' '> ]
    );

    constexpr auto integer = 
        wpr::seq<
            wpr::ch<'('>, 
            wpr::many<wpr::nextc [ wpr::filter<is_digit> ], std::basic_string>, 
            wpr::ch<')'>
        > [ wpr::peek<1> ] [ wpr::map<string_to_int> ];

    test(
        "parse integer in parenthesis",
        "(123456)",
        123456,
        integer
    );

    test_err(
        "parse integer in parenthesis with no '('",
        "13)",
        wpr::Sum<wpre::NotMatching<'('>, wpre::EndOfFile>{ wpre::NotMatching<'('>{} },
        integer
    );

    test_err(
        "parse integer in parenthesis with no '(' but EOF",
        "",
        wpr::Sum<wpre::NotMatching<'('>, wpre::EndOfFile>{ wpre::EndOfFile{} },
        integer
    );

    test_err(
        "parse integer in parenthesis with no ')'",
        "(13 ",
        wpr::Sum<wpre::NotMatching<')'>, wpre::EndOfFile>{ wpre::NotMatching<')'>{} },
        integer
    );

    test_err(
        "parse integer in parenthesis with no ')' but EOF",
        "(13",
        wpr::Sum<wpre::NotMatching<')'>, wpre::EndOfFile>{ wpre::EndOfFile{} },
        integer
    );

    test(
        "Simple peek",
        "something",
        'm',
        wpr::seq<wpr::nextc, wpr::nextc, wpr::nextc> [ wpr::peek<2> ]
    );

    test(
        "Simple select",
        "something",
        wpr::Product('m', 'o', 'm'),
        wpr::seq<wpr::nextc, wpr::nextc, wpr::nextc> [ wpr::select<2, 1, 2> ]
    );

    test_err(
        "Simple peek_err",
        "",
        wpre::EndOfFile{},
        wpr::first<wpr::nextc, wpr::nextc, wpr::nextc> [ wpr::peek_err<2> ]
    );

    test_err(
        "Simple select_err",
        "",
        wpr::Product(wpre::EndOfFile{}, wpre::EndOfFile{}, wpre::EndOfFile{}),
        wpr::first<wpr::nextc, wpr::nextc> [ wpr::select_err<1, 0, 1> ]
    );

/*
    debug_t<

    > _;
*/

    static_assert(
        wprd::is_parser_valid_v<BoundedReader, wpr::NextC> &&
        wprd::is_parser_valid_v<BoundedReader, wpr::Opt<wpr::NextC>> &&
        wprd::is_parser_valid_v<BoundedReader, wpr::Seq<wpr::NextC, wpr::NextC>> &&
        wprd::is_parser_valid_v<BoundedReader, wpr::Opt<wpr::Opt<wpr::NextC>>> &&
        wprd::is_parser_valid_v<BoundedReader, wpr::Seq<wpr::Seq<wpr::NextC, wpr::NextC>, wpr::NextC>> &&
        wprd::is_parser_valid_v<BoundedReader, wpr::Seq<wpr::Opt<wpr::NextC>, wpr::Seq<wpr::NextC, wpr::NextC>>> &&
        wprd::is_parser_valid_v<BoundedReader, wpr::First<decltype(wpr::ch<'a'>), wpr::NextC>> &&
        wprd::is_parser_valid_v<BoundedReader, wpr::First<wpr::Opt<wpr::NextC>, wpr::Opt<wpr::NextC>>> &&
        wprd::is_parser_valid_v<BoundedReader, wpr::Transformer<wpr::NextC, char_to_int>> &&
        wprd::is_parser_valid_v<BoundedReader, wpr::Satisfy<wpr::NextC, is_digit>> &&
        wprd::is_parser_valid_v<BoundedReader, wpr::Handle<wpr::NextC, EOF_to_space>> &&
        wprd::is_parser_valid_v<BoundedReader, wpr::Repeat<wpr::NextC, 2, wpr::open_maximum, std::basic_string>> &&
        true, 
        "Something is wrong...");

    return 0;
}

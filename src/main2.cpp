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
#include <ws/parser2/Test.hpp>
#include <ws/parser2/Readers.hpp>
#include <ws/parser2/Parsers.hpp>
#include <ws/parser2/Containers.hpp>
#include <ws/parser2/Details.hpp>

namespace wsp = ws::parser2;
namespace wspe = wsp::error;
namespace wspd = wsp::details;
namespace wspt = wsp::test;

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


/*

    These are all manual tests to test the parser validator
    Uncomment each of them to see if the assertion fail with the correct message
 */

/*
struct GoodParser : wsp::Parser<GoodParser, char, wspe::EndOfFile> {
    template<typename R> static wsp::Result<char, wspe::EndOfFile> parse(R reader) { return wsp::success(reader.cursor, '0'); }
};
static_assert(wspd::is_parser_valid_v<wsp::BoundedReader, GoodParser>);
*/
/*
//    static_assert(parser_details::false_v<T, Es...>, "The parse method's return type should be a Result");
struct ReturnTypeNotResult : wsp::Parser<ReturnTypeNotResult, char, wspe::EndOfFile> {
    template<typename R> static char parse(R reader) { return '0'; }
};
static_assert(wspd::is_parser_valid_v<wsp::BoundedReader, ReturnTypeNotResult>);
*/
/*
//    static_assert(parser_details::false_v<T>, "The parse method's return type is a Result with the wrong success type");
struct ResultSuccessWrong : wsp::Parser<ResultSuccessWrong, char, wspe::EndOfFile> {
    template<typename R> static wsp::Result<int, wspe::EndOfFile> parse(R reader) { return wsp::success(reader.cursor, 1337); }
};
static_assert(wspd::is_parser_valid_v<wsp::BoundedReader, ResultSuccessWrong>);
*/
/*
//    static_assert(parser_details::false_v<Es...>, "The parse method's return type is a Result with the wrong error list");
struct ResultErrorsWrong : wsp::Parser<ResultErrorsWrong, char, wspe::EndOfFile> {
    template<typename R> static wsp::Result<char, wspe::EndOfFile, int> parse(R reader) { return wsp::success(reader.cursor, '0'); }
};
static_assert(wspd::is_parser_valid_v<wsp::BoundedReader, ResultErrorsWrong>);
*/
/*
//    static_assert(parser_details::false_v<T, Es...>, "The parse method's return type is a Result with the wrong success type and error list");
struct ResultSuccessAndErrorsWrong : wsp::Parser<ResultSuccessAndErrorsWrong, char, wspe::EndOfFile> {
    template<typename R> static wsp::Result<int, wspe::EndOfFile, int> parse(R reader) { return wsp::success(reader.cursor, 1337); }
};
static_assert(wspd::is_parser_valid_v<wsp::BoundedReader, ResultSuccessAndErrorsWrong>);
*/
/*
//    static_assert(parser_details::false_v<R>, "The parse method should be static");
struct ParseNotStatic : wsp::Parser<ParseNotStatic, char, wspe::EndOfFile> {
    template<typename R> wsp::Result<char, wspe::EndOfFile> parse(R reader) { return wsp::success(reader.cursor, '0'); }
};
static_assert(wspd::is_parser_valid_v<wsp::BoundedReader, ParseNotStatic>);
*/
/*
//    static_assert(parser_details::false_v<P>, "The parse member is detected but doesn't seems like a static method, makes sure it has the signature: \ntemplate<typename Reader>\nstatic Result<T, Es...> parse(Reader reader)");
struct ParseNotFunction : wsp::Parser<ParseNotFunction, char, wspe::EndOfFile> {
    template<typename R> R parse; 
};
static_assert(wspd::is_parser_valid_v<wsp::BoundedReader, ParseNotFunction>);
*/
/*
//    static_assert(parser_details::false_v<R>, "The parse method should only have one argument, the reader (taken by value)");
struct ParseWithMoreArg : wsp::Parser<ParseWithMoreArg, char, wspe::EndOfFile> {
    template<typename R> static wsp::Result<char, wspe::EndOfFile> parse(R reader, int) { return wsp::success(reader.cursor, '0'); }
};
static_assert(wspd::is_parser_valid_v<wsp::BoundedReader, ParseWithMoreArg>);
*/
/*
//    static_assert(parser_details::false_v<R>, "The parse method isn't detected, makes sure it has the signature: \ntemplate<typename Reader>\nstatic Result<T, Es...> parse(Reader reader)");
struct Typo : wsp::Parser<Typo, char, wspe::EndOfFile> {
    template<typename R> static wsp::Result<char, wspe::EndOfFile> parse_(R reader) { return wsp::success(reader.cursor, '0'); }
};
static_assert(wspd::is_parser_valid_v<wsp::BoundedReader, Typo>);
*/
/*
//    static_assert(parser_details::false_v<P>, "The type you wants to validate doesn't inherit from Parser, to be more exact, the parser_type alias isn't a parser");
struct ParserTypeNotParser : wsp::BoundedReader {
    using parser_type = wsp::BoundedReader;
    template<typename R> static wsp::Result<char, wspe::EndOfFile> parse(R reader) { return wsp::success(reader.cursor, '0'); }
};
static_assert(wspd::is_parser_valid_v<wsp::BoundedReader, ParserTypeNotParser>);
*/
/*
//    static_assert(parser_details::false_v<P>, "The type you wants to validate doesn't inherit from Parser");
struct NoBaseParser {
    template<typename R> static wsp::Result<char, wspe::EndOfFile> parse(R reader) { return wsp::success(reader.cursor, '0'); }
};
static_assert(wspd::is_parser_valid_v<wsp::BoundedReader, NoBaseParser>);
*/

int char_to_int(char c) { return static_cast<int>(c); }
int digit_to_int(char c) { return c - '0'; }
char default_maybe(wsp::Maybe<char>&& m) { return m.value_or(' '); };
bool is_digit(char c) { return c >= '0' && c <= '9'; }
char EOF_to_space(wspe::EndOfFile) { return ' '; };
std::string select_middle(wsp::Product<char, std::string, char>&& p) {
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

    wspt::Tester tester;

    tester += wspt::it("Next", wsp::nextc) 
        > wspt::should_be('s')
            >> wspt::on("s")
            >> wspt::on("something")

        > wspt::should_fail(wspe::EndOfFile{})
            >> wspt::on("")
    ;

    tester += wspt::it("Optional next", wsp::opt<wsp::nextc>) 
        > wspt::should_be(wsp::Maybe<char>{ 's' })
            >> wspt::on("s")
            >> wspt::on("something")

        > wspt::should_be(wsp::Maybe<char>{})
            >> wspt::on("")
    ;

    tester += wspt::it("Double next", wsp::seq<wsp::nextc, wsp::nextc>) 
        > wspt::should_be(wsp::Product('s', 'o'))
            >> wspt::on("so")
            >> wspt::on("something")

        > wspt::should_fail(wspe::EndOfFile{})
            >> wspt::on("")
            >> wspt::on("s")
    ;

    tester += wspt::it("Match 's'", wsp::ch<'s'>) 
        > wspt::should_be('s')
            >> wspt::on("s")
            >> wspt::on("something")

        > wspt::should_fail(wspe::NotMatching<'s'>{})
            >> wspt::on("z")

        > wspt::should_fail(wspe::EndOfFile{})
            >> wspt::on("")
    ;

    tester += wspt::it("Match 's'", wsp::first<wsp::ch<'s'>, wsp::nextc>) 
        > wspt::should_be(wsp::Sum<char>('s'))
            >> wspt::on("s")
            >> wspt::on("something")

        > wspt::should_be(wsp::Sum<char>('a'))
            >> wspt::on("a")
            >> wspt::on("another")

        > wspt::should_fail(wsp::Product<wsp::Sum<wspe::NotMatching<'s'>, wspe::EndOfFile>, wspe::EndOfFile>{
            wsp::Sum<wspe::NotMatching<'s'>, wspe::EndOfFile>{ wspe::EndOfFile{} },
            wspe::EndOfFile{}
        })
            >> wspt::on("")
    ;

    tester += wspt::it("Match 'a' or 's'", wsp::first<wsp::ch<'s'>, wsp::ch<'a'>>) 
        > wspt::should_be(wsp::Sum<char>('s'))
            >> wspt::on("s")
            >> wspt::on("something")

        > wspt::should_be(wsp::Sum<char>('a'))
            >> wspt::on("a")
            >> wspt::on("another")

        > wspt::should_fail(wsp::Product<wsp::Sum<wspe::NotMatching<'s'>, wspe::EndOfFile>, wsp::Sum<wspe::NotMatching<'a'>, wspe::EndOfFile>>(
            wsp::Sum<wspe::NotMatching<'s'>, wspe::EndOfFile>{ wspe::NotMatching<'s'>{} },
            wsp::Sum<wspe::NotMatching<'a'>, wspe::EndOfFile>{ wspe::NotMatching<'a'>{} }
        ))
            >> wspt::on("foo")

        > wspt::should_fail(wsp::Product<wsp::Sum<wspe::NotMatching<'s'>, wspe::EndOfFile>, wsp::Sum<wspe::NotMatching<'a'>, wspe::EndOfFile>>(
            wsp::Sum<wspe::NotMatching<'s'>, wspe::EndOfFile>{ wspe::EndOfFile{} },
            wsp::Sum<wspe::NotMatching<'a'>, wspe::EndOfFile>{ wspe::EndOfFile{} }
        ))
            >> wspt::on("")
    ;

    tester += wspt::it("Map next", wsp::nextc [ wsp::map<char_to_int> ]) 
        > wspt::should_be(char_to_int('s'))
            >> wspt::on("s")
            >> wspt::on("something")

        > wspt::should_fail(wspe::EndOfFile{})
            >> wspt::on("")
    ;

    tester += wspt::it("Map optional next", wsp::opt<wsp::ch<'s'>> [ wsp::map<default_maybe> ]) 
        > wspt::should_be('s')
            >> wspt::on("s")
            >> wspt::on("something")

        > wspt::should_be(' ')
            >> wspt::on("foo")
            >> wspt::on("")
    ;

    tester += wspt::it("Many next", wsp::many<wsp::nextc, std::basic_string>) 
        > wspt::should_be(std::string{ "something" })
            >> wspt::on("something")

        > wspt::should_be(std::string{ "s" })
            >> wspt::on("s")

        > wspt::should_be(std::string{ "" })
            >> wspt::on("")
    ;

    tester += wspt::it("Repeat >=5 next", wsp::repeat<wsp::nextc, 5, wsp::open_maximum, std::basic_string>) 
        > wspt::should_be(std::string{ "something" })
            >> wspt::on("something")

        > wspt::should_be(std::string{ "somet" })
            >> wspt::on("somet")

        > wspt::should_fail(wspe::EndOfFile{})
            >> wspt::on("some")
            >> wspt::on("s")
            >> wspt::on("")
    ;


    tester += wspt::it("Repeat between 5 and 6 times next", wsp::repeat<wsp::nextc, 5, 6, std::basic_string>) 
        > wspt::should_be(std::string{ "someth" })
            >> wspt::on("something")

        > wspt::should_be(std::string{ "somet" })
            >> wspt::on("somet")

        > wspt::should_fail(wspe::EndOfFile{})
            >> wspt::on("some")
            >> wspt::on("")
    ;


    tester += wspt::it("Repeat 3 times next", wsp::exact<wsp::nextc, 3, std::basic_string>) 
        > wspt::should_be(std::string{ "som" })
            >> wspt::on("something")
            >> wspt::on("som")

        > wspt::should_fail(wspe::EndOfFile{})
            >> wspt::on("so")
            >> wspt::on("")
    ;


    tester += wspt::it("Some next", wsp::some<wsp::nextc, std::basic_string>) 
        > wspt::should_be(std::string{ "something" })
            >> wspt::on("something")

        > wspt::should_be(std::string{ "s" })
            >> wspt::on("s")

        > wspt::should_fail(wspe::EndOfFile{})
            >> wspt::on("")
    ;

    tester += wspt::it("Filter next", wsp::nextc [ wsp::filter<is_digit> ]) 
        > wspt::should_be('1')
            >> wspt::on("123")
            >> wspt::on("1")

        > wspt::should_fail(wspe::PredicateFailure{})
            >> wspt::on("s")

        > wspt::should_fail(wspe::EndOfFile{})
            >> wspt::on("")
    ;

    tester += wspt::it("Filter then map next", wsp::nextc [ wsp::filter<is_digit> ] [ wsp::map<digit_to_int> ]) 
        > wspt::should_be(1)
            >> wspt::on("123")
            >> wspt::on("1")

        > wspt::should_fail(wspe::PredicateFailure{})
            >> wspt::on("s")

        > wspt::should_fail(wspe::EndOfFile{})
            >> wspt::on("")
    ;

    tester += wspt::it("Handle next's EOF error", wsp::nextc [ wsp::handler<EOF_to_space> ]) 
        > wspt::should_be('s')
            >> wspt::on("s")
            >> wspt::on("something")

        > wspt::should_be(' ')
            >> wspt::on("")
    ;

    tester += wspt::it("Next with default", wsp::nextc [ wsp::or_else<' '> ]) 
        > wspt::should_be('s')
            >> wspt::on("s")
            >> wspt::on("something")

        > wspt::should_be(' ')
            >> wspt::on("")
    ;

    tester += wspt::it("Parse integer in parenthesis", 
        wsp::seq<
            wsp::ch<'('>, 
            wsp::some<wsp::nextc [ wsp::filter<is_digit> ], std::basic_string>, 
            wsp::ch<')'>
        > [ wsp::peek<1> ] [ wsp::map<string_to_int> ]
    ) 
        > wspt::should_be(123456)
            >> wspt::on("(123456)")
            >> wspt::on("(123456) something")

        > wspt::should_fail(wsp::Sum<wspe::NotMatching<'('>, wspe::EndOfFile>{ wspe::NotMatching<'('>{} })
            >> wspt::on("13)")
            >> wspt::on("[13)")

        > wspt::should_fail(wsp::Sum<wspe::NotMatching<'('>, wspe::EndOfFile>{ wspe::EndOfFile{} })
            >> wspt::on("")

        > wspt::should_fail(wsp::Sum<wspe::NotMatching<')'>, wspe::EndOfFile>{ wspe::NotMatching<')'>{} })
            >> wspt::on("(1 ")

        > wspt::should_fail(wsp::Sum<wspe::NotMatching<')'>, wspe::EndOfFile>{ wspe::EndOfFile{} })
            >> wspt::on("(1")
    ;

    tester += wspt::it("Peek", wsp::seq<wsp::nextc, wsp::nextc, wsp::nextc> [ wsp::peek<2> ]) 
        > wspt::should_be('m')
            >> wspt::on("something")
            >> wspt::on("som")

        > wspt::should_fail(wspe::EndOfFile{})
            >> wspt::on("so")
            >> wspt::on("")
    ;

    tester += wspt::it("Peek error", wsp::first<wsp::nextc, wsp::nextc, wsp::nextc> [ wsp::peek_err<2> ]) 
        > wspt::should_be(wsp::Sum<char>('s'))
            >> wspt::on("something")
            >> wspt::on("s")

        > wspt::should_fail(wspe::EndOfFile{})
            >> wspt::on("")
    ;

    tester += wspt::it("Select", wsp::seq<wsp::nextc, wsp::nextc, wsp::nextc> [ wsp::select<2, 1, 2> ]) 
        > wspt::should_be(wsp::Product('m', 'o', 'm'))
            >> wspt::on("something")
            >> wspt::on("som")

        > wspt::should_fail(wspe::EndOfFile{})
            >> wspt::on("so")
            >> wspt::on("")
    ;

    tester += wspt::it("Select error", wsp::first<wsp::nextc, wsp::nextc> [ wsp::select_err<1, 0, 1> ]) 
        > wspt::should_be(wsp::Sum<char>('s'))
            >> wspt::on("something")
            >> wspt::on("s")

        > wspt::should_fail(wsp::Product(wspe::EndOfFile{}, wspe::EndOfFile{}, wspe::EndOfFile{}))
            >> wspt::on("")
    ;

    tester.report();

/*
    debug_t<

    > _;
*/

    static_assert(
        wspd::is_parser_valid_v<wsp::BoundedReader, wsp::NextC> &&
        wspd::is_parser_valid_v<wsp::BoundedReader, wsp::Opt<wsp::NextC>> &&
        wspd::is_parser_valid_v<wsp::BoundedReader, wsp::Seq<wsp::NextC, wsp::NextC>> &&
        wspd::is_parser_valid_v<wsp::BoundedReader, wsp::Opt<wsp::Opt<wsp::NextC>>> &&
        wspd::is_parser_valid_v<wsp::BoundedReader, wsp::Seq<wsp::Seq<wsp::NextC, wsp::NextC>, wsp::NextC>> &&
        wspd::is_parser_valid_v<wsp::BoundedReader, wsp::Seq<wsp::Opt<wsp::NextC>, wsp::Seq<wsp::NextC, wsp::NextC>>> &&
        wspd::is_parser_valid_v<wsp::BoundedReader, wsp::First<decltype(wsp::ch<'a'>), wsp::NextC>> &&
        wspd::is_parser_valid_v<wsp::BoundedReader, wsp::First<wsp::Opt<wsp::NextC>, wsp::Opt<wsp::NextC>>> &&
        wspd::is_parser_valid_v<wsp::BoundedReader, wsp::Transformer<wsp::NextC, char_to_int>> &&
        wspd::is_parser_valid_v<wsp::BoundedReader, wsp::Satisfy<wsp::NextC, is_digit>> &&
        wspd::is_parser_valid_v<wsp::BoundedReader, wsp::Handle<wsp::NextC, EOF_to_space>> &&
        wspd::is_parser_valid_v<wsp::BoundedReader, wsp::Repeat<wsp::NextC, 2, wsp::open_maximum, std::basic_string>> &&
        true, 
        "Something is wrong...");

    return 0;
}

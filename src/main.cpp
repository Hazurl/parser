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
#include <wpr/Test.hpp>
#include <wpr/Readers.hpp>
#include <wpr/Parsers.hpp>
#include <wpr/Containers.hpp>
#include <wpr/Details.hpp>

namespace wpre = wpr::error;
namespace wprd = wpr::details;
namespace wprt = wpr::test;

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
struct GoodParser : wpr::Parser<GoodParser, char, wpre::EndOfFile> {
    template<typename R> static wpr::Result<char, wpre::EndOfFile> parse(R reader) { return wpr::success(reader.cursor, '0'); }
};
static_assert(wprd::is_parser_valid_v<wpr::BoundedReader, GoodParser>);
*/
/*
//    static_assert(parser_details::false_v<T, Es...>, "The parse method's return type should be a Result");
struct ReturnTypeNotResult : wpr::Parser<ReturnTypeNotResult, char, wpre::EndOfFile> {
    template<typename R> static char parse(R reader) { return '0'; }
};
static_assert(wprd::is_parser_valid_v<wpr::BoundedReader, ReturnTypeNotResult>);
*/
/*
//    static_assert(parser_details::false_v<T>, "The parse method's return type is a Result with the wrong success type");
struct ResultSuccessWrong : wpr::Parser<ResultSuccessWrong, char, wpre::EndOfFile> {
    template<typename R> static wpr::Result<int, wpre::EndOfFile> parse(R reader) { return wpr::success(reader.cursor, 1337); }
};
static_assert(wprd::is_parser_valid_v<wpr::BoundedReader, ResultSuccessWrong>);
*/
/*
//    static_assert(parser_details::false_v<Es...>, "The parse method's return type is a Result with the wrong error list");
struct ResultErrorsWrong : wpr::Parser<ResultErrorsWrong, char, wpre::EndOfFile> {
    template<typename R> static wpr::Result<char, wpre::EndOfFile, int> parse(R reader) { return wpr::success(reader.cursor, '0'); }
};
static_assert(wprd::is_parser_valid_v<wpr::BoundedReader, ResultErrorsWrong>);
*/
/*
//    static_assert(parser_details::false_v<T, Es...>, "The parse method's return type is a Result with the wrong success type and error list");
struct ResultSuccessAndErrorsWrong : wpr::Parser<ResultSuccessAndErrorsWrong, char, wpre::EndOfFile> {
    template<typename R> static wpr::Result<int, wpre::EndOfFile, int> parse(R reader) { return wpr::success(reader.cursor, 1337); }
};
static_assert(wprd::is_parser_valid_v<wpr::BoundedReader, ResultSuccessAndErrorsWrong>);
*/
/*
//    static_assert(parser_details::false_v<R>, "The parse method should be static");
struct ParseNotStatic : wpr::Parser<ParseNotStatic, char, wpre::EndOfFile> {
    template<typename R> wpr::Result<char, wpre::EndOfFile> parse(R reader) { return wpr::success(reader.cursor, '0'); }
};
static_assert(wprd::is_parser_valid_v<wpr::BoundedReader, ParseNotStatic>);
*/
/*
//    static_assert(parser_details::false_v<P>, "The parse member is detected but doesn't seems like a static method, makes sure it has the signature: \ntemplate<typename Reader>\nstatic Result<T, Es...> parse(Reader reader)");
struct ParseNotFunction : wpr::Parser<ParseNotFunction, char, wpre::EndOfFile> {
    template<typename R> R parse; 
};
static_assert(wprd::is_parser_valid_v<wpr::BoundedReader, ParseNotFunction>);
*/
/*
//    static_assert(parser_details::false_v<R>, "The parse method should only have one argument, the reader (taken by value)");
struct ParseWithMoreArg : wpr::Parser<ParseWithMoreArg, char, wpre::EndOfFile> {
    template<typename R> static wpr::Result<char, wpre::EndOfFile> parse(R reader, int) { return wpr::success(reader.cursor, '0'); }
};
static_assert(wprd::is_parser_valid_v<wpr::BoundedReader, ParseWithMoreArg>);
*/
/*
//    static_assert(parser_details::false_v<R>, "The parse method isn't detected, makes sure it has the signature: \ntemplate<typename Reader>\nstatic Result<T, Es...> parse(Reader reader)");
struct Typo : wpr::Parser<Typo, char, wpre::EndOfFile> {
    template<typename R> static wpr::Result<char, wpre::EndOfFile> parse_(R reader) { return wpr::success(reader.cursor, '0'); }
};
static_assert(wprd::is_parser_valid_v<wpr::BoundedReader, Typo>);
*/
/*
//    static_assert(parser_details::false_v<P>, "The type you wants to validate doesn't inherit from Parser, to be more exact, the parser_type alias isn't a parser");
struct ParserTypeNotParser : wpr::BoundedReader {
    using parser_type = wpr::BoundedReader;
    template<typename R> static wpr::Result<char, wpre::EndOfFile> parse(R reader) { return wpr::success(reader.cursor, '0'); }
};
static_assert(wprd::is_parser_valid_v<wpr::BoundedReader, ParserTypeNotParser>);
*/
/*
//    static_assert(parser_details::false_v<P>, "The type you wants to validate doesn't inherit from Parser");
struct NoBaseParser {
    template<typename R> static wpr::Result<char, wpre::EndOfFile> parse(R reader) { return wpr::success(reader.cursor, '0'); }
};
static_assert(wprd::is_parser_valid_v<wpr::BoundedReader, NoBaseParser>);
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

    wprt::Tester tester;

    tester += wprt::it("Next", wpr::nextc) 
        > wprt::should_be('s')
            >> wprt::on("s")
            >> wprt::on("something")

        > wprt::should_fail(wpre::EndOfFile{})
            >> wprt::on("")
    ;

    tester += wprt::it("Optional next", wpr::opt<wpr::nextc>) 
        > wprt::should_be(wpr::Maybe<char>{ 's' })
            >> wprt::on("s")
            >> wprt::on("something")

        > wprt::should_be(wpr::Maybe<char>{})
            >> wprt::on("")
    ;

    tester += wprt::it("Double next", wpr::seq<wpr::nextc, wpr::nextc>) 
        > wprt::should_be(wpr::Product('s', 'o'))
            >> wprt::on("so")
            >> wprt::on("something")

        > wprt::should_fail(wpre::EndOfFile{})
            >> wprt::on("")
            >> wprt::on("s")
    ;

    tester += wprt::it("Match 's'", wpr::ch<'s'>) 
        > wprt::should_be('s')
            >> wprt::on("s")
            >> wprt::on("something")

        > wprt::should_fail(wpre::NotMatching<'s'>{})
            >> wprt::on("z")

        > wprt::should_fail(wpre::EndOfFile{})
            >> wprt::on("")
    ;

    tester += wprt::it("Match 's'", wpr::first<wpr::ch<'s'>, wpr::nextc>) 
        > wprt::should_be(wpr::Sum<char>('s'))
            >> wprt::on("s")
            >> wprt::on("something")

        > wprt::should_be(wpr::Sum<char>('a'))
            >> wprt::on("a")
            >> wprt::on("another")

        > wprt::should_fail(wpr::Product<wpr::Sum<wpre::NotMatching<'s'>, wpre::EndOfFile>, wpre::EndOfFile>{
            wpr::Sum<wpre::NotMatching<'s'>, wpre::EndOfFile>{ wpre::EndOfFile{} },
            wpre::EndOfFile{}
        })
            >> wprt::on("")
    ;

    tester += wprt::it("Match 'a' or 's'", wpr::first<wpr::ch<'s'>, wpr::ch<'a'>>) 
        > wprt::should_be(wpr::Sum<char>('s'))
            >> wprt::on("s")
            >> wprt::on("something")

        > wprt::should_be(wpr::Sum<char>('a'))
            >> wprt::on("a")
            >> wprt::on("another")

        > wprt::should_fail(wpr::Product<wpr::Sum<wpre::NotMatching<'s'>, wpre::EndOfFile>, wpr::Sum<wpre::NotMatching<'a'>, wpre::EndOfFile>>(
            wpr::Sum<wpre::NotMatching<'s'>, wpre::EndOfFile>{ wpre::NotMatching<'s'>{} },
            wpr::Sum<wpre::NotMatching<'a'>, wpre::EndOfFile>{ wpre::NotMatching<'a'>{} }
        ))
            >> wprt::on("foo")

        > wprt::should_fail(wpr::Product<wpr::Sum<wpre::NotMatching<'s'>, wpre::EndOfFile>, wpr::Sum<wpre::NotMatching<'a'>, wpre::EndOfFile>>(
            wpr::Sum<wpre::NotMatching<'s'>, wpre::EndOfFile>{ wpre::EndOfFile{} },
            wpr::Sum<wpre::NotMatching<'a'>, wpre::EndOfFile>{ wpre::EndOfFile{} }
        ))
            >> wprt::on("")
    ;

    tester += wprt::it("Map next", wpr::nextc [ wpr::map<char_to_int> ]) 
        > wprt::should_be(char_to_int('s'))
            >> wprt::on("s")
            >> wprt::on("something")

        > wprt::should_fail(wpre::EndOfFile{})
            >> wprt::on("")
    ;

    tester += wprt::it("Map optional next", wpr::opt<wpr::ch<'s'>> [ wpr::map<default_maybe> ]) 
        > wprt::should_be('s')
            >> wprt::on("s")
            >> wprt::on("something")

        > wprt::should_be(' ')
            >> wprt::on("foo")
            >> wprt::on("")
    ;

    tester += wprt::it("Many next", wpr::many<wpr::nextc, std::basic_string>) 
        > wprt::should_be(std::string{ "something" })
            >> wprt::on("something")

        > wprt::should_be(std::string{ "s" })
            >> wprt::on("s")

        > wprt::should_be(std::string{ "" })
            >> wprt::on("")
    ;

    tester += wprt::it("Repeat >=5 next", wpr::repeat<wpr::nextc, 5, wpr::open_maximum, std::basic_string>) 
        > wprt::should_be(std::string{ "something" })
            >> wprt::on("something")

        > wprt::should_be(std::string{ "somet" })
            >> wprt::on("somet")

        > wprt::should_fail(wpre::EndOfFile{})
            >> wprt::on("some")
            >> wprt::on("s")
            >> wprt::on("")
    ;


    tester += wprt::it("Repeat between 5 and 6 times next", wpr::repeat<wpr::nextc, 5, 6, std::basic_string>) 
        > wprt::should_be(std::string{ "someth" })
            >> wprt::on("something")

        > wprt::should_be(std::string{ "somet" })
            >> wprt::on("somet")

        > wprt::should_fail(wpre::EndOfFile{})
            >> wprt::on("some")
            >> wprt::on("")
    ;


    tester += wprt::it("Repeat 3 times next", wpr::exact<wpr::nextc, 3, std::basic_string>) 
        > wprt::should_be(std::string{ "som" })
            >> wprt::on("something")
            >> wprt::on("som")

        > wprt::should_fail(wpre::EndOfFile{})
            >> wprt::on("so")
            >> wprt::on("")
    ;


    tester += wprt::it("Some next", wpr::some<wpr::nextc, std::basic_string>) 
        > wprt::should_be(std::string{ "something" })
            >> wprt::on("something")

        > wprt::should_be(std::string{ "s" })
            >> wprt::on("s")

        > wprt::should_fail(wpre::EndOfFile{})
            >> wprt::on("")
    ;

    tester += wprt::it("Filter next", wpr::nextc [ wpr::filter<is_digit> ]) 
        > wprt::should_be('1')
            >> wprt::on("123")
            >> wprt::on("1")

        > wprt::should_fail(wpre::PredicateFailure{})
            >> wprt::on("s")

        > wprt::should_fail(wpre::EndOfFile{})
            >> wprt::on("")
    ;

    tester += wprt::it("Filter then map next", wpr::nextc [ wpr::filter<is_digit> ] [ wpr::map<digit_to_int> ]) 
        > wprt::should_be(1)
            >> wprt::on("123")
            >> wprt::on("1")

        > wprt::should_fail(wpre::PredicateFailure{})
            >> wprt::on("s")

        > wprt::should_fail(wpre::EndOfFile{})
            >> wprt::on("")
    ;

    tester += wprt::it("Handle next's EOF error", wpr::nextc [ wpr::handler<EOF_to_space> ]) 
        > wprt::should_be('s')
            >> wprt::on("s")
            >> wprt::on("something")

        > wprt::should_be(' ')
            >> wprt::on("")
    ;

    tester += wprt::it("Next with default", wpr::nextc [ wpr::or_else<' '> ]) 
        > wprt::should_be('s')
            >> wprt::on("s")
            >> wprt::on("something")

        > wprt::should_be(' ')
            >> wprt::on("")
    ;

    tester += wprt::it("Parse integer in parenthesis", 
        wpr::seq<
            wpr::ch<'('>, 
            wpr::some<wpr::nextc [ wpr::filter<is_digit> ], std::basic_string>, 
            wpr::ch<')'>
        > [ wpr::peek<1> ] [ wpr::map<string_to_int> ]
    ) 
        > wprt::should_be(123456)
            >> wprt::on("(123456)")
            >> wprt::on("(123456) something")

        > wprt::should_fail(wpr::Sum<wpre::NotMatching<'('>, wpre::EndOfFile>{ wpre::NotMatching<'('>{} })
            >> wprt::on("13)")
            >> wprt::on("[13)")

        > wprt::should_fail(wpr::Sum<wpre::NotMatching<'('>, wpre::EndOfFile>{ wpre::EndOfFile{} })
            >> wprt::on("")

        > wprt::should_fail(wpr::Sum<wpre::NotMatching<')'>, wpre::EndOfFile>{ wpre::NotMatching<')'>{} })
            >> wprt::on("(1 ")

        > wprt::should_fail(wpr::Sum<wpre::NotMatching<')'>, wpre::EndOfFile>{ wpre::EndOfFile{} })
            >> wprt::on("(1")
    ;

    tester += wprt::it("Peek", wpr::seq<wpr::nextc, wpr::nextc, wpr::nextc> [ wpr::peek<2> ]) 
        > wprt::should_be('m')
            >> wprt::on("something")
            >> wprt::on("som")

        > wprt::should_fail(wpre::EndOfFile{})
            >> wprt::on("so")
            >> wprt::on("")
    ;

    tester += wprt::it("Peek error", wpr::first<wpr::nextc, wpr::nextc, wpr::nextc> [ wpr::peek_err<2> ]) 
        > wprt::should_be(wpr::Sum<char>('s'))
            >> wprt::on("something")
            >> wprt::on("s")

        > wprt::should_fail(wpre::EndOfFile{})
            >> wprt::on("")
    ;

    tester += wprt::it("Select", wpr::seq<wpr::nextc, wpr::nextc, wpr::nextc> [ wpr::select<2, 1, 2> ]) 
        > wprt::should_be(wpr::Product('m', 'o', 'm'))
            >> wprt::on("something")
            >> wprt::on("som")

        > wprt::should_fail(wpre::EndOfFile{})
            >> wprt::on("so")
            >> wprt::on("")
    ;

    tester += wprt::it("Select error", wpr::first<wpr::nextc, wpr::nextc> [ wpr::select_err<1, 0, 1> ]) 
        > wprt::should_be(wpr::Sum<char>('s'))
            >> wprt::on("something")
            >> wprt::on("s")

        > wprt::should_fail(wpr::Product(wpre::EndOfFile{}, wpre::EndOfFile{}, wpre::EndOfFile{}))
            >> wprt::on("")
    ;

    tester.report();

/*
    debug_t<

    > _;
*/

    static_assert(
        wprd::is_parser_valid_v<wpr::BoundedReader, wpr::NextC> &&
        wprd::is_parser_valid_v<wpr::BoundedReader, wpr::Opt<wpr::NextC>> &&
        wprd::is_parser_valid_v<wpr::BoundedReader, wpr::Seq<wpr::NextC, wpr::NextC>> &&
        wprd::is_parser_valid_v<wpr::BoundedReader, wpr::Opt<wpr::Opt<wpr::NextC>>> &&
        wprd::is_parser_valid_v<wpr::BoundedReader, wpr::Seq<wpr::Seq<wpr::NextC, wpr::NextC>, wpr::NextC>> &&
        wprd::is_parser_valid_v<wpr::BoundedReader, wpr::Seq<wpr::Opt<wpr::NextC>, wpr::Seq<wpr::NextC, wpr::NextC>>> &&
        wprd::is_parser_valid_v<wpr::BoundedReader, wpr::First<decltype(wpr::ch<'a'>), wpr::NextC>> &&
        wprd::is_parser_valid_v<wpr::BoundedReader, wpr::First<wpr::Opt<wpr::NextC>, wpr::Opt<wpr::NextC>>> &&
        wprd::is_parser_valid_v<wpr::BoundedReader, wpr::Transformer<wpr::NextC, char_to_int>> &&
        wprd::is_parser_valid_v<wpr::BoundedReader, wpr::Satisfy<wpr::NextC, is_digit>> &&
        wprd::is_parser_valid_v<wpr::BoundedReader, wpr::Handle<wpr::NextC, EOF_to_space>> &&
        wprd::is_parser_valid_v<wpr::BoundedReader, wpr::Repeat<wpr::NextC, 2, wpr::open_maximum, std::basic_string>> &&
        true, 
        "Something is wrong...");

    return 0;
}

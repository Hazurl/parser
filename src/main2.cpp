#include <iostream>
#include <cctype>
#include <optional>
#include <sstream>
#include <variant>
#include <type_traits>
#include <experimental/type_traits>

#include <module/module.h>
#include <json.hpp>
#include <ws/parser2/ParserInternal.hpp>
#include <ws/parser2/Storage.hpp>
#include <ws/parser2/Details.hpp>

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
};

template<typename P, auto& F>
struct Transformer;

template<typename...>
struct Show;

template<typename P, typename T>
struct IParser {
    using parsed_type = T;

    template<typename F>
    Transformer<P, F::func> operator[] (F) const;
};

template<typename P, typename>
struct is_parser {
    static inline constexpr bool value{ false };
};

template<typename P>
struct is_parser<P, std::void_t<typename P::parsed_type>> {
    static inline constexpr bool value{ true };
    using parsed_type = typename P::parsed_type;
};

template<typename P>
static inline constexpr bool is_parser_v{ is_parser<P, void>::value };
template<typename P>
using parsed_type_t = typename P::parsed_type;

template<typename C>
struct GetC : IParser<GetC<C>, C> {
    template<typename R>
    static C parse(R& r) {
        auto c = r.get();
        r.next();
        return c;
    }
};

using Get = GetC<char>;

struct GetNotWhiteSpace : IParser<GetNotWhiteSpace, char> {
    template<typename R>
    static char parse(R& r) {
        auto c = Get::parse(r);
        return c == ' ' ? parse(r) : c;
    } 
};

struct Digit : IParser<Digit, int> {
    template<typename R>
    static int parse(R& r);
};

struct ParenthizedExpr : IParser<ParenthizedExpr, int> {
    template<typename R>
    static int parse(R& r) {
        if (r.get() == '(') {
            Get::parse(r);

            auto ret = ParenthizedExpr::parse(r);

            if (Get::parse(r) != ')') throw std::runtime_error("parenthese not closed...");        

            return ret;    
        }

        return Digit::parse(r);
    }
};

template<typename R>
int Digit::parse(R& r) {
    auto c = Get::parse(r);
    if (c < '0' || c > '9') {
        throw std::runtime_error("not a digit ...");
    } 

    return c - '0';
}

template<typename A, typename B>
struct Seq : IParser<Seq<A, B>, std::pair< parsed_type_t<A>, parsed_type_t<B> > > {

    static_assert(is_parser_v<A> && is_parser_v<B>, "Both parameters of Seq must be parsers (inherit from IParser)");

    using parsed_type = parsed_type_t<Seq<A, B>>;

    template<typename R>
    static parsed_type parse(R& r) {
        auto a = A::parse(r);
        auto b = B::parse(r);
        return parsed_type{ std::move(a), std::move(b) };
    }
};

using Get2 = Seq<Get, Get>;

template<typename A, typename B, typename = std::enable_if_t<is_parser_v<A> && is_parser_v<B>>>
Seq<A, B> operator & (A, B) {
    return Seq<A, B>{};
}

using Get2ParenthisedExpr = decltype(
    ParenthizedExpr{} & ParenthizedExpr{}
); 

template<auto& Pred>
struct Satisfy : IParser<Satisfy<Pred>, char> {
    template<typename R>
    static char parse(R& r) {
        auto c = Get::parse(r);
        if (!Pred(c)) {
            throw std::runtime_error("Current character does not follow the predicate");
        }

        return c;
    }
};

template<typename P, auto& F>
struct Transformer : IParser<Transformer<P, F>, std::invoke_result_t<decltype(F), parsed_type_t<P>>> {
    static_assert(is_parser_v<P>, "First parameter of Transformer must be a parser");
    static_assert(std::is_invocable_v<decltype(F), parsed_type_t<P>>, "Function is not invokable with the parser's return type");

    template<typename R>
    static std::invoke_result_t<decltype(F), parsed_type_t<P>> parse(R& r) {
        return F(P::parse(r));
    }
};

bool is_digit(char c) { return c >= '0' && c <= '9'; }
int digit_to_int(char c) { return c - '0'; }

using Digit_ = Transformer<Satisfy<is_digit>, digit_to_int>;

template<char c>
struct Char : IParser<Char<c>, char> {
    static inline constexpr bool satisfy(char x) { return x == c; }

    template<typename R>
    static char parse(R& r) {
        return Satisfy<Char::satisfy>::parse(r);
    }
};

template<typename A, typename B>
struct Left : IParser<Left<A, B>, parsed_type_t<A>> {
    template<typename R>
    static parsed_type_t<A> parse(R& r) {
        return Seq<A, B>::parse(r).first;
    }
};

template<typename A, typename B>
struct Right : IParser<Right<A, B>, parsed_type_t<B>> {
    template<typename R>
    static parsed_type_t<B> parse(R& r) {
        return Seq<A, B>::parse(r).second;
    }
};

template<typename A, typename B, typename = std::enable_if_t<is_parser_v<A> && is_parser_v<B>>>
auto operator << (A, B) {
    return Left<A, B>{};
}

template<typename A, typename B, typename = std::enable_if_t<is_parser_v<A> && is_parser_v<B>>>
auto operator >> (A, B) {
    return Right<A, B>{};
}

struct ParenthizedExpr_ : IParser<ParenthizedExpr_, int> {

    static constexpr char title[] = "Parenthized expression";

    template<typename R>
    static int parse(R& r) {
        if (r.get() == '(') {
            return decltype(Char<'('>{} >> ParenthizedExpr_{} << Char<')'>{})::parse(r);
        }

        return Digit_::parse(r);
    }
};

template<typename A, typename B>
struct Either : IParser<Either<A, B>, std::variant<parsed_type_t<A>, parsed_type_t<B>> > {
    using parsed_type = std::variant<parsed_type_t<A>, parsed_type_t<B>>;

    template<typename R>
    static parsed_type parse(R& r) {
        auto r_backup = r;
        try {
            return parsed_type(std::in_place_index_t<0>{}, A::parse(r));
        } catch(...) {
            r = std::move(r_backup);
            return parsed_type(std::in_place_index_t<1>{}, B::parse(r));
        }
    }
};

template<typename A, typename B>
struct JoinedEither : IParser<JoinedEither<A, B>, parsed_type_t<A>> {

    static_assert(std::is_same_v<parsed_type_t<A>, parsed_type_t<B>>, "JoinedEither requires 2 parsers with the same return type");

    template<typename R>
    static parsed_type_t<A> parse(R& r) {
        auto r_backup = r;
        try {
            return A::parse(r);
        } catch(...) {
            r = std::move(r_backup);
            return B::parse(r);
        }
    }
};

template<typename A, typename B, typename = std::enable_if_t<is_parser_v<A> && is_parser_v<B>>>
auto operator | (A, B) {
    return JoinedEither<A, B>{};
}

struct ParenthizedExpr__ : IParser<ParenthizedExpr__, int> {
    template<typename R>
    static int parse(R& r);
};

using ParenthizedExpr___recursive_parser = decltype(
    Char<'('>{} >> ParenthizedExpr__{} << Char<')'>{}
|   Digit_{}
);

template<typename R>
int ParenthizedExpr__::parse(R& r) {
    return ParenthizedExpr___recursive_parser::parse(r);
}

#define CAT_(x, y) x ## y
#define CAT(x, y) CAT_(x, y)

#define declare_parser(name, ret...) \
struct CAT(name, _p) : IParser<CAT(name, _p), ret> { \
    template<typename R> \
    static ret parse(R& r); \
}; \
static inline constexpr CAT(name, _p) name{};

#define define_parser(name, value...) \
using CAT(name, _rec_parser) = decltype( value ); \
template<typename R> \
int CAT(name, _p)::parse(R& r) { \
    return CAT(name, _rec_parser)::parse(r); \
}

template<auto& F>
struct Func {
    static inline constexpr auto& func = F;
};

template<typename P, typename R>
template<typename F>
Transformer<P, F::func> IParser<P, R>::operator[] (F) const {
    return Transformer<P, F::func>{};
}

template<auto& Pred>
Satisfy<Pred> satisfy;

template<auto& F>
Func<F> func;

template<char c>
Char<c> char_;


declare_parser(test, int);
define_parser(test, 
    char_<'('> >> test << char_<')'>
|   satisfy<is_digit> [ func<digit_to_int> ]
);

template<typename P>
struct Logger : IParser<Logger<P>, parsed_type_t<P>> {
    static_assert(is_parser_v<P>, "Logger's Parameter must be a parser");

    template<typename R>
    static parsed_type_t<P> parse(R& r) {
        try {
            std::cout << "Parsing " << P::title << "... ";
            auto ret = P::parse(r);
            std::cout << "successfully!\n";
            return ret;
        } catch(...) {
            std::cout << "but failed!\n";
            throw;
        }
    }
};


static const char something_str[] = "something";

struct NewGet : ws::parser2::Parser<NewGet, char> {
    template<typename R>
    static ws::parser2::Result<char> parse(R r) {
        auto c = r.get();
        //r.next();
        return {std::in_place_index_t<0>{}, r.cursor, ws::parser2::Success<char>{ c }};
    }
};

static_assert(ws::parser2::details::is_parser_v<NewGet, StringReader>, "...");


int main() {
    std::cout << '\n';
    {
        auto reader = StringReader{ std::string_view{ "something" } };
        std::cout << "'" << Get::parse(reader) << "'" << '\n';
    }
    {
        auto reader = StringReader{ std::string_view{ "     something" } };
        std::cout << "'" << GetNotWhiteSpace::parse(reader) << "'" << '\n';
    }
    {
        auto reader = StringReader{ std::string_view{ "(((2)))" } };
        std::cout << ParenthizedExpr::parse(reader) << '\n';
    }
    {
        auto reader = StringReader{ std::string_view{ "something" } };
        auto ret = Get2::parse(reader);
        std::cout << ret.first << ", " << ret.second << '\n';
    }
    {
        auto reader = StringReader{ std::string_view{ "(((2)))((((((5))))))" } };
        auto ret = Get2ParenthisedExpr::parse(reader);
        std::cout << ret.first << ", " << ret.second << '\n';
    }
    {
        auto reader = StringReader{ std::string_view{ "(((2)))" } };
        std::cout << ParenthizedExpr_::parse(reader) << '\n';
    }
    {
        auto reader = StringReader{ std::string_view{ "(((2)))" } };
        std::cout << Logger<ParenthizedExpr_>::parse(reader) << '\n';
    }
    {
        auto reader = StringReader{ std::string_view{ "(((2)))(((2)))" } };
        std::cout << ParenthizedExpr__::parse(reader) << '\n';
    }
    {
        auto reader = StringReader{ std::string_view{ "(((2)))(((2)))" } };
        std::cout << test_p::parse(reader) << '\n';
    }

    std::cout << '\n';

    static_assert(ws::parser2::details::is_parser_v<ws::parser2::Next<char>, StringReader>, "...");

    {
        auto reader = StringReader{ std::string_view{ "something" } };
        auto ret = ws::parser2::Next<char>::parse(reader);
        if (ret.is_error()) {
            std::cout << "Error: " << ret.what() << '\n';
        } else {
            std::cout << "'" << ret.success() << "'" << '\n';
        }
    }
    {
        auto reader = StringReader{ std::string_view{ "" } };
        auto ret = ws::parser2::Next<char>::parse(reader);
        if (ret.is_error()) {
            std::cout << "Error: " << ret.what() << '\n';
        } else {
            std::cout << "'" << ret.success() << "'" << '\n';
        }
    }
    {
        auto reader = StringReader{ std::string_view{ "" } };
        
        using namespace ws::parser2;

        auto ret = decltype(opt<nextc>)::parse(reader);
        if (ret.is_error()) {
            std::cout << "Error: " << ret.what() << '\n';
        } else {
            if (ret.success().has_value())
                std::cout << "'" << ret.success().value() << "'" << '\n';
            else
                std::cout << "None\n";
        }
    }
    {
        auto reader = StringReader{ std::string_view{ "a" } };
        
        using namespace ws::parser2;

        auto ret = decltype(opt<nextc>)::parse(reader);
        if (ret.is_error()) {
            std::cout << "Error: " << ret.what() << '\n';
        } else {
            if (ret.success().has_value())
                std::cout << "'" << ret.success().value() << "'" << '\n';
            else
                std::cout << "None\n";
        }
    }



    return 0;
}

#pragma once

#include <ws/parser2/Containers.hpp>
#include <ws/parser2/Details.hpp>
#include <ws/parser2/parsers/Errors.hpp>

#include <iostream>

namespace ws::parser2 {

/*
    First parser
 */

// TODO: special case where one of the parser can't fail, making the First pasrer always successful
template<typename...Ps>
struct First : std::enable_if_t<(true && ... && details::is_parser_soft_check_v<Ps>), 
    Parser<
        First<Ps...>, 
        details::list_to_t<details::set_t<details::List<details::parsed_type_t<Ps>...>>, Sum>, 
        /* Parser<_, Es...>... => Product<Sum<Es...>...> */
        /* Get the product of all sums */
        Product<
            /* Wrap the errors of Ps in a Sum */ 
            details::list_to_t<
                /* get all errors from Ps */ 
                details::list_from_errors_t<
                    details::result_type_t<Ps>
                >, 
                Sum
            >
            ...
        >
    >> {

private:

    static_assert((true && ... && details::is_parser_soft_check_v<Ps>), "First requires parser-only parameters");

    using result_t = details::result_type_t<First<Ps...>>;
    using error_t = details::at_t<details::list_from_errors_t<result_t>, 0>;

public:

    template<typename R>
    static result_t parse(R reader) {
        return parse_at<0>(reader);
    }

private:

    template<std::size_t I, typename R, typename...Rs>
    static std::enable_if_t<(I >= sizeof...(Ps)), result_t> 
    parse_at(R reader, Rs&&... rs) {
        return fail<error_t>(reader.cursor(), Product(std::move(rs)...));
    }

    template<std::size_t I, typename R, typename...Rs>
    static std::enable_if_t<(I < sizeof...(Ps)), result_t> 
    parse_at(R reader, Rs&&... rs) {
        using P = details::at_t<details::List<Ps...>, I>;
        auto result = P::parse(reader);

        if constexpr (P::can_fail) {
            if(result.is_error()) {
                return parse_at<I+1>(
                    R::from_cursor(std::move(reader), result.cursor), 
                    std::move(rs)..., 
                    Sum(from_variant_t{}, std::move(result).errors())
                );
            }
        }

        return success(result.cursor, std::move(result.success()));

    }
};






/*
    Parser as value
 */

template<auto&...ps>
constexpr First<std::decay_t<decltype(ps)>...> first;


}
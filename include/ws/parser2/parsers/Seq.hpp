#pragma once

#include <ws/parser2/Containers.hpp>
#include <ws/parser2/Details.hpp>
#include <ws/parser2/parsers/Errors.hpp>

#include <iostream>

namespace ws::parser2 {

/*
    Seq parser
 */


template<typename...>
struct Show;

template<typename...Ps>
struct Seq : std::enable_if_t<(true && ... && details::is_parser_soft_check_v<Ps>), 
    details::parser_from_list_t<
        Seq<Ps...>, 
        Product<details::parsed_type_t<Ps>...>, 
        details::flatten_unique_t<details::List<details::list_from_errors_t<details::result_type_t<Ps>>...>>
    >> {

private:

    static_assert((true && ... && details::is_parser_soft_check_v<Ps>), "Seq requires parser-only parameters");

    using product_t = Product<details::parsed_type_t<Ps>...>;
    using result_t = details::result_type_t<Seq<Ps...>>;
    //details::list_to_t<details::push_t<Product<details::parsed_type_t<Ps>...>, details::flatten_unique_t<details::List<details::list_from_errors_t<details::result_type_t<Ps>>...>>>, Result>;

    //Show<tmp_tuple_t, product_t, result_t> ___;

public:

    template<typename R>
    static result_t parse(R reader) {
        return parse_at<0>(reader);
    }

private:

    template<std::size_t I, typename R, typename...Rs>
    static std::enable_if_t<(I >= sizeof...(Ps)), result_t> 
    parse_at(R reader, Rs&&... rs) {
        return success(reader.cursor(), std::move(rs)...);
    }

    template<std::size_t I, typename R, typename...Rs>
    static std::enable_if_t<(I < sizeof...(Ps)), result_t> 
    parse_at(R reader, Rs&&... rs) {
        using P = details::at_t<details::List<Ps...>, I>;
        auto result = P::parse(reader);

        if constexpr (P::can_fail) {
            if(result.is_error()) {
                return std::visit([cursor = result.cursor] (auto&& e) -> result_t {
                    return fail<std::decay_t<decltype(e)>>(cursor, std::move(e));
                }, std::move(result).errors());
            }
        }

        return parse_at<I+1>(
            R::from_cursor(std::move(reader), result.cursor), 
            std::move(rs)..., 
            std::move(std::move(result.success()))
        );

    }


};






/*
    Parser as value
 */

template<auto&...ps>
constexpr Seq<std::decay_t<decltype(ps)>...> seq;


}
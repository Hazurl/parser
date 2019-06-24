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

    using tmp_tuple_t = std::tuple<std::optional<details::parsed_type_t<Ps>>...>;
    using product_t = Product<details::parsed_type_t<Ps>...>;
    using result_t = details::result_type_t<Seq<Ps...>>;
    //details::list_to_t<details::push_t<Product<details::parsed_type_t<Ps>...>, details::flatten_unique_t<details::List<details::list_from_errors_t<details::result_type_t<Ps>>...>>>, Result>;

    //Show<tmp_tuple_t, product_t, result_t> ___;

public:

    template<typename R>
    static result_t parse(R reader) {
        tmp_tuple_t tmp_res;
        return parse_at<0>(tmp_res, reader);
    }

private:

    template<std::size_t I, typename R>
    static result_t parse_at(tmp_tuple_t& tmp_res, R reader) {
        if constexpr (I < sizeof...(Ps)) {
            using P = details::at_t<details::List<Ps...>, I>;
            auto res = P::parse(reader);

            return std::visit([&tmp_res, reader, &res] (auto&& err) -> details::result_type_t<Seq<Ps...>> {
                using Err = std::decay_t<decltype(err)>;

                if constexpr (std::is_same_v<Success<details::parsed_type_t<P>>, Err>) {

                    std::get<I>(tmp_res) = std::move(err.value);
                    return Seq<Ps...>::parse_at<I+1>(tmp_res, reader.copy_with_cursor(res.cursor + 1));

                } else {

                    return fail<Err>(reader.cursor, std::move(err));
                }
            }, res.value);
        } else {
            return success(reader.cursor, transform_tmp_tuple(tmp_res, std::make_index_sequence<sizeof...(Ps)>{}));
        }
    }

    template<std::size_t...Is>
    static product_t transform_tmp_tuple(tmp_tuple_t& tmp, std::index_sequence<Is...>) {
        return product_t(
            std::move(*std::get<Is>(tmp))...
        );
    }

};






/*
    Parser as value
 */

template<auto&...ps>
constexpr Seq<std::decay_t<decltype(ps)>...> seq;


}
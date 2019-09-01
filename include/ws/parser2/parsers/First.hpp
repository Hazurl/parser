#pragma once

#include <ws/parser2/Containers.hpp>
#include <ws/parser2/Details.hpp>
#include <ws/parser2/parsers/Errors.hpp>

#include <iostream>

namespace ws::parser2 {

template<typename...Ps>
struct First;

namespace {

template<typename P>
using error_in_list_t = 
    details::list_from_errors_t<
    details::result_type_t<P>>;

template<typename...Ps>
using list_of_list_of_error_t = 
    details::List<error_in_list_t<Ps>...>;

template<typename L>
struct IsNotEmpty { static inline constexpr bool value{ !details::is_empty_v<L> }; };

template<typename...Ps>
using list_of_list_of_error_while_not_empty_t = 
    details::take_while_t<IsNotEmpty, list_of_list_of_error_t<Ps...>>;

template<typename...Ps>
using list_of_errors_t = 
    details::flatten_t<list_of_list_of_error_while_not_empty_t<Ps...>>;

template<typename...Ps>
using product_of_errors = 
    details::list_to_t<list_of_errors_t<Ps...>, Product>;

template<typename...Ps>
using product_of_all_errors = 
    details::list_to_t<details::flatten_t<list_of_list_of_error_t<Ps...>>, Product>;




template<typename P>
using results_t = 
    details::parsed_type_t<P>;

template<typename...Ps>
using list_of_results_t = 
    details::set_t<details::List<results_t<Ps>...>>;

template<typename...Ps>
using sum_of_all_results_t = 
    details::list_to_t<list_of_results_t<Ps...>, Sum>;

template<std::size_t N, typename...Ps>
using sum_of_n_results_t = 
    details::list_to_t<details::take_n_t<N, list_of_results_t<Ps...>>, Sum>;



template<typename...Ps>
inline constexpr bool all_can_fail = (true && ... && Ps::can_fail);


/*
    Given parser A, B and C
    First<A, B, C>
    result in:
        - if A, B or C can't fail then:
            Success 
                | C can't fail = Sum<Sa, Sb, Sc>
                | B can't fail = Sum<Sa, Sb>
                | A can't fail = Sum<Sa>
            Error = None
        - if A, B and C can fail then:
            Success = Sum<Sa, Sb, Sc>
            Error = Product<Ea, Eb, Ec>
*/
template<typename...Ps>
using first_parser_t = 
    std::conditional_t<all_can_fail<Ps...>, 
        Parser<
            First<Ps...>, 
            sum_of_all_results_t<Ps...>, 
            product_of_all_errors<Ps...>
        >,
        Parser<
            First<Ps...>, 
            sum_of_n_results_t<details::length_v<list_of_errors_t<Ps...>> + 1 /* because the last parser can't fail */, Ps...>
        >
    >;

}

/*
    First parser
 */


template<typename Pa, typename...Ps>
struct FirstWithError : Pa {
public:

    using result_t = details::result_type_t<Pa>;

    template<typename R>
    static result_t parse(R reader) {
        return parse_at<0>(reader);
    }

private:

    template<std::size_t I, typename R, typename...Rs>
    static std::enable_if_t<(I >= sizeof...(Ps)), result_t> 
    parse_at(R reader, Rs&&... rs) {
        return fail(reader.cursor(), Product(std::move(rs)...));
    }

    template<std::size_t I, typename R, typename...Rs>
    static std::enable_if_t<(I < sizeof...(Ps)), result_t> 
    parse_at(R reader, Rs&&... rs) {
        using P = details::at_t<details::List<Ps...>, I>;
        auto result = P::parse(reader);

        if constexpr (P::can_fail) {
            if(result.is_error()) {
                return parse_at<I+1>(
                    reader, 
                    std::move(rs)..., 
                    std::move(result).error()
                );
            }
        }

        return success(result.cursor(), std::move(result.success()));

    }
};




template<typename Pa, typename...Ps>
struct FirstWithNoError : Pa {
public:

    using result_t = details::result_type_t<Pa>;

    template<typename R>
    static result_t parse(R reader) {
        return parse_at<0>(reader);
    }

private:

    // The last Parser will never fail
    template<std::size_t I, typename R>
    static std::enable_if_t<(I >= sizeof...(Ps) - 1), result_t> 
    parse_at(R reader) {
        using P = details::at_t<details::List<Ps...>, I>;
        auto result = P::parse(reader);
        return success(result.cursor(), std::move(result.success()));
    }

    template<std::size_t I, typename R>
    static std::enable_if_t<(I < sizeof...(Ps) - 1), result_t> 
    parse_at(R reader) {
        using P = details::at_t<details::List<Ps...>, I>;
        auto result = P::parse(reader);

        if(result.is_error()) {
            return parse_at<I+1>(reader);
        }

        return success(result.cursor(), std::move(result.success()));

    }
};



template<typename...Ps>
struct First : 
    std::enable_if_t<
        (true && ... && details::is_parser_soft_check_v<Ps>) && (sizeof...(Ps) > 0),
        std::conditional_t<
            all_can_fail<Ps...>, 
            FirstWithError<first_parser_t<Ps...>, Ps...>, 
            FirstWithNoError<first_parser_t<Ps...>, Ps...>
        >
    > {

private:

    static_assert((true && ... && details::is_parser_soft_check_v<Ps>), "First requires parser-only parameters");
    static_assert(sizeof...(Ps) > 0, "First parser require at least one parser");

};





/*
    Parser as value
 */

template<auto...ps>
constexpr First<std::decay_t<decltype(ps)>...> first;


}
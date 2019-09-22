#pragma once

#include <ws/parser2/Containers.hpp>
#include <ws/parser2/Details.hpp>
#include <ws/parser2/parsers/Errors.hpp>

#include <iostream>

namespace ws::parser2 {

template<typename P, typename...Bs>
struct Switcher;

template<auto& F, typename P>
struct Branch {
    static constexpr auto& predicate = F;
    using parser_t = P;
};

namespace {

template<auto v>
bool equal_predicate(std::decay_t<decltype(v)> const& t) {
    return v == t;
}

template<typename B>
struct IsBranch {
    static constexpr bool value = false;
};

template<auto& F, typename P>
struct IsBranch<Branch<F, P>> {
    static constexpr bool value = true;
};

template<typename T>
bool always_true_predicate(T const&) { return true; }

template<typename T>
struct IsNotBranch {
    static constexpr auto value = !IsBranch<T>::value;
};

template<typename T>
static constexpr bool is_not_branch_v = IsNotBranch<T>::value;

template<typename S, typename B>
struct BranchGetPredicate {
    static constexpr auto value = always_true_predicate<S>;
};

template<typename S, auto& F, typename P>
struct BranchGetPredicate<S, Branch<F, P>> {
    static constexpr auto value = F;
};

template<typename S, typename B>
static constexpr auto branch_get_predicate_v = BranchGetPredicate<S, B>::value;

template<typename B>
struct BranchGetParser {
    using type = B;
};

template<auto& F, typename P>
struct BranchGetParser<Branch<F, P>> {
    using type = P;
};

template<bool cond, typename E, typename L>
using push_unique_if_t = std::conditional_t<cond, details::push_unique_t<E, L>, L>;

template<typename B>
using branch_get_parser_t = typename BranchGetParser<B>::type;

template<typename B>
using branch_get_maybe_error_t = details::list_from_errors_t<details::result_type_t<branch_get_parser_t<B>>>;

template<typename...Bs>
using branch_get_error_list_t = details::flatten_unique_t<details::List<branch_get_maybe_error_t<Bs>...>>;

template<typename S, typename B>
static constexpr bool is_valid_branch_v = 
    details::is_parser_soft_check_v<branch_get_parser_t<B>> && 
    std::is_convertible_v<std::invoke_result_t<decltype(branch_get_predicate_v<S, B>), S>, bool>;

template<typename...Bs>
using switcher_success_t = 
    details::list_to_t<
        details::set_t<
            details::List<
                details::parsed_type_t<branch_get_parser_t<Bs>>...
            >
        >, 
        Sum
    >;

template<bool is_partial, typename P, typename...Bs>
using switcher_error_t = 
    details::list_to_t<
        push_unique_if_t<
            is_partial, 
            error::NoBranch<details::parsed_type_t<P>>, 
            push_unique_if_t<
                P::can_fail,
                details::error_of_t<details::result_type_t<P>>, 
                branch_get_error_list_t<Bs...>
            >
        >,
        Sum
    >;


template<bool is_partial, typename P, typename L>
struct SwitcherParserReduced {};
template<bool is_partial, typename P, typename...Bs>
struct SwitcherParserReduced<is_partial, P, details::List<Bs...>> {
    using type = 
        Parser<
            Switcher<P, Bs...>, 
            switcher_success_t<Bs...>, 
            switcher_error_t<is_partial, P, Bs...>
        >;
};

template<typename...Bs>
static constexpr bool is_partial_v = (true && ... && !is_not_branch_v<Bs>);
template<typename P, typename...Bs>
using switcher_parser_t = typename
    SwitcherParserReduced<
        is_partial_v<Bs...>,
        P,
        details::take_until_t<IsNotBranch, details::List<Bs...>>
    >::type;

}

/*
    Switcher :: 
        Parser s e -> 
        Branch (s -> bool) (Parser s' e')... -> 
        Parser Sum<s'...> Sum<e, e'...>

    Given a parser P and a list of branches, 
    parse with P first, then test each branch's predicate with the parsed value
    if a predicate holds, the branch's parser is run and returned
    
*/
 


template<typename P, typename...Bs>
struct Switcher : switcher_parser_t<P, Bs...> {
public:

    static_assert(details::is_parser_soft_check_v<P>, "Switcher requires parser-only parameters");
    static_assert((true && ... && is_valid_branch_v<details::parsed_type_t<P>, Bs>), "Switcher requires only valid Branches");
    
    using result_t = details::result_type_t<Switcher<P, Bs...>>;

    template<typename R>
    static result_t parse(R reader) {
        auto res = P::parse(reader);
        if constexpr (P::can_fail) {
            if (res.is_error()) {
                return fail(res.cursor(), std::move(res.error()));
            }
        }

        reader = R::from_cursor(reader, res.cursor());
        return parse_at<0>(reader, std::move(res.success()));
    }

private:

    template<std::size_t I, typename R, typename S>
    static std::enable_if_t<(I >= sizeof...(Bs)) && is_partial_v<Bs...>, result_t> 
    parse_at(R reader, S&& value) {
        return fail(reader.cursor(), error::NoBranch<details::parsed_type_t<P>>{ std::move(value) });
    }

    template<std::size_t I, typename R, typename S>
    static std::enable_if_t<(I < sizeof...(Bs)) || !is_partial_v<Bs...>, result_t> 
    parse_at(R reader, S&& value) {
        using B = details::at_t<details::List<Bs...>, I>;
        using T = branch_get_parser_t<B>;

        if constexpr(!is_not_branch_v<B>) {
            if (branch_get_predicate_v<details::parsed_type_t<P>, B>(static_cast<details::parsed_type_t<P> const&>(value))) {
                auto res = T::parse(reader);
                if constexpr(T::can_fail) {
                    if (res.is_error()) {
                        return fail(res.cursor(), std::move(res.error()));
                    }
                }

                return success(res.cursor(), std::move(res.success()));
            } else {
                return parse_at<I+1>(reader, std::move(value));
            }
        } else {
            auto res = T::parse(reader);
            if constexpr(T::can_fail) {
                if (res.is_error()) {
                    return fail(res.cursor(), std::move(res.error()));
                }
            }

            return success(res.cursor(), std::move(res.success()));
        }
    }
};





/*
    Parser as value
 */

template<auto p, auto...ps>
constexpr Switcher<std::decay_t<decltype(p)>, std::decay_t<decltype(ps)>...> switcher;

template<auto v, auto p>
constexpr Branch<equal_predicate<v>, std::decay_t<decltype(p)>> branch;

template<auto& f, auto p>
constexpr Branch<f, std::decay_t<decltype(p)>> branchf;

template<auto p>
constexpr auto otherwise = p;


}
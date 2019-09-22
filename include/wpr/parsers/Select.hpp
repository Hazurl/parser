#pragma once

#include <type_traits>

#include <wpr/Parsers.hpp>

namespace wpr {

namespace {

template<typename P, std::size_t N>
auto peek_helper(P product) {
    return std::move(std::get<N>(product));
}

template<typename P, std::size_t...Ns>
auto select_helper(P product) {
    using list_of_elements_t = details::list_from_t<Product, P>;
    return Product<details::at_t<list_of_elements_t, Ns>...>(
        (details::count_v<details::at_t<list_of_elements_t, Ns>, list_of_elements_t> > 1 ? 
            details::at_t<list_of_elements_t, Ns>( std::get<Ns>(product) ) : 
            details::at_t<list_of_elements_t, Ns>( std::move(std::get<Ns>(product)) )
        )...
    );
}

}

template<bool OnSuccessValue, std::size_t N>
struct Peek {};

template<std::size_t N>
constexpr Peek<true, N> peek;

template<std::size_t N>
constexpr Peek<false, N> peek_err;


namespace details {

template<std::size_t N, typename P>
struct GetTransformer<Peek<true, N>, P> {
    using product_t = details::parsed_type_t<P>;
    static_assert(details::is_same_HK_type_v<product_t, Product>, "Peek can only be use on Product-s");
    using type = Transformer<P, peek_helper<product_t, N>>;
};

template<std::size_t N, typename P>
struct GetTransformer<Peek<false, N>, P> {
    using product_t = details::error_of_t<details::result_type_t<P>>;
    static_assert(details::is_same_HK_type_v<product_t, Product>, "Peek can only be use on Product-s");
    using type = ErrorTransformer<P, peek_helper<product_t, N>>;
};

}




template<bool OnSuccessValue, std::size_t...Ns>
struct Select {};

template<std::size_t...Ns>
constexpr Select<true, Ns...> select;

template<std::size_t...Ns>
constexpr Select<false, Ns...> select_err;


namespace details {

template<std::size_t...Ns, typename P>
struct GetTransformer<Select<true, Ns...>, P> {
    using product_t = details::parsed_type_t<P>;
    static_assert(details::is_same_HK_type_v<product_t, Product>, "Select can only be use on Product-s");
    using type = Transformer<P, select_helper<product_t, Ns...>>;
};

template<std::size_t...Ns, typename P>
struct GetTransformer<Select<false, Ns...>, P> {
    using product_t = details::error_of_t<details::result_type_t<P>>;
    static_assert(details::is_same_HK_type_v<product_t, Product>, "Select can only be use on Product-s");
    using type = ErrorTransformer<P, select_helper<product_t, Ns...>>;
};

}



}
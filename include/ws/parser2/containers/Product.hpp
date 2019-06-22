#pragma once

#include <ostream>
#include <tuple>

namespace ws::parser2 {

/*
    Details
 */

template<typename...Ts>
struct Product;

namespace product_details {

template<typename...Ts, std::size_t...Is>
std::ostream& print_product_helper(std::ostream& os, Product<Ts...> const& prod, std::index_sequence<Is...>);

template<typename...Ts, std::size_t...Is>
bool equal_product_helper(Product<Ts...> const& lhs, Product<Ts...> const& rhs, std::index_sequence<Is...>);

}




/*
    Product type
 */

template<typename...Ts>
struct Product : std::tuple<Ts...> {};

template<typename...Ts> Product(Ts...) -> Product<Ts...>;






/*
    Operator <<
 */

template<typename...Ts>
std::ostream& operator <<(std::ostream& os, Product<Ts...> const& prod) {
    return product_details::print_product_helper(os, prod, std::make_index_sequence<sizeof...(Ts)>{});
}

namespace product_details {

    template<typename...Ts, std::size_t...Is>
    std::ostream& print_product_helper(std::ostream& os, Product<Ts...> const& prod, std::index_sequence<Is...>) {
        return ((os << "(") << ... << std::get<Is>(prod)) << ")";
    }

}






/*
    Operator ==
 */

template<typename...Ts>
bool operator ==(Product<Ts...> const& lhs, Product<Ts...> const& rhs) {
    return product_details::equal_product_helper(lhs, rhs, std::make_index_sequence<sizeof...(Ts)>{});
}

namespace product_details {

    template<typename...Ts, std::size_t...Is>
    bool equal_product_helper(Product<Ts...> const& lhs, Product<Ts...> const& rhs, std::index_sequence<Is...>) {
        return (... && (std::get<Is>(lhs) == std::get<Is>(rhs)));
    }

}

}
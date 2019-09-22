#pragma once

#include <ostream>
#include <tuple>

<<<<<<< refs/remotes/origin/parser-v2:include/wpr/containers/Product.hpp
namespace wpr {
=======
#include <ws/parser2/Details.hpp>


namespace ws::parser2 {
>>>>>>> Add as_tuple to product:include/ws/parser2/containers/Product.hpp

/*
    Details
 */

template<typename...Ts>
struct Product;

namespace product_details {

template<typename...Ts, std::size_t...Is>
std::ostream& print_product_helper(std::ostream& os, Product<Ts...> const& prod, std::index_sequence<Is...>);

template<typename...Ts, std::size_t...Is>
std::string describe_product_helper(Product<Ts...> const& prod, std::index_sequence<Is...>);

template<typename...Ts, std::size_t...Is>
bool equal_product_helper(Product<Ts...> const& lhs, Product<Ts...> const& rhs, std::index_sequence<Is...>);

}




/*
    Product type
 */

struct from_tuple_t {};

template<typename...Ts>
struct Product : std::tuple<Ts...> {
    Product() = default;
    Product(Ts... ts) : std::tuple<Ts...>(std::move(ts)...) {};
    Product(from_tuple_t, std::tuple<Ts...> ts) : std::tuple<Ts...>(ts) {};

    std::tuple<Ts...> const& as_tuple() const& {
        return static_cast<std::tuple<Ts...> const&>(*this);
    }

    std::tuple<Ts...> & as_tuple() & {
        return static_cast<std::tuple<Ts...> &>(*this);
    }

    std::tuple<Ts...> && as_tuple() && {
        return static_cast<std::tuple<Ts...> &&>(*this);
    }
};

template<>
struct Product<> : std::tuple<> {
    Product() = default;
    Product(from_tuple_t, std::tuple<> ts) : std::tuple<>(ts) {};

    std::tuple<> const& as_tuple() const& {
        return static_cast<std::tuple<> const&>(*this);
    }

    std::tuple<> & as_tuple() & {
        return static_cast<std::tuple<> &>(*this);
    }

    std::tuple<> && as_tuple() && {
        return static_cast<std::tuple<> &&>(*this);
    }
};

template<typename...Ts> Product(Ts...) -> Product<Ts...>;
template<typename...Ts> Product(from_tuple_t, std::tuple<Ts...>) -> Product<Ts...>;






/*
    Describe
 */

template<typename...Ts>
struct Describe<Product<Ts...>> {
    std::string operator()(Product<Ts...> const& prod) {
        return product_details::describe_product_helper(prod, std::make_index_sequence<sizeof...(Ts)>{});
    }
};

namespace product_details {

    template<typename...Ts, std::size_t...Is>
    std::string describe_product_helper(Product<Ts...> const& prod, std::index_sequence<Is...>) {
        return (std::string{ "(" } + ... + (std::string(Is == 0 ? "" : ", ") + describe(std::get<Is>(prod)))) + ")";
    }

}





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
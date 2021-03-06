#pragma once

#include <wpr/Containers.hpp>
#include <wpr/Details.hpp>
#include <wpr/parsers/Errors.hpp>

#include <vector>
#include <limits>

namespace wpr {

/*
    Open boundary
*/

inline constexpr std::size_t open_minimum = 0;
inline constexpr std::size_t open_maximum = std::numeric_limits<std::size_t>::max();


/*
    Repeat parser
 */

template<typename P, std::size_t min = open_minimum, std::size_t max = open_maximum, template<typename> typename C = std::vector>
struct Repeat : Parser<Repeat<P, min, max, C>, C<details::parsed_type_t<P>>, std::conditional_t<(min <= open_minimum), void, details::error_of_t<details::result_type_t<P>>>> {

    using container_t = C<details::parsed_type_t<P>>;

private:

    static constexpr bool is_open_min = min <= open_minimum;

public:

    template<typename R>
    static details::result_type_t<Repeat<P, min, max, C>> parse(R reader) {
        std::size_t last_cursor{ reader.cursor() };
        container_t container;

        while(std::size(container) < max) {
            auto res = P::parse(reader);

            // Fail if P failed and the minimum amount to parse has not been reached yet
            // If the minimum amount has been reached, return a success
            // Finally, if the maximum amount to parse has been reached, stop and return a success

            if constexpr(P::can_fail) {
                if (res.is_error()) {
                    if constexpr(!is_open_min) {
                        if (std::size(container) < min) {
                            return fail(res.cursor(), std::move(res.error()));
                        }
                    }

                    return success(last_cursor, std::move(container));
                }
            }

            last_cursor = res.cursor();
            container.push_back(std::move(res).success());
            reader = R::from_cursor(reader, last_cursor);
        }

        return success(last_cursor, std::move(container));
    }

};





/*
    Parser as value
 */

template<auto P, std::size_t min = open_minimum, std::size_t max = open_maximum, template<typename> typename C = std::vector>
constexpr Repeat<std::decay_t<decltype(P)>, min, max, C> repeat;





/*
    Alias for many
 */

template<auto P, template<typename> typename C = std::vector>
constexpr Repeat<std::decay_t<decltype(P)>, 0, open_maximum, C> many;





/*
    Alias for some
 */

template<auto P, template<typename> typename C = std::vector>
constexpr Repeat<std::decay_t<decltype(P)>, 1, open_maximum, C> some;






/*
    Alias for exact
 */

template<auto P, std::size_t N, template<typename> typename C = std::vector>
constexpr Repeat<std::decay_t<decltype(P)>, N, N, C> exact;






}
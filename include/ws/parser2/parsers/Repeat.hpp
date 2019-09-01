#pragma once

#include <ws/parser2/Containers.hpp>
#include <ws/parser2/Details.hpp>
#include <ws/parser2/parsers/Errors.hpp>

#include <vector>
#include <limits>

namespace ws::parser2 {

/*
    Open boundary
*/

inline constexpr std::size_t open_minimum = 0;
inline constexpr std::size_t open_maximum = std::numeric_limits<std::size_t>::max();


/*
    Repeat parser
 */

template<typename P, std::size_t min = open_minimum, std::size_t max = open_maximum, template<typename> typename C = std::vector>
struct Repeat : Parser<Repeat<P, min, max, C>, C<details::parsed_type_t<P>>, std::conditional_t<(min <= open_minimum), void, error::Expected<P>>> {

    using container_t = C<details::parsed_type_t<P>>;

private:

    static constexpr bool is_open_min = min <= open_minimum;

public:

    template<typename R>
    static details::result_type_t<Repeat<P, min, max, C>> parse(R reader) {
        std::size_t last_cursor{ reader.cursor() };
        container_t container;

        //std::size_t i{ 0 };

        while(std::size(container) < (max/* > 10 ? 10 : max*/)) {
            //if (++i > 20) break;
            auto res = P::parse(reader);

            // Fail if P failed and the minimum amount to parse has not been reached yet
            // If the minimum amount has been reached, return a success
            // Finally, if the maximum amount to parse has been reached, stop and return a success

            if constexpr(P::can_fail) {
                if (res.is_error()) {
                    if constexpr(!is_open_min) {
                        if (std::size(container) < min) {
                            return fail(res.cursor(), error::Expected<P>{});
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
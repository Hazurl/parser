#pragma once

namespace wpr {

struct BoundedReader {

    inline BoundedReader(char const* begin_, char const* end_) 
        : begin{ begin_                          }, end{ end_               }, index { 0 } {}
    template<std::size_t N>
    inline BoundedReader(char const (&str)[N]) 
        : begin{ str                             }, end{ str + N - 1        }, index { 0 } {}
    inline BoundedReader(std::string const& str) 
        : begin{ str.empty() ? nullptr : &str[0] }, end{ begin + str.size() }, index { 0 } {}
    inline BoundedReader(std::string_view const& str) 
        : begin{ str.empty() ? nullptr : &str[0] }, end{ begin + str.size() }, index { 0 } {}

    inline static BoundedReader copy_move(BoundedReader r, std::size_t advance) {
        r.index += advance;
        return r;
    }

    inline static BoundedReader from_cursor(BoundedReader r, std::size_t cursor) {
        r.index = cursor;
        return r;
    }

    inline std::size_t cursor() const {
        return index;
    }

    inline bool is_end(std::size_t i = 0) const {
        return begin + index + i >= end;
    }

    inline char const& operator[](std::size_t i) const {
        assert(!is_end(i));
        return begin[index + i];
    }

    inline char peek() const {
        return operator[](0);
    }

    inline BoundedReader& operator++() {
        ++index;
        return *this;
    }

private:

    char const* begin{ nullptr };
    char const* end{ nullptr };
    std::size_t index{ 0 };

};

}
# Result

```cpp
template<typename S, typename E = void>
struct Result;
```

This is the type returned by evaluating a parser with a reader. `Result<S>` or `Result<S, void>` is the result of a parser that can't fail.

A result holds either `S` or `E` aswell as a `cursor` that represent the furthest the parser goes.

## can_fail

`static constexpr bool can_fail`
It's a trait telling whether or not the result can hold a failure.

## Constructor

```cpp
template<typename...Args>
Result(error_tag_t, std::size_t cursor, Args&&...args);             (1)

template<typename...Args>
Result(success_tag_t, std::size_t cursor, Args&&...args);           (2)

template<typename L, typename...Args>
Result(ResultBuilder<L> l);                                         (3)
```
`(1)` constructs a Result holding a failure `E` from `args`.\
`(2)` constructs a Result holding a success `S` from `args`.

`(1)` is only enable if `E` isn't `void`.

`ResultBuilder<L>` in `(3)`is constructed from the functions `success` and `fail`.

## is_success, is_error

```cpp
bool Result::is_success() const;
```
Returns `true` if the result holds `S`.

```cpp
bool Result::is_error() const;
```

Returns `true` if the result holds `E`. It's only enable if `E` isn't `void`.

## success, error

```cpp
S const& Result::success() const&;
S&& Result::success() &&;
S& Result::success() &;
```
Access the success. The behaviour is undefined if `!is_success()`.

```cpp
E const& Result::error() const&;
E&& Result::error() &&;
E& Result::error() &;
```
Access the error. The behaviour is undefined if `!is_error()`. It's only enable if `E` isn't `void`.

## cursor

```cpp
std::size_t Result::cursor() const;
``` 

Simply returns the `cursor` attached to the result.

## success, fail

```cpp
template<typename...Args>
auto success(std::size_t cursor, Args&&...args);

template<typename...Args>
auto fail(std::size_t cursor, Args&&...args);
```

Both methods returns a ResultBuilder that can be used to create a Result. It's purpose is to avoid verbosity of having to pass the tag and the result template argument.

> Example:
> ```cpp
> Result<int, float> foo(bool b, std::size_t cursor) {
>     if (b) {
>         return success(cursor, 42);
>     }   
>     else {
>         return fail(cursor, 13.37f);
>     }   
> }
>```
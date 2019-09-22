# Parser

As said in the introduction, a parser can either succeed or fail, some of them always succeed. The essence of this library is to *combine* parsers. One can distinguish three types of parsers:

- **Terminal**: It doesn't use any other parser, we can name for example `nextc` which read the next character or `unit<v>` which return `v` as a success. 
- **Combinator**: It combine different parsers to create a more complex parser, we can name `seq<p...>` which parse sequentially each parser `p` or `first<p...>` which return the first value successfully parsed.
- **Transformer**: It doesn't read from the input, it simply transform the output of other parsers, we can name `Map<f, P>` which apply `f` to the success value of `P`.

A parser is not a complex type, it simply a class with a public static method:
```cpp
template<typename Reader>
Result<S, E> parse(Reader reader);
```

But most of the time you don't need to create them manually as the library implements most of the basic building block to create any parser.

## Traits

You can also access various informations about a parser with these accessors:
- `template<typename P> using parsed_type_t` is the type parsed by P
- `template<typename P> using error_type_t` is the error returned by P if it fails
- `template<typename P> using parser_type_t` is the Parser that P inherit
- `template<typename P> using result_type_t` is the type returned by the `parse` method of P
- `template<typename P> constexpr bool can_fail_v` is true if and only if `P` can fail

If you write your own parser, you can check is validity with `is_parser_v<P>`. If it's not, some static_assert will lead you to cause.

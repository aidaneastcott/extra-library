# Extra Library

## Overview
A simple header-only library containing miscellaneous utility macros, functions, and types for C and C++.

## Features

### xtr::enumerate
A helper function for accessing the index in a range-based for loop.

```cpp
std::vector<int> vec1{20, 40, 60, 80};

// Keeping track of the index manually with a ranged-based for loop
std::size_t index = 0;
for (auto &&value : vec1) {
  // Access index and value for each element in the vector
  ++index;
}

// Equivalent code with xtr::enumerate
for (auto &&[index, value] : xtr::enumerate(vec1)) {
  // Access index and value for each element in the vector
}

auto make_vec = []() { return std::vector{20, 40, 60, 80}; };

// Works correctly with temporaries
for (auto &&[index, value] : xtr::enumerate(make_vec())) {
  // Access index and value for each element in the vector
}

// Construct the container in place using the variadic overload
for (auto &&[index, value] : xtr::enumerate<std::vector<int>>(20, 40, 60, 80)) {
  // Access index and value for each element in the vector
}
```

The value in the range-based for loop is a `std::tuple` storing 2 values, the index and element value respectively. If the container has a member type `size_type`, then the index is a const qualified value of that type. Otherwise, the index is a const qualified `std::size_t`. The type of the element value is the same as in a regular range-based for loop.

### xtr::multiarray
A type alias to `std::array` for a more readable multidimensional array syntax.

```cpp
// 2D array using a C style array
int arr1[5][10] = { 0 };

// 2D array using std::array without type alias
std::array<std::array<int, 10>, 5> arr2;

// 2D array using std::array with xtr::multiarray alias
xtr::multiarray<int, 5, 10> arr3;

static_assert(std::is_same_v<decltype(arr2), decltype(arr3)>);
```

### xtr::is_aligned
A helper function for checking the alignment of memory addresses
```cpp
char val = 65;

// Always true
assert(xtr::is_aligned<char>(&val));

// Maybe not true
assert(xtr::is_aligned<int>(&val));
```

### Macros for debug logging
- Macro function `debug_log` prints a message to `stdout` when in debug mode

### Macros for compiler optimization hints
- Macro function `assume` tells the compiler to assume an expression will always be true
- Macro function `assert_assume` combines `assume` behavior with an assertion in debug mode
- Macro functions `likely` and `unlikely` hint to the compiler the likelihood of an expression being true
- Macro `restrict` enables the C language restrict keyword in C++

### Additional macro functions
- Macro functions `concat_string`, `literal_string`, and `macro_string` for dealing with string literals and macros

## License
Licensed under [MIT](LICENSE).

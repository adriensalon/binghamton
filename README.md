# binghamton

This library aims at providing generic and robust C++17 implementations of well-known steganography algorithms from the Digital Data Embedding Laboratory at Binghamton University.

A research implementation is available at [dde.binghamton.edu](https://dde.binghamton.edu/download/stego_algorithms/) as a MATLAB backend and a compiled CLI tool. Unlike the original sources from the Binghamton University this implementation does not use explicit x86 SSE2 intrinsics and leverages modern language constructs without requiring any other dependency than C++17.

GTest and stb_image are only provided for running tests that assert correctness of payloads across payload -> embed -> extract -> payload roundtrips.

## Features

- WOW (Wavelet Obtained Weights) implemented in [method/wow.hpp](include/binghamton/method/wow.hpp)

<!-- ## Usage

This library expects RGB images as linear 
```cpp
std::vector<std::uint8_t> image_pixels;
std::size_t image_width, image_height,
```

A steganography key...

```cpp
std::array<std::uint8_t, 32> steganography_key {};
```
The payload

```cpp
std::vector<std::uint8_t> payload = {
    1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1
};
```

```cpp
std::vector<std::uint8_t> image_embedded_pixels;
double cost = binghamton::embed_wow(
        image_pixels, image_width, image_height,
        steganography_key,
        payload,
        image_embedded_pixels);
``` -->

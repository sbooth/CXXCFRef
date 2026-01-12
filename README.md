# CXXCFRef

A simple [RAII](https://en.cppreference.com/w/cpp/language/raii.html) wrapper for [Core Foundation](https://developer.apple.com/documentation/corefoundation) objects.

> [!NOTE]
> C++17 is required.

## Installation

### Swift Package Manager

Add a package dependency to https://github.com/sbooth/CXXCFRef in Xcode.

### Manual or Custom Build

1. Clone the [CXXCFRef](https://github.com/sbooth/CXXCFRef) repository.
2. `swift build`.

## Alternatives

If you prefer a minimalist [`std::unique_ptr`](https://en.cppreference.com/w/cpp/memory/unique_ptr.html)-based approach:

```c++
struct cf_deleter {
    void operator()(CFTypeRef cf CF_RELEASES_ARGUMENT) noexcept { CFRelease(cf); }
};

template <typename T>
using cf_ptr = std::unique_ptr<std::remove_pointer_t<T>, cf_deleter>;
``` 

Or a C-based approach:

```c
static void CleanupCFString(CFStringRef *cf_ptr) { if(*cf_ptr) CFRelease(*cf_ptr); }
__attribute__((cleanup(CleanupCFString))) CFStringRef str = /* ... */
```

## License

Released under the [MIT License](https://github.com/sbooth/CXXCFRef/blob/main/LICENSE.txt).

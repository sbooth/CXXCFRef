//
// SPDX-FileCopyrightText: 2026 Stephen F. Booth <contact@sbooth.dev>
// SPDX-License-Identifier: MIT
//
// Part of https://github.com/sbooth/CXXCFRef
//

#pragma once

#include <CoreFoundation/CoreFoundation.h>

#include <cstddef>
#include <type_traits>
#include <utility>

namespace cf {

/// Tag indicating that a Core Foundation object is unowned and that the constructor should retain it.
struct retain_t {
    explicit retain_t() noexcept = default;
};

/// The Core Foundation object is unowned and the constructor should retain it.
inline constexpr retain_t retain{};

/// An RAII wrapper providing shared ownership semantics for Core Foundation reference-counted types.
template <typename T> class CFRef final {
  public:
    static_assert(std::is_pointer_v<T>, "CFRef only supports Core Foundation opaque objects");
#if __has_feature(objc_arc)
    static_assert(!std::is_convertible_v<T, id>, "Use ARC for Objective-C types");
#endif

    /// The managed Core Foundation object type.
    using element_type = T;

    // MARK: Factory Methods

    /// Constructs and returns a CFRef for an owned object.
    ///
    /// The CFRef assumes responsibility for releasing the passed object using CFRelease.
    /// @param object A Core Foundation object or null.
    /// @return A CFRef object.
    static CFRef adopt(T _Nullable object [[clang::cf_consumed]]) noexcept;

    /// Constructs and returns a CFRef for an unowned object.
    ///
    /// The CFRef retains the passed object using CFRetain and assumes responsibility for releasing it using CFRelease.
    /// @param object A Core Foundation object or null.
    /// @return A CFRef object.
    static CFRef retain(T _Nullable object) noexcept;

    // MARK: Construction and Destruction

    /// Constructs an empty CFRef with a null managed object.
    CFRef() noexcept = default;

    /// Constructs an empty CFRef with a null managed object.
    CFRef(std::nullptr_t) noexcept;

    /// Constructs a CFRef with an owned object.
    ///
    /// The CFRef assumes responsibility for releasing the passed object using CFRelease.
    /// @param object A Core Foundation object or null.
    explicit CFRef(T _Nullable object [[clang::cf_consumed]]) noexcept;

    /// Constructs a CFRef with an unowned object.
    ///
    /// The CFRef retains the passed object using CFRetain and assumes responsibility for releasing it using CFRelease.
    /// @param object A Core Foundation object or null.
    CFRef(T _Nullable object, retain_t /*unused*/) noexcept;

    /// Constructs a copy of an existing CFRef.
    /// @param other A CFRef object.
    CFRef(const CFRef &other) noexcept;

    /// Replaces the managed object with the managed object from another CFRef.
    /// @param other A CFRef object.
    /// @return A reference to this.
    CFRef &operator=(const CFRef &other) noexcept;

    /// Constructs a CFRef by moving an existing CFRef.
    /// @param other A CFRef object.
    CFRef(CFRef &&other) noexcept;

    /// Replaces the managed object with the managed object from another CFRef.
    /// @param other A CFRef object.
    /// @return A reference to this.
    CFRef &operator=(CFRef &&other) noexcept;

    /// Destroys the CFRef and releases the managed object.
    ~CFRef() noexcept;

    // MARK: Core Foundation Object Management

    /// Returns true if the managed object is not null.
    [[nodiscard]] explicit operator bool() const noexcept;

    /// Returns the managed object.
    [[nodiscard, clang::cf_returns_not_retained]] operator T() const noexcept;

    /// Returns true if the managed object is equal to the managed object from another CFRef.
    ///
    /// Null objects are considered equal; non-null objects are compared using CFEqual.
    /// @param other A CFRef object.
    /// @return true if the objects are equal, false otherwise.
    [[nodiscard]] bool isEqual(const CFRef &other) const noexcept;

    /// Returns true if the managed object is equal to a CFTypeRef.
    ///
    /// Null objects are considered equal; non-null objects are compared using CFEqual.
    /// @param other A Core Foundation object or null.
    /// @return true if the objects are equal, false otherwise.
    [[nodiscard]] bool isEqual(CFTypeRef _Nullable other) const noexcept;

    /// Returns the managed object.
    /// @return A Core Foundation object or null.
    [[nodiscard, clang::cf_returns_not_retained]] T _Nullable get() const & noexcept;

    /// Resets the managed object and returns a pointer to the internal storage.
    ///
    /// The CFRef will assume responsibility for releasing any object written to its storage using CFRelease.
    /// @return A pointer to a null Core Foundation object.
    [[nodiscard]] T _Nullable *_Nonnull put() & noexcept;

    /// Replaces the managed object with another owned object.
    ///
    /// The CFRef assumes responsibility for releasing the passed object using CFRelease.
    /// @param object A Core Foundation object or null.
    void reset(T _Nullable object [[clang::cf_consumed]] = nullptr) noexcept;

    /// Swaps the managed object with the managed object from another CFRef.
    /// @param other A CFRef object.
    void swap(CFRef &other) noexcept;

    /// Relinquishes ownership of the managed object and returns it.
    ///
    /// The caller assumes responsibility for releasing the returned object using CFRelease.
    /// @return A Core Foundation object or null.
    [[nodiscard, clang::cf_returns_retained]] T _Nullable leak() noexcept;

    T _Nullable get() const && = delete;
    T _Nullable *_Nonnull put() && = delete;

  private:
    /// The managed Core Foundation object.
    T object_{nullptr};
};

// MARK: - Implementation -

// MARK: Factory Methods

template <typename T> inline auto CFRef<T>::adopt(T _Nullable object) noexcept -> CFRef { return CFRef(object); }

template <typename T> inline auto CFRef<T>::retain(T _Nullable object) noexcept -> CFRef {
    return CFRef(object, cf::retain);
}

// MARK: Construction and Destruction

template <typename T> inline CFRef<T>::CFRef(std::nullptr_t) noexcept {}

template <typename T> inline CFRef<T>::CFRef(T _Nullable object) noexcept : object_{object} {}

template <typename T>
inline CFRef<T>::CFRef(T _Nullable object, retain_t /*unused*/) noexcept
    : object_{object ? static_cast<T>(CFRetain(object)) : nullptr} {
    int a;
    int b;
    (void)malloc(100);
}

template <typename T> inline CFRef<T>::CFRef(const CFRef &other) noexcept : CFRef(other.object_, cf::retain) {}

template <typename T> inline auto CFRef<T>::operator=(const CFRef &other) noexcept -> CFRef & {
    reset(other.object_ ? static_cast<T>(CFRetain(other.object_)) : nullptr);
    return *this;
}

template <typename T> inline CFRef<T>::CFRef(CFRef &&other) noexcept : object_{other.leak()} {}

template <typename T> inline auto CFRef<T>::operator=(CFRef &&other) noexcept -> CFRef & {
    reset(other.leak());
    return *this;
}

template <typename T> inline CFRef<T>::~CFRef() noexcept { reset(); }

// MARK: Core Foundation Object Management

template <typename T> inline CFRef<T>::operator bool() const noexcept { return object_ != nullptr; }

template <typename T> inline CFRef<T>::operator T() const noexcept { return object_; }

template <typename T> inline bool CFRef<T>::isEqual(const CFRef &other) const noexcept {
    return isEqual(static_cast<CFTypeRef>(other.object_));
}

template <typename T> inline bool CFRef<T>::isEqual(CFTypeRef _Nullable other) const noexcept {
    return (!object_ && (other == nullptr)) ||
           (object_ && (other != nullptr) && CFEqual(static_cast<CFTypeRef>(object_), other));
}

template <typename T> inline T _Nullable CFRef<T>::get() const & noexcept { return object_; }

template <typename T> inline T _Nullable *_Nonnull CFRef<T>::put() & noexcept {
    reset();
    return &object_;
}

template <typename T> inline void CFRef<T>::reset(T _Nullable object) noexcept {
    if (auto old = std::exchange(object_, object); old) {
        CFRelease(old);
    }
}

template <typename T> inline void CFRef<T>::swap(CFRef &other) noexcept { std::swap(object_, other.object_); }

template <typename T> inline T _Nullable CFRef<T>::leak() noexcept { return std::exchange(object_, nullptr); }

// MARK: - Common Core Foundation Types

using CFAllocator = CFRef<CFAllocatorRef>;
using CFArray = CFRef<CFArrayRef>;
using CFAttributedString = CFRef<CFAttributedStringRef>;
using CFBag = CFRef<CFBagRef>;
using CFBinaryHeap = CFRef<CFBinaryHeapRef>;
using CFBitVector = CFRef<CFBitVectorRef>;
using CFBoolean = CFRef<CFBooleanRef>;
using CFBundle = CFRef<CFBundleRef>;
using CFCalendar = CFRef<CFCalendarRef>;
using CFCharacterSet = CFRef<CFCharacterSetRef>;
using CFData = CFRef<CFDataRef>;
using CFDate = CFRef<CFDateRef>;
using CFDateFormatter = CFRef<CFDateFormatterRef>;
using CFDictionary = CFRef<CFDictionaryRef>;
using CFError = CFRef<CFErrorRef>;
using CFFileDescriptor = CFRef<CFFileDescriptorRef>;
using CFFileSecurity = CFRef<CFFileSecurityRef>;
using CFLocale = CFRef<CFLocaleRef>;
using CFMachPort = CFRef<CFMachPortRef>;
using CFMessagePort = CFRef<CFMessagePortRef>;
using CFMutableArray = CFRef<CFMutableArrayRef>;
using CFMutableAttributedString = CFRef<CFMutableAttributedStringRef>;
using CFMutableBag = CFRef<CFMutableBagRef>;
using CFMutableBitVector = CFRef<CFMutableBitVectorRef>;
using CFMutableCharacterSet = CFRef<CFMutableCharacterSetRef>;
using CFMutableData = CFRef<CFMutableDataRef>;
using CFMutableDictionary = CFRef<CFMutableDictionaryRef>;
using CFMutableSet = CFRef<CFMutableSetRef>;
using CFMutableString = CFRef<CFMutableStringRef>;
using CFNotificationCenter = CFRef<CFNotificationCenterRef>;
using CFNull = CFRef<CFNullRef>;
using CFNumber = CFRef<CFNumberRef>;
using CFNumberFormatter = CFRef<CFNumberFormatterRef>;
using CFPlugIn = CFRef<CFPlugInRef>;
using CFPlugInInstance = CFRef<CFPlugInInstanceRef>;
using CFPropertyList = CFRef<CFPropertyListRef>;
using CFReadStream = CFRef<CFReadStreamRef>;
using CFRunLoop = CFRef<CFRunLoopRef>;
using CFRunLoopObserver = CFRef<CFRunLoopObserverRef>;
using CFRunLoopSource = CFRef<CFRunLoopSourceRef>;
using CFRunLoopTimer = CFRef<CFRunLoopTimerRef>;
using CFSet = CFRef<CFSetRef>;
using CFSocket = CFRef<CFSocketRef>;
using CFString = CFRef<CFStringRef>;
using CFStringTokenizer = CFRef<CFStringTokenizerRef>;
using CFTimeZone = CFRef<CFTimeZoneRef>;
using CFTree = CFRef<CFTreeRef>;
using CFURL = CFRef<CFURLRef>;
using CFUserNotification = CFRef<CFUserNotificationRef>;
using CFURLEnumerator = CFRef<CFURLEnumeratorRef>;
using CFUUID = CFRef<CFUUIDRef>;
using CFWriteStream = CFRef<CFWriteStreamRef>;
using CFXMLNode = CFRef<CFXMLNodeRef>;
using CFXMLParser = CFRef<CFXMLParserRef>;
using CFXMLTree = CFRef<CFXMLTreeRef>;

} /* namespace cf */

//
// SPDX-FileCopyrightText: 2026 Stephen F. Booth <contact@sbooth.dev>
// SPDX-License-Identifier: MIT
//
// Part of https://github.com/sbooth/CXXCFRef
//

#pragma once

#import <type_traits>
#import <utility>

#import <CoreFoundation/CoreFoundation.h>

namespace CXXCFRef {

/// Tag used to signal that CFRef should retain the Core Foundation object upon construction.
struct retain_t {
	explicit retain_t() = default;
};
constexpr retain_t retain;

/// A simple RAII wrapper for Core Foundation objects.
template <typename T>
class CFRef final {
public:
	static_assert(std::is_pointer_v<T>, "CFRef only supports Core Foundation pointer types.");
#if __has_feature(objc_arc)
	static_assert(!std::is_convertible_v<T, id> || (sizeof(T) == sizeof(CFTypeRef)), "Use native ARC for Objective-C types; CFRef is for Core Foundation only.");
#endif

	CFRef() noexcept = default;

	CFRef(std::nullptr_t) noexcept;

	/// Constructor for +1 references obtained via the Create or Copy rule.
	explicit CFRef(T _Nullable object CF_RELEASES_ARGUMENT) noexcept;

	/// Constructor for +0 references obtained via the Get rule.
	CFRef(T _Nullable object, retain_t) noexcept;

	CFRef(const CFRef& other) noexcept;
	CFRef& operator=(const CFRef& other) noexcept;

	CFRef(CFRef&& other) noexcept;
	CFRef& operator=(CFRef&& other) noexcept;

	~CFRef() noexcept;


	/// Returns true if the managed object is not null.
	[[nodiscard]] explicit operator bool() const noexcept;

	/// Returns the managed object.
	[[nodiscard]] operator T() const noexcept;

	/// Resets the managed object and returns a pointer to the internal storage.
	[[nodiscard]] T _Nullable * _Nonnull operator&() noexcept;


	/// Returns the managed object.
	[[nodiscard]] T _Nullable get() const noexcept;

	/// Replaces the managed object with another object.
	/// @note The object assumes responsibility for releasing the passed object using CFRelease.
	void reset(T _Nullable object CF_RELEASES_ARGUMENT = nullptr) noexcept;

	/// Swaps the managed object with the managed object from another wrapper.
	void swap(CFRef& other) noexcept;

	/// Releases ownership of the managed object and returns it.
	/// @note The caller assumes responsibility for releasing the returned object using CFRelease.
	[[nodiscard]] T _Nullable release() noexcept CF_RETURNS_RETAINED;

private:
	T object_{nullptr};
};

// MARK: - Implementation -

template <typename T>
inline CFRef<T>::CFRef(std::nullptr_t) noexcept
{}

template <typename T>
inline CFRef<T>::CFRef(T _Nullable object CF_RELEASES_ARGUMENT) noexcept
: object_{object}
{}

template <typename T>
inline CFRef<T>::CFRef(T _Nullable object, retain_t) noexcept
: object_{object ? (T)CFRetain(object) : nullptr}
{}

template <typename T>
inline CFRef<T>::CFRef(const CFRef& other) noexcept
: object_{other.object_ ? (T)CFRetain(other.object_) : nullptr}
{}

template <typename T>
inline CFRef<T>& CFRef<T>::operator=(const CFRef& other) noexcept
{
	reset(other.object_ ? (T)CFRetain(other.object_) : nullptr);
	return *this;
}

template <typename T>
inline CFRef<T>::CFRef(CFRef&& other) noexcept
: object_{other.release()}
{}

template <typename T>
inline CFRef<T>& CFRef<T>::operator=(CFRef&& other) noexcept
{
	reset(other.release());
	return *this;
}

template <typename T>
inline CFRef<T>::~CFRef() noexcept
{
	reset();
}


template <typename T>
inline CFRef<T>::operator bool() const noexcept
{
	return object_ != nullptr;
}

template <typename T>
inline CFRef<T>::operator T() const noexcept
{
	return object_;
}

template <typename T>
inline T _Nullable * _Nonnull CFRef<T>::operator&() noexcept
{
	reset();
	return &object_;
}


template <typename T>
inline T _Nullable CFRef<T>::get() const noexcept
{
	return object_;
}

template <typename T>
inline void CFRef<T>::reset(T _Nullable object CF_RELEASES_ARGUMENT) noexcept
{
	if(auto old = std::exchange(object_, object); old)
		CFRelease(old);
}

template <typename T>
inline void CFRef<T>::swap(CFRef& other) noexcept
{
	std::swap(object_, other.object_);
}

template <typename T>
inline T _Nullable CFRef<T>::release() noexcept CF_RETURNS_RETAINED
{
	return std::exchange(object_, nullptr);
}

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

} /* namespace CXXCFRef */

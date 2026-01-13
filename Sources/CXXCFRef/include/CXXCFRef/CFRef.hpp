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

/// Tag indicating that a Core Foundation object is unowned and that the constructor should retain it.
struct retain_ref_t {
	explicit retain_ref_t() noexcept = default;
};

/// The Core Foundation object is unowned and the constructor should retain it.
constexpr retain_ref_t retain_ref;

/// A simple RAII wrapper for Core Foundation objects.
template <typename T>
class CFRef final {
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
	static CFRef adopt(T _Nullable object CF_RELEASES_ARGUMENT) noexcept;

	/// Constructs and returns a CFRef for an unowned object.
	///
	/// The CFRef retains the passed object using CFRetain and assumes responsibility for releasing it using CFRelease..
	/// @param object A Core Foundation object or null.
	/// @return A CFRef object.
	static CFRef retain(T _Nullable object) noexcept;

	// MARK: Creation and Destruction

	/// Constructs an empty CFRef with a null managed object.
	CFRef() noexcept = default;

	/// Constructs an empty CFRef with a null managed object.
	CFRef(std::nullptr_t) noexcept;

	/// Constructs a CFRef with an owned object.
	///
	/// The CFRef assumes responsibility for releasing the passed object using CFRelease.
	/// @param object A Core Foundation object or null.
	explicit CFRef(T _Nullable object CF_RELEASES_ARGUMENT) noexcept;

	/// Constructs a CFRef with an unowned object.
	///
	/// The CFRef retains the passed object using CFRetain and assumes responsibility for releasing it using CFRelease..
	/// @param object A Core Foundation object or null.
	CFRef(T _Nullable object, retain_ref_t) noexcept;

	/// Constructs a copy of an existing CFRef.
	/// @param other A CFRef object.
	CFRef(const CFRef& other) noexcept;

	/// Replaces the managed object with the managed object from another CFRef.
	/// @param other A CFRef object.
	/// @return A reference to this.
	CFRef& operator=(const CFRef& other) noexcept;

	/// Constructs a CFRef by moving an existing CFRef.
	/// @param other A CFRef object.
	CFRef(CFRef&& other) noexcept;

	/// Replaces the managed object with the managed object from another CFRef.
	/// @param other A CFRef object.
	/// @return A reference to this.
	CFRef& operator=(CFRef&& other) noexcept;

	/// Destroys the CFRef and releases the managed object.
	~CFRef() noexcept;

	// MARK: Core Foundation Object Management

	/// Returns true if the managed object is not null.
	[[nodiscard]] explicit operator bool() const noexcept;

	/// Returns the managed object.
	[[nodiscard]] operator T() const noexcept;

	/// Returns the managed object.
	/// @return A Core Foundation object or null.
	[[nodiscard]] T _Nullable get() const & noexcept;

	/// Resets the managed object and returns a pointer to the internal storage.
	///
	/// The CFRef will assume responsibility for releasing any object written to its storage using CFRelease.
	/// @return A pointer to a null Core Foundation object.
	[[nodiscard]] T _Nullable * _Nonnull put() & noexcept;

	/// Replaces the managed object with another owned object.
	///
	/// The CFRef assumes responsibility for releasing the passed object using CFRelease.
	/// @param object A Core Foundation object or null.
	void reset(T _Nullable object CF_RELEASES_ARGUMENT = nullptr) noexcept;

	/// Swaps the managed object with the managed object from another CFRef.
	/// @param other A CFRef object.
	void swap(CFRef& other) noexcept;

	/// Releases ownership of the managed object and returns it.
	///
	/// The caller assumes responsibility for releasing the returned object using CFRelease.
	/// @return A Core Foundation object or null.
	[[nodiscard]] T _Nullable release() noexcept CF_RETURNS_RETAINED;

	T _Nullable get() const && = delete;
	T _Nullable * _Nonnull put() && = delete;

private:
	/// The managed Core Foundation object.
	T object_{nullptr};
};

// MARK: - Implementation -

// MARK: Factory Methods

template <typename T>
inline CFRef<T> CFRef<T>::adopt(T _Nullable object CF_RELEASES_ARGUMENT) noexcept
{
	return CFRef{object};
}

template <typename T>
inline CFRef<T> CFRef<T>::retain(T _Nullable object) noexcept
{
	return CFRef{object, retain_ref};
}

// MARK: Creation and Destruction

template <typename T>
inline CFRef<T>::CFRef(std::nullptr_t) noexcept
{}

template <typename T>
inline CFRef<T>::CFRef(T _Nullable object CF_RELEASES_ARGUMENT) noexcept
: object_{object}
{}

template <typename T>
inline CFRef<T>::CFRef(T _Nullable object, retain_ref_t) noexcept
: object_{object ? (T)CFRetain(object) : nullptr}
{}

template <typename T>
inline CFRef<T>::CFRef(const CFRef& other) noexcept
: CFRef{other.object_, retain_ref}
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

// MARK: Core Foundation Object Management

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
inline T _Nullable CFRef<T>::get() const & noexcept
{
	return object_;
}

template <typename T>
inline T _Nullable * _Nonnull CFRef<T>::put() & noexcept
{
	reset();
	return &object_;
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

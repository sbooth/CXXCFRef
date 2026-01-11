//
// SPDX-FileCopyrightText: 2026 Stephen F. Booth <contact@sbooth.dev>
// SPDX-License-Identifier: MIT
//
// Part of https://github.com/sbooth/CXXCFRef
//

import Testing
@testable import CXXCFRef

@Test func basic() async throws {
	let s = CXXCFRef.CFString()
	#expect(s.__convertToBool() == false)
}

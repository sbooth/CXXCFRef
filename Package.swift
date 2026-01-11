// swift-tools-version: 5.9
//
// SPDX-FileCopyrightText: 2026 Stephen F. Booth <contact@sbooth.dev>
// SPDX-License-Identifier: MIT
//
// Part of https://github.com/sbooth/CXXCFRef
//

import PackageDescription

let package = Package(
	name: "CXXCFRef",
	products: [
		// Products define the executables and libraries a package produces, making them visible to other packages.
		.library(
			name: "CXXCFRef",
			targets: [
				"CXXCFRef",
			]
		),
	],
	targets: [
		// Targets are the basic building blocks of a package, defining a module or a test suite.
		// Targets can depend on other targets in this package and products from dependencies.
		.target(
			name: "CXXCFRef",
			cSettings: [
				.headerSearchPath("include/CXXCFRef"),
			]
		),
		.testTarget(
			name: "CXXCFRefTests",
			dependencies: [
				"CXXCFRef",
			],
			swiftSettings: [
				.interoperabilityMode(.Cxx),
			]
		),
	],
	cxxLanguageStandard: .cxx17
)

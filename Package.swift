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
        .library(
            name: "CXXCFRef",
            targets: [
                "CXXCFRef",
            ]
        ),
    ],
    targets: [
        .target(
            name: "CXXCFRef",
            cSettings: [
                .headerSearchPath("include/cf"),
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

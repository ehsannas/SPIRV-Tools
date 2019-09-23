COMMON_COPTS = [
        "-DSPIRV_CHECK_CONTEXT",
        "-DSPIRV_COLOR_TERMINAL",
    ] + select({
    "@bazel_tools//src/conditions:windows": [""],
    "//conditions:default": [
        "-DSPIRV_LINUX",
        "-DSPIRV_TIMER_ENABLED",
        "-Wall",
        "-Wextra",
        "-Wnon-virtual-dtor",
        "-Wno-missing-field-initializers",
        "-Werror",
        "-std=c++11",
        "-fvisibility=hidden",
        "-fno-exceptions",
        "-fno-rtti",
        "-Wno-long-long",
        "-Wshadow",
        "-Wundef",
        "-Wconversion",
        "-Wno-sign-conversion",
    ],
})

TEST_COPTS = COMMON_COPTS + select({
    "@bazel_tools//src/conditions:windows": [""],
    "//conditions:default": [
        "-Wno-undef",
        "-Wno-self-assign",
        "-Wno-shadow",
        "-Wno-unused-parameter"
    ],
})

DEBUGINFO_GRAMMAR_JSON_FILE = "source/extinst.debuginfo.grammar.json"

def generate_core_tables(version = None):
    if not version:
        fail("Must specify version", "version")
    grammars = [
        "@spirv_headers//:spirv_core_grammar_" + version,
        DEBUGINFO_GRAMMAR_JSON_FILE,
    ]
    outs = [
        "core.insts-{}.inc".format(version),
        "operand.kinds-{}.inc".format(version),
    ]
    fmtargs = grammars + outs
    native.genrule(
        name = "gen_core_tables_" + version,
        srcs = grammars,
        outs = outs,
        cmd = (
            "$(location :generate_grammar_tables) " +
            "--spirv-core-grammar=$(location {0}) " +
            "--extinst-debuginfo-grammar=$(location {1}) " +
            "--core-insts-output=$(location {2}) " +
            "--operand-kinds-output=$(location {3})"
        ).format(*fmtargs),
        tools = [":generate_grammar_tables"],
        visibility = ["//visibility:private"],
    )

def generate_enum_string_mapping(version = None):
    if not version:
        fail("Must specify version", "version")
    grammars = [
        "@spirv_headers//:spirv_core_grammar_" + version,
        DEBUGINFO_GRAMMAR_JSON_FILE,
    ]
    outs = [
        "extension_enum.inc",
        "enum_string_mapping.inc",
    ]
    fmtargs = grammars + outs
    native.genrule(
        name = "gen_enum_string_mapping",
        srcs = grammars,
        outs = outs,
        cmd = (
            "$(location :generate_grammar_tables) " +
            "--spirv-core-grammar=$(location {0}) " +
            "--extinst-debuginfo-grammar=$(location {1}) " +
            "--extension-enum-output=$(location {2}) " +
            "--enum-string-mapping-output=$(location {3})"
        ).format(*fmtargs),
        tools = [":generate_grammar_tables"],
        visibility = ["//visibility:private"],
    )

def generate_opencl_tables(version = None):
    if not version:
        fail("Must specify version", "version")
    grammars = [
        "@spirv_headers//:spirv_opencl_grammar_" + version,
    ]
    outs = ["opencl.std.insts.inc"]
    fmtargs = grammars + outs
    native.genrule(
        name = "gen_opencl_tables_" + version,
        srcs = grammars,
        outs = outs,
        cmd = (
            "$(location :generate_grammar_tables) " +
            "--extinst-opencl-grammar=$(location {0}) " +
            "--opencl-insts-output=$(location {1})"
        ).format(*fmtargs),
        tools = [":generate_grammar_tables"],
        visibility = ["//visibility:private"],
    )

def generate_glsl_tables(version = None):
    if not version:
        fail("Must specify version", "version")
    grammars = [
        "@spirv_headers//:spirv_glsl_grammar_" + version,
    ]
    outs = ["glsl.std.450.insts.inc"]
    fmtargs = grammars + outs
    native.genrule(
        name = "gen_glsl_tables_" + version,
        srcs = grammars,
        outs = outs,
        cmd = (
            "$(location :generate_grammar_tables) " +
            "--extinst-glsl-grammar=$(location {0}) " +
            "--glsl-insts-output=$(location {1})"
        ).format(*fmtargs),
        tools = [":generate_grammar_tables"],
        visibility = ["//visibility:private"],
    )

def generate_vendor_tables(extension = None):
    if not extension:
        fail("Must specify extension", "extension")
    extension_rule = extension.replace("-", "_")
    grammars = ["source/extinst.{}.grammar.json".format(extension)]
    outs = ["{}.insts.inc".format(extension)]
    fmtargs = grammars + outs
    native.genrule(
        name = "gen_vendor_tables_" + extension_rule,
        srcs = grammars,
        outs = outs,
        cmd = (
            "$(location :generate_grammar_tables) " +
            "--extinst-vendor-grammar=$(location {0}) " +
            "--vendor-insts-output=$(location {1})"
        ).format(*fmtargs),
        tools = [":generate_grammar_tables"],
        visibility = ["//visibility:private"],
    )

def generate_extinst_lang_headers(name, grammar = None):
    if not grammar:
        fail("Must specify grammar", "grammar")
    fmtargs = [name]
    native.genrule(
        name = "gen_extinst_lang_headers_" + name,
        srcs = [grammar],
        outs = [name + ".h"],
        cmd = (
            "$(location :generate_language_headers) " +
            "--extinst-name={0} " +
            "--extinst-grammar=$< " +
            "--extinst-output-base=$(@D)/{0}"
        ).format(*fmtargs),
        tools = [":generate_language_headers"],
        visibility = ["//visibility:private"],
    )

def base_test(name, srcs, deps = []):
    native.cc_test(
        name = "base_" + name + "_test",
        srcs = srcs,
        compatible_with = [],
        copts = TEST_COPTS,
        size = "small",
        deps = [
            ":test_common",
            "@com_google_googletest//:gtest_main",
            "@com_google_googletest//:gtest",
            "@com_google_effcee//:effcee",
        ] + deps,
    )

def link_test(name, srcs, deps = []):
    native.cc_test(
        name = "link_" + name + "_test",
        srcs = srcs,
        compatible_with = [],
        copts = TEST_COPTS,
        size = "small",
        deps = [
            ":link_test_common",
            "@com_google_googletest//:gtest_main",
            "@com_google_googletest//:gtest",
            "@com_google_effcee//:effcee",
        ] + deps,
    )

def opt_test(name, srcs, deps = []):
    native.cc_test(
        name = "opt_" + name + "_test",
        srcs = srcs,
        compatible_with = [],
        copts = TEST_COPTS,
        size = "small",
        deps = [
            ":opt_test_common",
            "@com_google_googletest//:gtest_main",
            "@com_google_googletest//:gtest",
            "@com_google_effcee//:effcee",
        ] + deps,
    )

def reduce_test(name, srcs, deps = []):
    native.cc_test(
        name = "reduce_" + name + "_test",
        srcs = srcs,
        compatible_with = [],
        copts = TEST_COPTS,
        size = "small",
        deps = [
            ":reduce_test_common",
            ":spirv_tools_reduce",
            "@com_google_googletest//:gtest_main",
            "@com_google_googletest//:gtest",
            "@com_google_effcee//:effcee",
        ] + deps,
    )

def util_test(name, srcs, deps = []):
    native.cc_test(
        name = "util_" + name + "_test",
        srcs = srcs,
        compatible_with = [],
        copts = TEST_COPTS,
        size = "small",
        deps = [
            ":opt_test_common",
            "@com_google_googletest//:gtest_main",
            "@com_google_googletest//:gtest",
            "@com_google_effcee//:effcee",
        ] + deps,
    )

def val_test(name, srcs = [], size = "small", copts = [], deps = [], **kwargs):
    native.cc_test(
        name = "val_" + name + "_test",
        srcs = srcs,
        compatible_with = [],
        copts = TEST_COPTS + copts,
        size = size,
        deps = [
            ":val_test_common",
            "@com_google_googletest//:gtest_main",
            "@com_google_googletest//:gtest",
            "@com_google_effcee//:effcee",
        ] + deps,
        **kwargs
    )

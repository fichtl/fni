"Utilities for third party packages."

load("@bazel_tools//tools/build_defs/repo:utils.bzl", "patch", "workspace_and_buildfile")

# NOTE: usable when compiling with cmake
all_content = """filegroup(
    name = "all",
    srcs = glob(["**"]),
    visibility = ["//visibility:public"],
)
"""

def _to_char(n):
    alpha = "0123456789ABCDEF"
    return alpha[n]

def hex(number):
    """Format integer to hexdecimal representation

    Args:
        number: number to format

    Returns:
        hexdecimal representation of the number argument
    """

    hex_string = ""
    is_signed = number < 0
    r = number * -1 if is_signed else number
    for _ in range(1000000):
        if r > 0:
            rem = r % 16
            hex_string = _to_char(rem) + hex_string
            r //= 16
        else:
            break

    if not bool(hex_string):
        hex_string = "0"

    if len(hex_string) % 2 != 0:
        hex_string = "0" + hex_string

    return "{}{}".format("-" if is_signed else "", hex_string)

def _make_identifier(s):
    result = ""
    for i in range(len(s)):
        result += s[i] if s[i].isalnum() else "_"
    return result

# Defines the implementation actions to generate_export_header.
def _generate_export_header_impl(ctx):
    output = ctx.outputs.out

    guard = _make_identifier(output.basename.upper())

    content = [
        "#ifndef %s" % guard,
        "#define %s" % guard,
        "",
        "#ifdef %s" % ctx.attr.static_define,
        "#  define %s" % ctx.attr.export_macro_name,
        "#  define %s" % ctx.attr.no_export_macro_name,
        "#else",
        "#  define %s __attribute__((visibility(\"default\")))" % ctx.attr.export_macro_name,  # noqa
        "#  define %s __attribute__((visibility(\"hidden\")))" % ctx.attr.no_export_macro_name,  # noqa
        "#endif",
        "",
        "#ifndef %s" % ctx.attr.deprecated_macro_name,
        "#  define %s __attribute__ ((__deprecated__))" % ctx.attr.deprecated_macro_name,  # noqa
        "#endif",
        "",
        "#ifndef %s" % ctx.attr.export_deprecated_macro_name,
        "#  define %s %s %s" % (ctx.attr.export_deprecated_macro_name, ctx.attr.export_macro_name, ctx.attr.deprecated_macro_name),  # noqa
        "#endif",
        "",
        "#ifndef %s" % ctx.attr.no_export_deprecated_macro_name,
        "#  define %s %s %s" % (ctx.attr.no_export_deprecated_macro_name, ctx.attr.no_export_macro_name, ctx.attr.deprecated_macro_name),  # noqa
        "#endif",
        "",
        "#endif",
    ]

    ctx.actions.write(output = output, content = "\n".join(content) + "\n")

# Defines the rule to generate_export_header.
_generate_export_header_gen = rule(
    attrs = {
        "out": attr.output(mandatory = True),
        "export_macro_name": attr.string(),
        "deprecated_macro_name": attr.string(),
        "export_deprecated_macro_name": attr.string(),
        "no_export_macro_name": attr.string(),
        "no_export_deprecated_macro_name": attr.string(),
        "static_define": attr.string(),
    },
    output_to_genfiles = True,
    implementation = _generate_export_header_impl,
)

def generate_export_header(
        lib = None,
        name = None,
        out = None,
        export_macro_name = None,
        deprecated_macro_name = None,
        export_deprecated_macro_name = None,
        no_export_macro_name = None,
        no_export_deprecated_macro_name = None,
        static_define = None,
        **kwargs):
    """Creates a rule to generate an export header for a named library.

    This is an incomplete implementation of CMake's generate_export_header. (In
    particular, it assumes a platform that uses __attribute__((visibility("default"))) to
    decorate exports.)

    By default, the rule will have a mangled name related to the library name, and will
    produce "<lib>_export.h".

    The CMake documentation of the generate_export_header macro is:
    https://cmake.org/cmake/help/latest/module/GenerateExportHeader.html

    Args:
        lib: library name
        name: bazel target name
        out: output file path
        export_macro_name: its name explains itself
        deprecated_macro_name: its name explains itself
        export_deprecated_macro_name: its name explains itself
        no_export_macro_name: its name explains itself
        no_export_deprecated_macro_name: its name explains itself
        static_define: its name explains itself
        **kwargs: keyward args passed to `_generate_export_header_gen`

    Returns:
        same exported headers as CMake GenerateExportHeader
    """

    if name == None:
        name = "__%s_export_h" % lib
    if out == None:
        out = "%s_export.h" % lib
    if export_macro_name == None:
        export_macro_name = "%s_EXPORT" % lib.upper()
    if deprecated_macro_name == None:
        deprecated_macro_name = "%s_DEPRECATED" % lib.upper()
    if export_deprecated_macro_name == None:
        export_deprecated_macro_name = "%s_DEPRECATED_EXPORT" % lib.upper()
    if no_export_macro_name == None:
        no_export_macro_name = "%s_NO_EXPORT" % lib.upper()
    if no_export_deprecated_macro_name == None:
        no_export_deprecated_macro_name = \
            "%s_DEPRECATED_NO_EXPORT" % lib.upper()
    if static_define == None:
        static_define = "%s_STATIC_DEFINE" % lib.upper()

    _generate_export_header_gen(
        name = name,
        out = out,
        export_macro_name = export_macro_name,
        deprecated_macro_name = deprecated_macro_name,
        export_deprecated_macro_name = export_deprecated_macro_name,
        no_export_macro_name = no_export_macro_name,
        no_export_deprecated_macro_name = no_export_deprecated_macro_name,
        static_define = static_define,
        **kwargs
    )

def _http_archive_with_extra_files(ctx):
    ctx.download_and_extract(
        url = ctx.attr.url,
        sha256 = ctx.attr.sha256,
        stripPrefix = ctx.attr.strip_prefix,
    )
    workspace_and_buildfile(ctx)
    patch(ctx)
    for f in ctx.attr.extra_files:
        ctx.file(f.name, content = ctx.read(f), executable = False)

http_archive_with_extra_headers = repository_rule(
    implementation = _http_archive_with_extra_files,
    attrs = {
        "url": attr.string(),
        "sha256": attr.string(),
        "strip_prefix": attr.string(),
        "patches": attr.label_list(),
        "patch_args": attr.string_list(default = ["-p0"]),
        "build_file": attr.label(allow_single_file = True),
        "build_file_content": attr.string(),
        "extra_files": attr.label_list(),
        "workspace_file": attr.label(),
        "workspace_file_content": attr.string(),
    },
)

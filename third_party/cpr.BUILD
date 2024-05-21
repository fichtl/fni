load("@dni//third_party:utils.bzl", "hex")

package(default_visibility = ["//visibility:public"])

licenses(["notice"])

cpr_version = "1.10.5"

cpr_vers = cpr_version.split(".")

cpr_version_num = hex(int(cpr_vers[0])) + hex(int(cpr_vers[1])) + hex(int(cpr_vers[2]))

genrule(
    name = "gen_cprver_h",
    srcs = ["cmake/cprver.h.in"],
    outs = ["cpr/cprver.h"],
    cmd = """awk '{
gsub("\\\\$$\\\\{cpr_VERSION\\\\}", """ + cpr_version + """);
gsub("\\\\$$\\\\{cpr_VERSION_MAJOR\\\\}", """ + cpr_vers[0] + """);
gsub("\\\\$$\\\\{cpr_VERSION_MINOR\\\\}", """ + cpr_vers[1] + """);
gsub("\\\\$$\\\\{cpr_VERSION_PATCH\\\\}", """ + cpr_vers[2] + """);
gsub("\\\\$$\\\\{cpr_VERSION_NUM\\\\}", "0x""" + cpr_version_num + """");
print; }' $(<) > $(@)""",
)

cc_library(
    name = "cpr",
    srcs = glob(["cpr/**/*.cpp"]),
    hdrs = [":gen_cprver_h"] + glob([
        "include/cpr/**/*.h",
    ]),
    includes = ["include"],
    linkopts = ["-lcurl"],
)

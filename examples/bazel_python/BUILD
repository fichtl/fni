licenses(["notice"])

py_binary(
    name = "main",
    srcs = ["dni.py"],
    main = "dni.py",
)

[py_test(
    name = test_file[:-len(".py")],
    srcs = [
        test_file,
    ],
) for test_file in glob(["test_*.py"])]

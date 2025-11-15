set_languages("cxxlatest")

add_rules("mode.debug", "mode.release")
add_includedirs(".")
set_rundir(".")

add_requires("gtest")

target("grammar")
    set_kind("binary")
    add_files(
        "main.cpp",
        "grammar.cpp"
    )


target("grammar.test")
    set_kind("binary")
    add_files("./test/grammar.cpp", "grammar.cpp")
    add_packages("gtest")

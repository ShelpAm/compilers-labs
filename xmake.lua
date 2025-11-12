set_languages("cxxlatest")

target("grammar")
    set_kind("binary")
    add_files("main.cpp", "grammar.cpp")

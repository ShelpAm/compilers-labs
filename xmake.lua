set_languages("cxxlatest")

add_rules("mode.debug", "mode.release")

target("grammar")
    set_kind("binary")
    add_files("main.cpp", "grammar.cpp")

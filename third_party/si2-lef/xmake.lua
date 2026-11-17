-- xmake.lua
add_rules("mode.debug", "mode.release")

-- 定义静态库目标
target("si2-lef")
    set_kind("static")
    set_targetdir("../../target/library")
    before_build(function (target)
        local source_path = "third_party/si2-lef/source"
        local lex_y = source_path .. "/lef.y"
        local lex_hpp = source_path .. "/lef.tab.hpp"
        local lex_cpp = source_path .. "/lef.tab.cpp"

        os.exec(string.format("bison -v -p lefyy -d %s -o %s", lex_y, lex_cpp))
    end)
    add_files("source/*.cpp")
    add_includedirs("source")
add_rules("mode.debug", "mode.release")
set_languages("c99", "c++20")


includes("third_party/si2-lef")


target("xtaro_compiler")
    set_kind("binary")
    set_targetdir("./target")

    add_includedirs("source", "source/global")
    add_includedirs("third_party/argparse")
    add_files(
        "source/characterize/**.cc",
        "source/circuit/**.cc",
        "source/dump/**.cc",
        "source/global/**.cc",
        "source/parse/**.cc",
        "source/utility/**.cc",
        "source/verify/**.cc",
        "source/app/compiler/**.cc",
        "source/app/constant.cc"
    )
    
    add_linkdirs("target/library")
    add_links("si2-lef")
    add_deps("si2-lef")

    
target("xtaro_creator")
    set_kind("binary")
    set_targetdir("target")

    add_includedirs("source", "source/global")
    add_includedirs("third_party/argparse")
    add_files(
        "source/characterize/**.cc",
        "source/circuit/**.cc",
        "source/dump/**.cc",
        "source/global/**.cc",
        "source/parse/**.cc",
        "source/utility/**.cc",
        "source/verify/**.cc",
        "source/app/creator/**.cc",
        "source/app/constant.cc"
    )
    
    add_linkdirs("target/library")
    add_links("si2-lef")
    add_deps("si2-lef")


-- target("xtaro_gui")
--     set_default(false)
--     set_kind("binary")
--     set_targetdir("./target")

--     add_includedirs("compiler")
--     add_files("compiler/**.cc")
--     add_files("compiler/app/gui/**.h")
--     remove_files("compiler/app/cli/**.cc")

--     add_rules("qt.widgetapp")
    
--     add_linkdirs("library")
--     add_links("si2-lef")
--     add_deps("si2-lef")


target("gdsii")
    set_default(false)
    set_kind("binary")
    set_targetdir("./target")

    add_includedirs("source")
    add_files("source/parse/gdsii/**.cc", "tools/gdsii.cc")
    add_files("source/utility/**.cc")

-- xmake project -k compile_commands
-- xmake f --mode=debug
-- xmake f -k clang

-- add_rules("mode.debug")

--
-- If you want to known more usage about xmake, please see https://xmake.io
--
-- ## FAQ
--
-- You can enter the project directory firstly before building project.
--
--   $ cd projectdir
--
-- 1. How to build project?
--
--   $ xmake
--
-- 2. How to configure project?
--
--   $ xmake f -p [macosx|linux|iphoneos ..] -a [x86_64|i386|arm64 ..] -m [debug|release]
--
-- 3. Where is the build output directory?
--
--   The default output directory is `./build` and you can configure the output directory.
--
--   $ xmake f -o outputdir
--   $ xmake
--
-- 4. How to run and debug target after building project?
--
--   $ xmake run [targetname]
--   $ xmake run -d [targetname]
--
-- 5. How to install target to the system directory or other output directory?
--
--   $ xmake install
--   $ xmake install -o installdir
--
-- 6. Add some frequently-used compilation flags in xmake.lua
--
-- @code
--    -- add debug and release modes
--    add_rules("mode.debug", "mode.release")
--
--    -- add macro definition
--    add_defines("NDEBUG", "_GNU_SOURCE=1")
--
--    -- set warning all as error
--    set_warnings("all", "error")
--
--    -- set language: c99, c++11
--    set_languages("c99", "c++11")
--
--    -- set optimization: none, faster, fastest, smallest
--    set_optimize("fastest")
--
--    -- add include search directories
--    add_includedirs("/usr/include", "/usr/local/include")
--
--    -- add link libraries and search directories
--    add_links("tbox")
--    add_linkdirs("/usr/local/lib", "/usr/lib")
--
--    -- add system link libraries
--    add_syslinks("z", "pthread")
--
--    -- add compilation and link flags
--    add_cxflags("-stdnolib", "-fno-strict-aliasing")
--    add_ldflags("-L/usr/local/lib", "-lpthread", {force = true})
--
-- @endcode
--


local PROJECTION_DIR = os.curdir()
local RUNTIME_DIR = path.join(PROJECTION_DIR, "Runtime")


set_toolset("cc", "clang-cl")
set_toolset("cxx", "clang-cl")
add_cxxflags("-std:c++20", { tools = { "clang-cl" }})
add_defines("__cpp_consteval", { tools = { "clang-cl" }})
add_defines("NOMINMAX", "UNICODE", "_UNICODE")
add_rules("mode.debug", "mode.release", "mode.releasedbg")
set_arch("x64")


-- requires packages
add_requires("fmt")
add_requires("spdlog")
add_requires("imgui")
add_requires("glfw")
add_requires("vulkan-hpp")

target("VulkanApp")
    set_languages("c++latest")
    set_warnings("all")
    set_kind("binary")
    add_headerfiles("**.h")
    add_headerfiles("**.hpp")
    add_headerfiles("**.inc")
    add_files("**.cpp")
    add_includedirs(RUNTIME_DIR)
    add_defines("PLATFORM_WIN")

    -- dependent packages
    add_packages("fmt")
    add_packages("spdlog")
    add_packages("imgui")
    add_packages("glfw")
    add_packages("vulkan-hpp")
target_end()
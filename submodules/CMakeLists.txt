# ---------------------------
# modules that already have cmake lists
# ---------------------------
add_subdirectory(fmt)
add_subdirectory(json)

# ---------------------------
# imgui
# ---------------------------
file(GLOB IMGUI_SOURCES CONFIGURE_DEPENDS "imgui/*.h" "imgui/*.cpp")
add_library(imgui STATIC
    ${IMGUI_SOURCES}
    imgui/backends/imgui_impl_opengl.cpp
    imgui/backends/imgui_impl_opengl.h
    imgui/backends/imgui_impl_win32.cpp
    imgui/backends/imgui_impl_win32.h)
target_compile_options(imgui PRIVATE /W4 /WX /Zi /MT /Gy /Gm- /Zl /Od)
target_include_directories(imgui PUBLIC imgui)
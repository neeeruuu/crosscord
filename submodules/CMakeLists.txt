# ---------------------------
# modules that already have cmake lists
# ---------------------------
add_subdirectory(fmt)
add_subdirectory(json)
add_subdirectory(glfw)
add_subdirectory(miniz)

# ---------------------------
# imgui
# ---------------------------
file(GLOB IMGUI_SOURCES CONFIGURE_DEPENDS "imgui/*.h" "imgui/*.cpp")
add_library(imgui STATIC
    ${IMGUI_SOURCES}
    imgui/backends/imgui_impl_opengl3.cpp
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_glfw.h
    imgui/backends/imgui_impl_win32.cpp
    imgui/backends/imgui_impl_win32.h)
target_link_libraries(imgui PRIVATE glfw)
target_compile_options(imgui PRIVATE /W4 /WX /Zi /MT /Gy /Gm- /Zl /Od)
target_include_directories(imgui SYSTEM PUBLIC imgui)

# ---------------------------
# imgui-filebrowser
# ---------------------------
add_library(imgui_filebrowser INTERFACE)
target_include_directories(imgui_filebrowser SYSTEM INTERFACE "imgui-filebrowser")

# ---------------------------
# libspng
# ---------------------------
add_library(libspng STATIC libspng/spng/spng.c libspng/spng/spng.h)
target_link_libraries(libspng PRIVATE miniz)
target_include_directories(libspng SYSTEM PUBLIC libspng/spng)
add_compile_definitions(SPNG_USE_MINIZ)
# add_subdirectory(libspng)
# add_compile_options("SPNG_USE_MINIZ")
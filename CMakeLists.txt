# ------------------------------------------------------
# set up cmake
# ------------------------------------------------------
cmake_minimum_required(VERSION 3.24)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "/EHsc /D_AMD64_ /DWIN32_LEAN_AND_MEAN") # Set C++ exception handler, define platform and remove some unused headers

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(crosscord LANGUAGES CXX ASM_MASM)

add_subdirectory(submodules)

# ------------------------------------------------------
# dll
# ------------------------------------------------------
file(GLOB_RECURSE crosscordsources CONFIGURE_DEPENDS "src/crosscord/*.cpp")
file(GLOB_RECURSE crosscord_headers CONFIGURE_DEPENDS "src/crosscord/*.h")

add_executable(crosscord SHARED ${crosscord_sources} ${crosscord_headers})
target_link_libraries(crosscord PRIVATE
    nlohmann_json
    imgui)
target_include_directories(crosscord PRIVATE "src/crosscord")
set_target_properties(crosscord PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:crosscord>
    OUTPUT_NAME "crosscord"
)

# ------------------------------------------------------
# merge files from source and include them in the ide
# ------------------------------------------------------
function(group_files sources)
	foreach(FILE ${sources})
		get_filename_component(PARENT_DIR "${FILE}" DIRECTORY)

		string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" GROUP "${PARENT_DIR}") # remove full dir
		string(REGEX MATCH "[(A-z)|(a-z)|(0-9)]+/[(A-z)|(a-z)|(0-9)]+" GROUPNAME ${GROUP}) # get the first two folders from dir
		
		string(LENGTH ${GROUPNAME} GROUPLEN) # get project name len
		string(SUBSTRING ${GROUP} ${GROUPLEN} -1 GROUP) # remove project name from dir

		source_group("${GROUP}" FILES "${FILE}") # group file
	endforeach()
endfunction()
group_files("${crosscord_sources};${crosscord_headers}")

# ------------------------------------------------------
# set output directory
# ------------------------------------------------------
set_target_properties(crosscord PROPERTIES RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_CURRENT_BINARY_DIR}/build")
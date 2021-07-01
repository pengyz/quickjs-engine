cmake_minimum_required(VERSION 3.12.5)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/third_party/vcpkg/scripts/buildsystems/vcpkg.cmake)
set(VCPKG_TARGET_TRIPLET x64-linux CACHE STRING "vcpkg triplet")

project(quickjsEngine)

# 设置属性
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 17)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/${PROJECT_ARCH})
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/${PROJECT_ARCH})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${PROJECT_ARCH})

file(GLOB _HEADE_FILES "*.h")
source_group("Header Files", FILES ${_HEADER_FILES})
file(GLOB _SOURCE_FILES "*.cpp")
source_group("Source Files", FILES ${_SOURCE_FILES})

file(GLOB _JS_MODULE_HEADERS "js_module/*.h")
source_group("JsModule Headers")
file(GLOB _JS_MODULE_SOURCES "js_module/*.cpp")
source_group("JSModule Sources")

find_package(imgui CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(unofficial-libuv CONFIG REQUIRED)
find_package(rttr CONFIG REQUIRED)

add_subdirectory(third_party/quickjs-cmake)

include_directories(
  ${quickjs_sources_root}
)

add_executable(${PROJECT_NAME}
  ${_HEADE_FILES}
  ${_SOURCE_FILES}
  ${_JS_MODULE_HEADERS}
  ${_JS_MODULE_SOURCES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  imgui::imgui
  glfw
  quickjs
  unofficial::libuv::libuv
  RTTR::Core RTTR::Core_Lib
)

cmake_minimum_required(VERSION 3.5)
project(hippo_common)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)

# include_directories(
#   include/${PROJECT_NAME}
# )

set(lib_name param_utils)
# ament_auto_add_library(${PROJECT_NAME} SHARED src/param_utils.cpp)
add_library(${lib_name} src/param_utils.cpp)
target_include_directories(${lib_name}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(${lib_name} rclcpp)
ament_export_targets(${lib_name}Target HAS_LIBRARY_TARGET)
ament_export_dependencies(rclcpp)

install(
  DIRECTORY include/
  DESTINATION include
)

install(TARGETS ${lib_name}
  EXPORT ${lib_name}Target
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)

ament_package()

project(config-base)

include_directories("include" "deps")

source_group("Source" "example/main.cpp")
source_group("Include" "include/config-base.h")

add_executable(${PROJECT_NAME} "example/main.cpp")
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
)

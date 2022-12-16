# --- GLAD ---

add_library(glad STATIC)
target_sources(glad PRIVATE glad/src/glad.c)
target_include_directories(glad PUBLIC glad/include)

# --- GLFW ---

add_subdirectory(glfw)
include_directories(glfw/include)

# --- SPDLOG ---

add_subdirectory(spdlog)
include_directories(spdlog/include)

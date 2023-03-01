link_libraries(gl3w glfw bu_glw)

add_executable(mandelbrot mandelbrot/mandelbrot.cpp)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mandelbrot/vert.glsl vert.glsl COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mandelbrot/frag.glsl frag.glsl COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mandelbrot/fragJ.glsl fragJ.glsl COPYONLY)

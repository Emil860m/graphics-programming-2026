
set(libraries glad glfw assimp imgui itugl ${APPLE_LIBRARIES})

file(GLOB_RECURSE target_inc "*.h" )
file(GLOB_RECURSE target_src "*.cpp" )

file(GLOB_RECURSE shaders "*.vert" "*.frag" "*.geom" "*.glsl")
source_group("Shaders" FILES ${shaders})

FILE(COPY shaders DESTINATION ${CMAKE_BINARY_DIR}/exercises/exercise05)
FILE(COPY models DESTINATION ${CMAKE_BINARY_DIR}/exercises/exercise05)
add_executable(${TARGETNAME} ${target_inc} ${target_src} ${shaders})
target_link_libraries(${TARGETNAME}
    ${libraries}
    X11
    pthread
    dl
)

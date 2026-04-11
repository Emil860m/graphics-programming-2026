
set(libraries glad glfw itugl ${APPLE_LIBRARIES})

file(GLOB_RECURSE target_inc "*.h" )
file(GLOB_RECURSE target_src "*.cpp" )

file(GLOB_RECURSE shaders "*.vert" "*.frag" "*.geom" "*.glsl")
source_group("Shaders" FILES ${shaders})
FILE(COPY shaders DESTINATION ${CMAKE_BINARY_DIR}/exercises/exercise04)
FILE(COPY textures DESTINATION ${CMAKE_BINARY_DIR}/exercises/exercise04)


add_executable(${TARGETNAME} ${target_inc} ${target_src} ${shaders})
target_link_libraries(${TARGETNAME}
    ${libraries}
    X11
    pthread
    dl
)

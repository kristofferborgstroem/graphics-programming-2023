
set(libraries glad glfw itugl ${APPLE_LIBRARIES})

file(GLOB_RECURSE target_inc "*.h" )
file(GLOB_RECURSE target_src "*.cpp" )

add_executable(${TARGETNAME} ${target_inc} ${target_src} Particle.cpp Particle.h)
target_link_libraries(${TARGETNAME} ${libraries})

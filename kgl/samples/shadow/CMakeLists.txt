set(EXTRAS color.glsl pcss.glsl shader.glsl shadow.glsl shadowgen.glsl tex.glsl esm.glsl)
add_executable(shadow WIN32 main.cpp ${EXTRAS})
set_target_properties(shadow PROPERTIES RUNTIME_OUTPUT_DIRECTORY .)
set_target_properties(shadow PROPERTIES OUTPUT_NAME shadow)
set(SHADOW_LIBS kgl)

target_link_libraries(shadow ${SHADOW_LIBS})

set(EXTRAS render.glsl ssao.glsl prepass.glsl filter.glsl fastssao.glsl hbao.glsl blurcompute.glsl)
add_executable(ao WIN32 main.cpp ${EXTRAS})
set_target_properties(ao PROPERTIES RUNTIME_OUTPUT_DIRECTORY .)
set_target_properties(ao PROPERTIES OUTPUT_NAME ao)
set(AO_LIBS kgl)

target_link_libraries(ao ${AO_LIBS})

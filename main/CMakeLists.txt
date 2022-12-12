idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS ".")

target_compile_features(${COMPONENT_LIB} PUBLIC c_std_23)

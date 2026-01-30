set(INCLUDES "./" "./App" "./UI" "./UI/custom" "./UI/generated" "./UI/generated/guider_customer_fonts")
file(GLOB_RECURSE SOURCES "./*.c")

idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS ${INCLUDES})
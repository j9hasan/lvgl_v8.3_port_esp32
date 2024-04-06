SET(SOURCES
    ui.c
    )
idf_component_register(SRCS ${SOURCES}
                       INCLUDE_DIRS .
                       REQUIRES lvgl sd_fat
                       )

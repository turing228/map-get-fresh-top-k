project(MapWithGetVeryFrequent)

#add_executable(MapWithGetVeryFrequent_lib map_with_get_very_frequent.h)
#add_library(MapWithGetVeryFrequent_lib map_with_get_very_frequent.h)

set(HEADER_FILES
        map_get_fresh_top_k.h
        frequency_estimation_analyzer.h
        )

set(SOURCE_FILES
        )

add_library(map_with_get_very_frequent_lib STATIC ${SOURCE_FILES} ${HEADER_FILES})

set_target_properties(map_with_get_very_frequent_lib PROPERTIES LINKER_LANGUAGE CXX)
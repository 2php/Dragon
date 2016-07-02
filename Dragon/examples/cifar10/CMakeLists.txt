set(TARGET_FILE convert_cifar10.cpp)
set(SRC_FILES ../../src/protos/dragon.pb.cc ../../src/utils/logging.cpp ../../src/utils/flags.cpp)
add_executable(ConvertCifar ${TARGET_FILE} ${SRC_FILES})

FILE(GLOB targets ${3RDPARTY_LIBS}/*.so ${3RDPARTY_LIBS}/*.a ${3RDPARTY_LIBS}/*.lib) 
foreach(target ${targets})
	TARGET_LINK_LIBRARIES(ConvertCifar  ${target})
endforeach()

# ---[ link others
if(OS_LINUX)
	TARGET_LINK_LIBRARIES(ConvertCifar pthread)
endif()
if(OS_WINDOWS)
	TARGET_LINK_LIBRARIES(ConvertCifar  shlwapi.lib)
endif()


set_target_properties(ConvertCifar PROPERTIES OUTPUT_NAME convert_cifar)

# ---[ install
install (TARGETS ConvertCifar DESTINATION bin)


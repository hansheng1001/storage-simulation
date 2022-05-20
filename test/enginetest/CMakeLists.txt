add_executable(EventTest EventTest.cpp)
add_executable(eventLoopTest EventLoopTest.cpp)

SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/plugin)

target_link_libraries(EventTest Engine gtest pthread)
target_link_libraries(eventLoopTest Engine gtest pthread)

#这里要生成动态链接库

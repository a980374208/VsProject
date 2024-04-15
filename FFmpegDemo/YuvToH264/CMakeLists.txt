cmake_policy(SET CMP0015 NEW)
project(encoder)
add_executable(encoder)
if(MSVC)
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT encoder)
	message(STATUS "Setting compile options for encoder")
	set_target_properties(encoder PROPERTIES COMPILE_FLAGS "/TP" ) 
endif()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/Release/)

set(avformat "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/lib/avformat.lib")
set(avcodec "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/lib/avcodec.lib")
set(avutil "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/lib/avutil.lib")
target_link_libraries(
  encoder
  PRIVATE 
		  avformat
		  avcodec
          avutil)
target_include_directories(encoder PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}/../third_party/include
	)
target_link_directories(encoder PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/lib
)

set(FFMPEG_DLL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/bin/")

# 构建后事件，将 DLL 复制到可执行文件目录
file(COPY ${FFMPEG_DLL_PATH}
     DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}")

target_sources(
    encoder PRIVATE main.c)
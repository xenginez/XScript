cmake_minimum_required(VERSION 3.8)

project ("XScript")

include(fetchcontent)

fetchcontent_declare(
	stb
	GIT_REPOSITORY https://github.com/nothings/stb.git
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/stb
)
fetchcontent_declare(
	uchardet
	GIT_REPOSITORY https://github.com/BYVoid/uchardet.git
	GIT_TAG v0.0.5
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/uchardet
)
fetchcontent_declare(
	win-iconv
	GIT_REPOSITORY https://github.com/win-iconv/win-iconv.git
	GIT_TAG v0.0.8
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/win-iconv
)
fetchcontent_declare(
	concurrentqueue
	GIT_REPOSITORY https://github.com/cameron314/concurrentqueue.git
	GIT_TAG v1.0.4
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/concurrentqueue
)

fetchcontent_makeavailable(stb)
fetchcontent_makeavailable(uchardet)
fetchcontent_makeavailable(win-iconv)
fetchcontent_makeavailable(concurrentqueue)

add_subdirectory(src)
add_subdirectory(tools)
add_subdirectory(tests)

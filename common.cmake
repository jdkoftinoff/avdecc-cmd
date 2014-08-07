cmake_minimum_required (VERSION 2.8)
project (${PROJECT})
enable_testing()

option(TODO "Enable TODO items that are in progress" OFF)
option(TESTS "Enable building of extended test code in library" ON)
option(EXAMPLES "Enable building of example programs" ON)
option(TOOLS "Enable building of tools" ON)
option(TOOLS_DEV "Enable building of tools-dev" ON)

if(CMAKE_BUILD_TOOL MATCHES "(msdev|devenv|nmake|MSBuild)")
    add_definitions("/W2")
else()
    add_definitions("-Wall -Wextra")
endif()

if(TODO MATCHES "ON")
   add_definitions("-DTODO=1")
   message(STATUS "TODO items that are in progress are enabled for compiling")
endif()


set(LIBS ${LIBS} ${CHECK_LIBRARIES} ${PROJECT})

include_directories( include ${ADDITIONAL_INCLUDE_DIRECTORIES} )

if(TESTS MATCHES "ON")
   message(STATUS "Extended test code functions are enabled")
   file(GLOB PROJECT_INCLUDES "include/*.hpp" "include/tests/*.hpp" "include/*.h" "include/tests/*.h" )
   file(GLOB PROJECT_SRC "src/*.c" "src/test/*.c" "src/*.cpp" "src/test/*.cpp")
else()
   file(GLOB PROJECT_INCLUDES "include/*.h" "include/*.hpp")
   file(GLOB PROJECT_SRC "src/*.c" "src/*.cpp" )
endif()

add_library (${PROJECT} ${PROJECT_SRC} ${PROJECT_INCLUDES})

link_directories( ${PROJECT_BINARY_DIR} )


if(EXAMPLES MATCHES "ON")
    file(GLOB PROJECT_EXAMPLES "examples/*.c" "examples/*.cpp")
    foreach(item ${PROJECT_EXAMPLES})
      GET_FILENAME_COMPONENT(examplename ${item} NAME_WE )
      add_executable(${examplename} ${item})
      target_link_libraries(${examplename} ${LIBS} )
    endforeach(item)
endif()

if(TOOLS MATCHES "ON")
    file(GLOB PROJECT_TOOLS "tools/*.c" "tools/*.cpp")
    foreach(item ${PROJECT_TOOLS})
      GET_FILENAME_COMPONENT(toolname ${item} NAME_WE )
      add_executable(${toolname} ${item})
      target_link_libraries(${toolname} ${LIBS} )
    endforeach(item)
endif()

if(TOOLS_DEV MATCHES "ON")
    file(GLOB PROJECT_TOOLS_DEV "tools-dev/*.c" "tools-dev/*.cpp")
    foreach(item ${PROJECT_TOOLS_DEV})
      GET_FILENAME_COMPONENT(toolname ${item} NAME_WE )
      add_executable(${toolname} ${item})
      target_link_libraries(${toolname} ${LIBS} )
    endforeach(item)
endif()

if(TESTS MATCHES "ON")
   file(GLOB PROJECT_TESTS "tests/*.c" "tests/*.cpp")
   foreach(item ${PROJECT_TESTS})
      GET_FILENAME_COMPONENT(testname ${item} NAME_WE )
      add_executable(${testname} ${item})
      target_link_libraries(${testname} ${LIBS} )
      add_test(NAME ${testname} COMMAND ${testname} )
   endforeach(item)
endif()

INSTALL(TARGETS ${PROJECT} DESTINATION "lib" EXPORT ${PROJECT}-exports.cmake )

EXPORT(TARGETS ${PROJECT} FILE ${PROJECT}-exports.cmake )

INSTALL(FILES
   ${PROJECT_INCLUDES}
   DESTINATION include
)

install(EXPORT ${PROJECT}-exports.cmake DESTINATION "lib"  )


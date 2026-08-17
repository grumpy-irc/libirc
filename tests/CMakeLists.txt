PROJECT(libirc_tests)

SET(CMAKE_AUTOMOC ON)

if(QT6_BUILD)
  find_package(Qt6Core REQUIRED)
  find_package(Qt6Test REQUIRED)
elseif(QT5_BUILD)
  find_package(Qt5Core REQUIRED)
  find_package(Qt5Test REQUIRED)
else()
  message(FATAL_ERROR "Enable QT5_BUILD or QT6_BUILD")
endif()

if(QT6_BUILD)
    QT6_ADD_RESOURCES(parser_ut_RESOURCES_RCC resources.qrc)
elseif(QT5_BUILD)
    QT5_ADD_RESOURCES(parser_ut_RESOURCES_RCC resources.qrc)
else()
    message(FATAL_ERROR "Enable QT5_BUILD or QT6_BUILD")
endif()

add_executable(parser_ut parser_ut.cpp ${parser_ut_RESOURCES_RCC})

if(QT6_BUILD)
  target_link_libraries(parser_ut Qt6::Core Qt6::Test)
elseif(QT5_BUILD)
  target_link_libraries(parser_ut Qt5::Core Qt5::Test)
endif()

target_link_libraries(parser_ut ircclient irc)
target_include_directories(parser_ut PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_test(NAME parser_ut COMMAND parser_ut)

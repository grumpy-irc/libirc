PROJECT(irc)

if(QT5_BUILD)
  find_package(Qt5Core REQUIRED)
  find_package(Qt5Network REQUIRED)
  set(QT_INCLUDES ${Qt5Gui_INCLUDE_DIRS} ${Qt5Network_INCLUDE_DIRS})
  include_directories(${QT_INCLUDES})
else()
  find_package(Qt4 REQUIRED)
endif()

SET(CMAKE_AUTOMOC ON)

file (GLOB hd "*.h")
file (GLOB src "*.cpp")
file (GLOB resources "*.qrc")

if(QT5_BUILD)
    QT5_WRAP_CPP(irc_HEADERS_MOC ${irc_HEADERS})
    QT5_ADD_RESOURCES(irc_RESOURCES_RCC ${resources})
else()
    QT4_WRAP_CPP(irc_HEADERS_MOC ${irc_HEADERS})
    QT4_ADD_RESOURCES(irc_RESOURCES_RCC ${resources})
endif()

if (NOT QT5_BUILD)
  INCLUDE(${QT_USE_FILE})
endif()

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DLIBIRC_LIBRARY -DQT_USE_QSTRINGBUILDER)

ADD_LIBRARY(irc SHARED ${src} ${hd})

if (QT5_BUILD)
  qt5_use_modules(irc Core Network)
endif()

TARGET_LINK_LIBRARIES(irc ${QT_LIBRARIES})

PROJECT(ircclient)
SET(CMAKE_AUTOMOC ON)
SET(QT_USE_QTCORE TRUE)
SET(QT_USE_QTNETWORK TRUE)
include(GNUInstallDirs)

option(QT6_BUILD "Build with Qt6" false)

if(QT6_BUILD)
  find_package(Qt6Core REQUIRED)
  find_package(Qt6Network REQUIRED)
  set(QT_INCLUDES ${Qt6Gui_INCLUDE_DIRS} ${Qt6Widgets_INCLUDE_DIRS} ${Qt6Network_INCLUDE_DIRS} ${Qt6WebKit_INCLUDE_DIRS} ${Qt6Xml_INCLUDE_DIRS} ${Qt6Svg_INCLUDE_DIRS})
  include_directories(${QT_INCLUDES})
elseif(QT5_BUILD)
  find_package(Qt5Core REQUIRED)
  find_package(Qt5Network REQUIRED)
  set(QT_INCLUDES ${Qt5Gui_INCLUDE_DIRS} ${Qt5Widgets_INCLUDE_DIRS} ${Qt5Network_INCLUDE_DIRS} ${Qt5WebKit_INCLUDE_DIRS} ${Qt5Xml_INCLUDE_DIRS} ${Qt5Svg_INCLUDE_DIRS})
  include_directories(${QT_INCLUDES})
else()
  find_package(Qt4 REQUIRED)
endif()

file (GLOB src "*.cpp")
file (GLOB resources "*.qrc")
file (GLOB hx "*.h")

if(QT6_BUILD)
    QT6_WRAP_CPP(ircclient_HEADERS_MOC ${ircclient_HEADERS})
    QT6_ADD_RESOURCES(ircclient_RESOURCES_RCC ${resources})
elseif(QT5_BUILD)
    QT5_WRAP_CPP(ircclient_HEADERS_MOC ${ircclient_HEADERS})
    QT5_ADD_RESOURCES(ircclient_RESOURCES_RCC ${resources})
else()
    QT4_WRAP_CPP(ircclient_HEADERS_MOC ${ircclient_HEADERS})
    QT4_ADD_RESOURCES(ircclient_RESOURCES_RCC ${resources})
endif()

if (NOT QT5_BUILD)
  INCLUDE(${QT_USE_FILE})
endif()

ADD_DEFINITIONS(${QT_DEFINITIONS})
#ADD_DEFINITIONS(-DLIBIRCCLIENT_LIBRARY -DLIBIRC_HASH -DQT_USE_QSTRINGBUILDER)
ADD_DEFINITIONS(-DLIBIRCCLIENT_LIBRARY -DQT_USE_QSTRINGBUILDER)

ADD_LIBRARY(ircclient SHARED ${src} ${hx})

if (QT6_BUILD)
    TARGET_LINK_LIBRARIES(ircclient Qt6::Core Qt6::Network)
elseif (QT5_BUILD)
    TARGET_LINK_LIBRARIES(ircclient Qt5::Core Qt5::Network)
endif()

TARGET_LINK_LIBRARIES(ircclient irc ${QT_LIBRARIES})
if (NOT WIN32)
    INSTALL(TARGETS ircclient LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

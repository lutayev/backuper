cmake_minimum_required(VERSION 3.5)

set(PROJECT Backuper)
project(${PROJECT} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)


#Sources/headers/ui/resources list
set(HEADERS
    src/core.h
    src/syncentity.h
    src/configcontroller.h

    )

set(SOURCES
    src/main.cpp
    src/core.cpp
    src/syncentity.cpp
    src/configcontroller.cpp

    )

set(UI
    src/core.ui

    )

set(RESOURCES src/resources.qrc)

#Qt5 stuff
find_package(Qt5Widgets)
find_package(Qt5Core)
find_package(Qt5Gui)
find_package(Qt5Xml)
find_package(Qt5Sql)
find_package(Qt5Network)
qt5_add_resources(QRC_SOURCES ${RESOURCES})
QT5_WRAP_UI(UIS ${UI})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

#Additional system-specific library dependencies
if (APPLE)
    list(APPEND EXTRA_LIBS
        ""
        )
elseif(WIN32)
    list(APPEND EXTRA_LIBS
        ""
        )
    set(CMAKE_EXE_LINKER_FLAGS "-std=gnu99 -static -static-libgcc -static-libstdc++ -mwindows")
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
elseif(UNIX)
    list(APPEND EXTRA_LIBS
        ""
        )
endif()

#Make, link and install
add_executable(${PROJECT} ${UIS} ${HEADERS} ${SOURCES} ${MOC_SOURCES} ${QRC_SOURCES} )
target_link_libraries(${PROJECT} Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Xml Qt5::Sql ${EXTRA_LIBS})

install(TARGETS ${PROJECT} DESTINATION ${CMAKE_INSTALL_PREFIX})

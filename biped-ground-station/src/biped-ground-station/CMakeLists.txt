cmake_minimum_required(VERSION 3.5)

project(biped-ground-station VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS OpenGLWidgets PrintSupport Widgets)

set(PROJECT_SOURCES
        daemon/camera_daemon.cpp
        daemon/camera_daemon.h
        ui/camera_view.cpp
        ui/camera_view.h
        ui/confirm_dialog.cpp
        ui/confirm_dialog.h
        ui/confirm_dialog.ui
        common/global.cpp
        common/global.h
        daemon/inbound_daemon.cpp
        daemon/inbound_daemon.h
        ui/joypad.cpp
        ui/joypad.h
        daemon/joypad_daemon.cpp
        daemon/joypad_daemon.h
        daemon/logging_daemon.cpp
        daemon/logging_daemon.h
        main/main.cpp
        daemon/outbound_daemon.cpp
        daemon/outbound_daemon.h
        common/parameter.h
        ui/parameter_dialog.cpp
        ui/parameter_dialog.h
        ui/parameter_dialog.ui
        ui/parameter_set.cpp
        ui/parameter_set.h
        ui/parameter_set.ui
        ui/qcustomplot.cpp
        ui/qcustomplot.h
        ui/rolling_plot.cpp
        ui/rolling_plot.h
        common/type.h
        network/udp.cpp
        network/udp.h
        utility/utility.cpp
        utility/utility.h
        ui/window.cpp
        ui/window.h
        ui/window.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(biped-ground-station
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET biped-ground-station APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(biped-ground-station SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(biped-ground-station
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(biped-ground-station PRIVATE
    ${Boost_LIBRARIES}
    Qt${QT_VERSION_MAJOR}::OpenGLWidgets
    Qt${QT_VERSION_MAJOR}::PrintSupport
    Qt${QT_VERSION_MAJOR}::Widgets
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.biped-ground-station)
endif()

set_target_properties(biped-ground-station PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)

include_directories(
    "${CMAKE_SOURCE_DIR}"
)

install(TARGETS biped-ground-station
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(biped-ground-station)
endif()

cmake_minimum_required(VERSION 3.16)
project(ClassTopLand)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra)
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()
if(NOT CMAKE_PREFIX_PATH)
    if (UNIX AND NOT APPLE)
        set(CMAKE_PREFIX_PATH "/home/jiahang/Qt/6.7.3/gcc_64/lib/cmake/")
    elseif(WIN32)
        set(CMAKE_PREFIX_PATH "C:/Qt/6.9.3/msvc2022_64/lib/cmake")
    elseif(APPLE)
        set(CMAKE_PREFIX_PATH "/Users/jiahang/Qt/6.7.3/macos/lib/cmake/")
    endif()
endif()

if (UNIX AND NOT APPLE)
    set(CMAKE_LINK_LIBRARY_FLAG "-static")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++ -static-libgcc")
endif()
find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        Network
        REQUIRED
)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS "src/*.cpp" "src/*.h" "src/*.ui")
set(RESOURCE_FILES res.qrc resource.rc)

include_directories(${Qt6Gui_PRIVATE_INCLUDE_DIRS})
set(MAIN_SOURCE main.cpp)

qt_add_executable(
        ${PROJECT_NAME}
        ${MAIN_SOURCE}
        ${SOURCE_FILES}
        ${RESOURCE_FILES}
)
set_target_properties(ClassTopLand PROPERTIES
        WIN32_EXECUTABLE TRUE
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist
        MACOSX_BUNDLE_GUI_IDENTIFIER com.aero8m.classtopland
        MACOSX_BUNDLE_BUNDLE_NAME "ClassTopLand"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "NightlyBuild"
        MACOSX_BUNDLE_LONG_VERSION_STRING "NightlyBuild"
        MACOSX_BUNDLE_COPYRIGHT "OpenSource By GPL"
        MACOSX_BUNDLE_ICON_FILE app.icns
)
target_sources(ClassTopLand PRIVATE
        app.icns
)

set_source_files_properties(app.icns PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources"
)

if (UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    target_include_directories(ClassTopLand PRIVATE ${X11_INCLUDE_DIR})
    target_link_libraries(ClassTopLand PRIVATE ${X11_LIBRARIES})
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::Network
)


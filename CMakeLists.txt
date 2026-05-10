cmake_minimum_required(VERSION 3.10)
project(ClassTopLand)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_AUTOMOC ON)         # 自动调用 moc
set(CMAKE_AUTORCC ON)         # 自动调用 rcc
set(CMAKE_AUTOUIC ON)         # 自动调用 uic
set(CMAKE_INCLUDE_CURRENT_DIR ON)
#set(NIGHTLY_RELEASE ON)
# 设置 Qt 的查找路径（可选）
if (UNIX AND NOT APPLE)
    set(CMAKE_PREFIX_PATH "/home/jiahang/Qt/6.7.3/gcc_64/lib/cmake/")
elseif(WIN32 AND NOT NIGHTLY_RELEASE)
    set(CMAKE_PREFIX_PATH "C:/Qt/6.9.3/msvc2022_64/lib/cmake")
elseif(APPLE)
    set(CMAKE_PREFIX_PATH "/Users/jiahang/Qt/6.7.3/macos/lib/cmake/")
elseif(NIGHTLY_RELEASE)
    set(CMAKE_PREFIX_PATH "C:/Qt/6.7.3/msvc2022_64/lib/cmake")
endif()

if (UNIX AND NOT APPLE)
    set(CMAKE_LINK_LIBRARY_FLAG "-static")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++ -static-libgcc")
endif()
# 查找 Qt6 组件
find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        Network
        REQUIRED
)

# 收集 src 及其子目录中的源文件
file(GLOB_RECURSE SOURCE_FILES "src/*.cpp" "src/*.h" "src/*.ui")
set(RESOURCE_FILES res.qrc resource.rc)

include_directories(${Qt6Gui_PRIVATE_INCLUDE_DIRS})
# 添加主函数入口
set(MAIN_SOURCE main.cpp
        src/Utils/Utils.cpp
        src/Utils/Utils.h
        src/FluentTabWidget/FluentTabWidget.cpp
        src/FluentTabWidget/FluentTabWidget.h
        src/VERSION.h
)

qt_add_executable(
        ${PROJECT_NAME}
        ${MAIN_SOURCE}
        ${SOURCE_FILES}
        ${RESOURCE_FILES}
)
# 创建可执行文件（会自动处理 .ui/.qrc）

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
# 链接需要的 Qt 库
target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::Network # 添加这一行
        # Qt::WebEnginePrivate  # 如果仍然报错，可以尝试添加
        #Qt::GuiPrivate
        Qt::Test
)


# 如果你需要显式包含某些目录（比如头文件放在 include/ 中），可以加上：
# include_directories(include)

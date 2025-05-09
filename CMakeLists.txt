cmake_minimum_required(VERSION 3.5)

project(MainToDoApp VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Поиск Qt с нужными модулями
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Network Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network Widgets)

# Список исходных файлов
set(HEADERS
    src/api.h
    src/mainwindow.h
    src/models.h
    src/authorization.h
    src/TaskDialog.h
)

set(SOURCES
    src/api.cpp
    src/mainwindow.cpp
    src/models.cpp
    src/authorization.cpp
    src/TaskDialog.cpp
    main.cpp
)

# Создание исполняемого файла
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(MainToDoApp
        MANUAL_FINALIZATION
        ${HEADERS}
        ${SOURCES}
    )
else()
    if(ANDROID)
        add_library(MainToDoApp SHARED
            ${HEADERS}
            ${SOURCES}
        )
        set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(MainToDoApp
            ${HEADERS}
            ${SOURCES}
        )
    endif()
endif()

# Подключение Qt модулей
target_link_libraries(MainToDoApp PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Widgets
)

# Настройки для macOS
if(${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.MainToDoApp)
endif()

set_target_properties(MainToDoApp PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# Установка
include(GNUInstallDirs)
install(TARGETS MainToDoApp
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Финальная настройка для Qt6
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(MainToDoApp)
endif()

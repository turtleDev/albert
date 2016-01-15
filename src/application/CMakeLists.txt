find_package(Qt5Widgets 5.1 REQUIRED)
find_package(Qt5Quick 5.1 REQUIRED)

include_directories(
    hotkey/
    interfaces/
    pluginsystem/
    pluginsystem/extension/
    settingswidget/
)

set(SRC
    main.cpp
    albertapp.cpp
    mainwindow.cpp
    history.hpp
    hotkey/hotkeymanager.cpp
    hotkey/hotkeymanager_p.h
    pluginsystem/pluginloader.cpp
    pluginsystem/pluginmanager.cpp
    pluginsystem/extension/extensionmanager.cpp
    pluginsystem/extension/query.cpp
    pluginsystem/extension/query_p.hpp
    pluginsystem/extension/iextension.h
    pluginsystem/extension/abstractobjects.hpp
    pluginsystem/extension/objects.hpp
    settingswidget/grabkeybutton.cpp
    settingswidget/settingswidget.cpp
    settingswidget/pluginmodel.cpp
    settingswidget/propertyeditor.cpp
    settingswidget/colordialog.hpp
)

set(LIB
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Quick_LIBRARIES}
)

qt5_wrap_ui(UI
    settingswidget/settingswidget.ui
)

qt5_add_resources(QRC
    resources.qrc
)


# Do platform specific pre target stuff
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    # Make sure x11 module is present
    find_package(Qt5X11Extras 5.1 REQUIRED)
    # Platform switches
    set(SRC ${SRC} hotkey/hotkeymanager_x11.cpp)
    set(LIB ${LIB} ${Qt5X11Extras_LIBRARIES} X11)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    #TODO INSTALL
    set(SRCS ${SRCS} src/hotkey/hotkeymanager_win.cpp)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUNICODE")
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")


# Define target
add_executable(${PROJECT_NAME} ${SRC} ${UI} ${QRC})
target_link_libraries(${PROJECT_NAME} ${LIB})


# Do platform specific post target stuff
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    # Define where to install the binary
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
    # Install desktop entry
    install(FILES resources/albert.desktop DESTINATION share/applications )
    # Install icon
    install(FILES resources/icons/albert.svg DESTINATION share/icons/hicolor/scalable/apps)
    # Define where to install the themes
    install(DIRECTORY resources/themes DESTINATION share/${PROJECT_NAME} COMPONENT config)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

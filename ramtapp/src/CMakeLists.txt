project("appmgr")

set (TARGET "appmgr")
add_executable(${TARGET} main.cpp 
    Application.cpp
    Menu.cpp
    PerfTestMgr.cpp
    ThunderBridge.cpp
    ManagerRegistry.cpp
    managers/AppMgrControl.cpp 
    managers/PkgMgrControl.cpp  
    managers/DownloadMgrCtrl.cpp
    managers/InstallMgrCtrl.cpp
    managers/CatalogManager.cpp
    managers/RDKWindowMgrCtrl.cpp
    )

find_package(PkgConfig REQUIRED)

pkg_check_modules(WPEFRAMEWORKCORE REQUIRED WPEFrameworkCore)
pkg_check_modules(WPEFRAMEWORKPLUGINS REQUIRED WPEFrameworkPlugins)
pkg_check_modules(WPEFRAMEWORKWEBSOCKET REQUIRED WPEFrameworkWebSocket)
pkg_check_modules(WPEFRAMEWORKDEFINITIONS REQUIRED WPEFrameworkDefinitions)


set(COMMON_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${WPEFRAMEWORKCORE_INCLUDE_DIRS}
    ${WPEFRAMEWORKPLUGINS_INCLUDE_DIRS}
    ${WPEFRAMEWORKWEBSOCKET_INCLUDE_DIRS}
    ${WPEFRAMEWORKDEFINITIONS_INCLUDE_DIRS}
    ../include
    )

set(COMMON_LIBRARIES
    ${WPEFRAMEWORKCORE_LIBRARIES}
    ${WPEFRAMEWORKPLUGINS_LIBRARIES}
    ${WPEFRAMEWORKWEBSOCKET_LIBRARIES}
    ${WPEFRAMEWORKDEFINITIONS_LIBRARIES}
    jsoncpp
    curl
    )

include(GNUInstallDirs)
target_include_directories(${TARGET} PRIVATE ${COMMON_INCLUDE_DIRS})
target_link_libraries(${TARGET} PRIVATE ${COMMON_LIBRARIES})

install(TARGETS ${TARGET} DESTINATION ${CMAKE_INSTALL_BINDIR})
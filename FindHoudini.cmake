# Find Houdini
if (MSVC)
    set(Houdini_PATH "C:/Program Files/Side Effects Software/Houdini 20.5.332")
elseif (APPLE)
    set(Houdini_PATH "/Applications/Houdini/Houdini20.5.278/Frameworks/Houdini.framework/Versions/20.5/Resources")
endif ()
set(Houdini_DIR ${Houdini_PATH}/toolkit/cmake)
find_package(Houdini REQUIRED)
houdini_get_default_install_dir(HOUDINI_INSTALL_DIR)

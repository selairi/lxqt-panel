set(PLUGIN "backlight")

set(HEADERS
    backlight.h
    sliderdialog.h
)

set(SOURCES
    backlight.cpp
    sliderdialog.cpp
)

set(LIBRARIES
    ${LIBRARIES}
    lxqt-globalkeys
)

BUILD_LXQT_PLUGIN(${PLUGIN})

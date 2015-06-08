include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../common
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/libtelephonyservice
    ${CMAKE_SOURCE_DIR}/indicator
    ${MESSAGING_MENU_INCLUDE_DIRS}
    ${TP_QT5_INCLUDE_DIRS}
    ${HISTORY_INCLUDE_DIRS}
    ${GSETTINGS_QT_INCLUDE_DIRS}
    )

generate_telepathy_test(MessagingMenuTest
              SOURCES MessagingMenuTest.cpp messagingmenumock.cpp ${CMAKE_SOURCE_DIR}/indicator/messagingmenu.cpp
              LIBRARIES  ${TP_QT5_LIBRARIES} telephonyservice mockcontroller telepathytest ${HISTORY_LIBRARIES})
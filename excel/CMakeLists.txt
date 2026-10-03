qt_add_executable(EasySheets WIN32
    src/main.cpp
    src/MainWindow.h src/MainWindow.cpp
    src/ChartDialog.h src/ChartDialog.cpp
)
target_link_libraries(EasySheets PRIVATE EasyCore Qt6::PrintSupport)
install(TARGETS EasySheets RUNTIME DESTINATION bin)

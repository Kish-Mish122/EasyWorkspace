qt_add_executable(EasyWrite WIN32
    src/main.cpp
    src/MainWindow.h src/MainWindow.cpp
)
target_link_libraries(EasyWrite PRIVATE EasyCore Qt6::PrintSupport)
install(TARGETS EasyWrite RUNTIME DESTINATION bin)

qt_add_executable(EasySlides WIN32
    src/main.cpp
    src/Slide.h src/Slide.cpp
    src/SlideItem.h src/SlideItem.cpp
    src/SlideScene.h src/SlideScene.cpp
    src/SlideShowWindow.h src/SlideShowWindow.cpp
    src/MainWindow.h src/MainWindow.cpp
)
target_link_libraries(EasySlides PRIVATE EasyCore)
install(TARGETS EasySlides RUNTIME DESTINATION bin)

add_library(EasyCore STATIC
    src/Common.h src/Common.cpp
    src/ThemeManager.h src/ThemeManager.cpp
    src/CustomTitleBar.h src/CustomTitleBar.cpp
    src/RibbonTabBar.h src/RibbonTabBar.cpp
    src/RibbonPanel.h src/RibbonPanel.cpp
    src/SmoothScrollArea.h src/SmoothScrollArea.cpp
    src/PageSettingsDialog.h src/PageSettingsDialog.cpp
    src/FormulaEngine.h src/FormulaEngine.cpp
    src/SpreadsheetModel.h src/SpreadsheetModel.cpp
)
target_include_directories(EasyCore PUBLIC src)
target_link_libraries(EasyCore PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets)

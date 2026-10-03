#include "ThemeManager.h"

#include <QApplication>
#include <QPalette>

namespace easy {

ThemeManager& ThemeManager::instance()
{
    static ThemeManager inst;
    return inst;
}

QPalette ThemeManager::buildLightPalette(const QColor& accent)
{
    QPalette p;
    p.setColor(QPalette::Window, QColor(0xf8, 0xf9, 0xfa));
    p.setColor(QPalette::WindowText, QColor(0x20, 0x20, 0x20));
    p.setColor(QPalette::Base, Qt::white);
    p.setColor(QPalette::AlternateBase, QColor(0xf3, 0xf3, 0xf4));
    p.setColor(QPalette::Text, QColor(0x20, 0x20, 0x20));
    p.setColor(QPalette::Button, QColor(0xffffff));
    p.setColor(QPalette::ButtonText, QColor(0x20, 0x20, 0x20));
    p.setColor(QPalette::BrightText, Qt::white);
    p.setColor(QPalette::Highlight, accent);
    p.setColor(QPalette::HighlightedText, Qt::white);
    p.setColor(QPalette::Light, Qt::white);
    p.setColor(QPalette::Midlight, QColor(0xe5, 0xe5, 0xe5));
    p.setColor(QPalette::Dark, QColor(0x9f, 0x9f, 0x9f));
    p.setColor(QPalette::Shadow, QColor(0x00, 0x00, 0x00));
    p.setColor(QPalette::PlaceholderText, QColor(0x9a, 0x9a, 0x9a));
    p.setColor(QPalette::Link, accent);
    p.setColor(QPalette::LinkVisited, accent.darker(120));
    return p;
}

QPalette ThemeManager::buildDarkPalette(const QColor& accent)
{
    QPalette p;
    p.setColor(QPalette::Window, QColor(0x1e, 0x1e, 0x1e));
    p.setColor(QPalette::WindowText, QColor(0xf0, 0xf0, 0xf0));
    p.setColor(QPalette::Base, QColor(0x2d, 0x2d, 0x30));
    p.setColor(QPalette::AlternateBase, QColor(0x25, 0x25, 0x26));
    p.setColor(QPalette::Text, QColor(0xf0, 0xf0, 0xf0));
    p.setColor(QPalette::Button, QColor(0x2d, 0x2d, 0x30));
    p.setColor(QPalette::ButtonText, QColor(0xf0, 0xf0, 0xf0));
    p.setColor(QPalette::BrightText, Qt::white);
    p.setColor(QPalette::Highlight, accent);
    p.setColor(QPalette::HighlightedText, Qt::white);
    p.setColor(QPalette::Light, QColor(0x3d, 0x3d, 0x40));
    p.setColor(QPalette::Midlight, QColor(0x33, 0x33, 0x35));
    p.setColor(QPalette::Dark, QColor(0x1e, 0x1e, 0x1e));
    p.setColor(QPalette::Shadow, QColor(0x00, 0x00, 0x00));
    p.setColor(QPalette::PlaceholderText, QColor(0x80, 0x80, 0x80));
    p.setColor(QPalette::Link, accent);
    p.setColor(QPalette::LinkVisited, accent.lighter(120));
    return p;
}

static QString makeLightQss(const QColor& accent)
{
    QString a = accent.name(QColor::HexRgb);
    QString ad = accent.darker(120).name(QColor::HexRgb);
    QString s;
    s += "QMainWindow{background:#f8f9fa;}";
    s += "QMenuBar{background:" + a + ";color:#ffffff;border-bottom:1px solid #e5e5e5;padding:0px;font-size:12px;}";
    s += "QMenuBar::item{padding:6px 12px;background:transparent;border-radius:4px;margin:2px 2px;}";
    s += "QMenuBar::item:selected{background:#f0f0f0;}";
    s += "QMenuBar::item:pressed{background:#e5e5e5;}";
    s += "QMenu{background:#ffffff;border:1px solid #e5e5e5;border-radius:8px;padding:4px;font-size:12px;}";
    s += "QMenu::item{padding:6px 32px 6px 16px;border-radius:4px;background:transparent;}";
    s += "QMenu::item:selected{background:#f5f5f5;}";
    s += "QMenu::item:pressed{background:#eaeaea;}";
    s += "QMenu::separator{height:1px;background:#e5e5e5;margin:4px 12px;}";
    s += "QMenu::indicator{width:16px;height:16px;}";
    s += "QToolBar{spacing:2px;padding:4px 6px;border:0;border-bottom:1px solid #e5e5e5;background:rgba(255,255,255,200);}";
    s += "QToolBar::separator{width:1px;background:#e5e5e5;margin:4px 4px;}";
    s += "QToolBar::handle{background:transparent;width:14px;}";
    s += "QToolButton{padding:4px 8px;border:0;border-radius:4px;background:transparent;color:#202020;font-size:12px;}";
    s += "QToolButton:hover{background:#f5f5f5;}";
    s += "QToolButton:pressed{background:#ebebeb;}";
    s += "QToolButton:checked{background:" + a + ";color:white;}";
    s += "QToolButton:checked:hover{background:" + a + ";}";
    s += "QToolButton:checked:pressed{background:" + ad + ";}";
    s += "QStatusBar{background:rgba(255,255,255,220);border-top:1px solid #e5e5e5;color:#606060;font-size:11px;}";
    s += "QStatusBar QLabel{border:0;padding:2px 8px;}";
    s += "QLineEdit{padding:5px 10px;border:1px solid #e5e5e5;border-radius:6px;background:#ffffff;color:#202020;selection-background-color:" + a + ";font-size:12px;}";
    s += "QLineEdit:hover{border-color:#c0c0c0;}";
    s += "QLineEdit:focus{border:2px solid " + a + ";padding:4px 9px;}";
    s += "QComboBox{padding:4px 10px;border:1px solid #e5e5e5;border-radius:6px;background:#ffffff;color:#202020;font-size:12px;}";
    s += "QComboBox:hover{border-color:#c0c0c0;}";
    s += "QComboBox:focus{border:2px solid " + a + ";}";
    s += "QComboBox::drop-down{border:0;width:20px;}";
    s += "QComboBox::down-arrow{image:none;border-left:4px solid transparent;border-right:4px solid transparent;border-top:5px solid #606060;margin-right:4px;}";
    s += "QComboBox QAbstractItemView{border:1px solid #e5e5e5;border-radius:6px;background:#ffffff;selection-background-color:" + a + ";selection-color:#ffffff;padding:4px;}";
    s += "QComboBox QAbstractItemView::item{padding:6px 12px;border-radius:4px;}";
    s += "QComboBox QAbstractItemView::item:selected{background:" + a + ";color:#ffffff;}";
    s += "QSpinBox{padding:4px 10px;border:1px solid #e5e5e5;border-radius:6px;background:#ffffff;color:#202020;font-size:12px;}";
    s += "QSpinBox:hover{border-color:#c0c0c0;}";
    s += "QSpinBox:focus{border:2px solid " + a + ";}";
    s += "QSpinBox::up-button,QSpinBox::down-button{border:0;width:18px;background:transparent;}";
    s += "QSpinBox::up-button:hover,QSpinBox::down-button:hover{background:#f0f0f0;}";
    s += "QSpinBox::up-arrow{border-left:4px solid transparent;border-right:4px solid transparent;border-bottom:5px solid #606060;}";
    s += "QSpinBox::down-arrow{border-left:4px solid transparent;border-right:4px solid transparent;border-top:5px solid #606060;}";
    s += "QTableView{border:1px solid #e5e5e5;border-radius:8px;background:#ffffff;gridline-color:#e8e8e8;selection-background-color:" + a + ";selection-color:#ffffff;font-size:12px;}";
    s += "QTableView::item{padding:3px 6px;border:0;}";
    s += "QTableView::item:hover{background:#f8f8f8;}";
    s += "QTableView::item:selected{background:" + a + ";color:#ffffff;}";
    s += "QHeaderView::section{background:#fafafa;border:0;border-bottom:1px solid #e5e5e5;border-right:1px solid #e8e8e8;padding:4px 8px;color:#404040;font-weight:bold;font-size:11px;}";
    s += "QHeaderView::section:first{border-top-left-radius:8px;}";
    s += "QHeaderView::section:last{border-top-right-radius:8px;border-right:0;}";
    s += "QHeaderView::section:hover{background:#f0f0f0;}";
    s += "QScrollBar:vertical{background:#f5f5f5;width:14px;border:0;border-radius:7px;margin:0;}";
    s += "QScrollBar::handle:vertical{background:#c8c8c8;border-radius:6px;min-height:24px;}";
    s += "QScrollBar::handle:vertical:hover{background:#a8a8a8;}";
    s += "QScrollBar::add-line:vertical,QScrollBar::sub-line:vertical{height:0;}";
    s += "QScrollBar::add-page:vertical,QScrollBar::sub-page:vertical{background:none;}";
    s += "QScrollBar:horizontal{background:#f5f5f5;height:14px;border:0;border-radius:7px;margin:0;}";
    s += "QScrollBar::handle:horizontal{background:#c8c8c8;border-radius:6px;min-width:24px;}";
    s += "QScrollBar::handle:horizontal:hover{background:#a8a8a8;}";
    s += "QScrollBar::add-line:horizontal,QScrollBar::sub-line:horizontal{width:0;}";
    s += "QScrollBar::add-page:horizontal,QScrollBar::sub-page:horizontal{background:none;}";
    s += "QTextEdit{border:0;border-radius:8px;background:#ffffff;padding:4px;selection-background-color:" + a + ";font-size:12px;}";
    s += "QScrollArea{border:0;border-radius:8px;}";
    s += "QSplitter::handle{background:#e5e5e5;}";
    s += "QSplitter::handle:horizontal{width:1px;}";
    s += "QSplitter::handle:vertical{height:1px;}";
    s += "QSplitter::handle:hover{background:" + a + ";}";
    s += "QListWidget{border:1px solid #e5e5e5;border-radius:8px;background:#ffffff;selection-background-color:" + a + ";selection-color:#ffffff;font-size:12px;}";
    s += "QListWidget::item{padding:4px 8px;border-radius:4px;margin:2px 4px;}";
    s += "QListWidget::item:hover{background:#f5f5f5;}";
    s += "QListWidget::item:selected{background:" + a + ";color:#ffffff;}";
    s += "QPushButton{padding:6px 16px;border:1px solid #e5e5e5;border-radius:6px;background:#ffffff;color:#202020;font-size:12px;}";
    s += "QPushButton:hover{background:#f5f5f5;border-color:#d0d0d0;}";
    s += "QPushButton:pressed{background:#ebebeb;border-color:#c0c0c0;}";
    s += "QPushButton:focus{border:2px solid " + a + ";padding:5px 15px;}";
    s += "QLabel{color:#202020;font-size:12px;}";
    s += "QGroupBox{border:1px solid #e5e5e5;border-radius:8px;margin-top:12px;padding-top:24px;font-weight:bold;}";
    s += "QGroupBox::title{subcontrol-origin:margin;subcontrol-position:top left;left:12px;top:0px;padding:0 4px;}";
    s += "QCheckBox{color:#202020;font-size:12px;spacing:8px;}";
    s += "QCheckBox::indicator{width:16px;height:16px;border:1px solid #d0d0d0;border-radius:4px;background:#ffffff;}";
    s += "QCheckBox::indicator:hover{border-color:" + a + ";}";
    s += "QCheckBox::indicator:checked{background:" + a + ";border-color:" + a + ";}";
    s += "QRadioButton{color:#202020;font-size:12px;spacing:6px;}";
    s += "QRadioButton::indicator{width:16px;height:16px;}";
    s += "QRadioButton::indicator::unchecked{border:1px solid #d0d0d0;border-radius:8px;background:#ffffff;}";
    s += "QRadioButton::indicator::checked{border:3px solid " + a + ";border-radius:8px;background:#ffffff;}";
    s += "QProgressBar{border:1px solid #e5e5e5;border-radius:6px;text-align:center;background:#f5f5f5;height:8px;}";
    s += "QProgressBar::chunk{background:" + a + ";border-radius:5px;}";
    s += "QDialog{background:#f8f9fa;}";
    s += "QFrame{border:0;}";
    return s;
}

static QString makeDarkQss(const QColor& accent)
{
    QString a = accent.name(QColor::HexRgb);
    QString ad = accent.darker(120).name(QColor::HexRgb);
    QString s;
    s += "QMainWindow{background:#1e1e1e;}";
    s += "QMenuBar{background:" + a + ";color:#ffffff;border-bottom:1px solid #3e3e42;padding:0px;font-size:12px;}";
    s += "QMenuBar::item{padding:6px 12px;background:transparent;border-radius:4px;margin:2px 2px;}";
    s += "QMenuBar::item:selected{background:#3e3e42;}";
    s += "QMenuBar::item:pressed{background:#454548;}";
    s += "QMenu{background:#2d2d30;border:1px solid #3e3e42;border-radius:8px;padding:4px;font-size:12px;}";
    s += "QMenu::item{padding:6px 32px 6px 16px;border-radius:4px;background:transparent;color:#f0f0f0;}";
    s += "QMenu::item:selected{background:#3e3e42;}";
    s += "QMenu::item:pressed{background:#454548;}";
    s += "QMenu::separator{height:1px;background:#3e3e42;margin:4px 12px;}";
    s += "QMenu::indicator{width:16px;height:16px;}";
    s += "QToolBar{spacing:2px;padding:4px 6px;border:0;border-bottom:1px solid #3e3e42;background:rgba(45,45,48,200);}";
    s += "QToolBar::separator{width:1px;background:#3e3e42;margin:4px 4px;}";
    s += "QToolBar::handle{background:transparent;width:14px;}";
    s += "QToolButton{padding:4px 8px;border:0;border-radius:4px;background:transparent;color:#f0f0f0;font-size:12px;}";
    s += "QToolButton:hover{background:#3e3e42;}";
    s += "QToolButton:pressed{background:#454548;}";
    s += "QToolButton:checked{background:" + a + ";color:white;}";
    s += "QToolButton:checked:hover{background:" + a + ";}";
    s += "QToolButton:checked:pressed{background:" + ad + ";}";
    s += "QStatusBar{background:rgba(45,45,48,220);border-top:1px solid #3e3e42;color:#a0a0a0;font-size:11px;}";
    s += "QStatusBar QLabel{border:0;padding:2px 8px;}";
    s += "QLineEdit{padding:5px 10px;border:1px solid #3e3e42;border-radius:6px;background:#2d2d30;color:#f0f0f0;selection-background-color:" + a + ";font-size:12px;}";
    s += "QLineEdit:hover{border-color:#606060;}";
    s += "QLineEdit:focus{border:2px solid " + a + ";padding:4px 9px;}";
    s += "QComboBox{padding:4px 10px;border:1px solid #3e3e42;border-radius:6px;background:#2d2d30;color:#f0f0f0;font-size:12px;}";
    s += "QComboBox:hover{border-color:#606060;}";
    s += "QComboBox:focus{border:2px solid " + a + ";}";
    s += "QComboBox::drop-down{border:0;width:20px;}";
    s += "QComboBox::down-arrow{image:none;border-left:4px solid transparent;border-right:4px solid transparent;border-top:5px solid #a0a0a0;margin-right:4px;}";
    s += "QComboBox QAbstractItemView{border:1px solid #3e3e42;border-radius:6px;background:#2d2d30;selection-background-color:" + a + ";selection-color:#ffffff;padding:4px;color:#f0f0f0;}";
    s += "QComboBox QAbstractItemView::item{padding:6px 12px;border-radius:4px;}";
    s += "QComboBox QAbstractItemView::item:selected{background:" + a + ";color:#ffffff;}";
    s += "QSpinBox{padding:4px 10px;border:1px solid #3e3e42;border-radius:6px;background:#2d2d30;color:#f0f0f0;font-size:12px;}";
    s += "QSpinBox:hover{border-color:#606060;}";
    s += "QSpinBox:focus{border:2px solid " + a + ";}";
    s += "QSpinBox::up-button,QSpinBox::down-button{border:0;width:18px;background:transparent;}";
    s += "QSpinBox::up-button:hover,QSpinBox::down-button:hover{background:#3e3e42;}";
    s += "QSpinBox::up-arrow{border-left:4px solid transparent;border-right:4px solid transparent;border-bottom:5px solid #a0a0a0;}";
    s += "QSpinBox::down-arrow{border-left:4px solid transparent;border-right:4px solid transparent;border-top:5px solid #a0a0a0;}";
    s += "QTableView{border:1px solid #3e3e42;border-radius:8px;background:#2d2d30;gridline-color:#3e3e42;selection-background-color:" + a + ";selection-color:#ffffff;font-size:12px;}";
    s += "QTableView::item{padding:3px 6px;border:0;}";
    s += "QTableView::item:hover{background:#333336;}";
    s += "QTableView::item:selected{background:" + a + ";color:#ffffff;}";
    s += "QHeaderView::section{background:#252526;border:0;border-bottom:1px solid #3e3e42;border-right:1px solid #3e3e42;padding:4px 8px;color:#e0e0e0;font-weight:bold;font-size:11px;}";
    s += "QHeaderView::section:first{border-top-left-radius:8px;}";
    s += "QHeaderView::section:last{border-top-right-radius:8px;border-right:0;}";
    s += "QHeaderView::section:hover{background:#333336;}";
    s += "QScrollBar:vertical{background:#252526;width:14px;border:0;border-radius:7px;margin:0;}";
    s += "QScrollBar::handle:vertical{background:#6e6e72;border-radius:6px;min-height:24px;}";
    s += "QScrollBar::handle:vertical:hover{background:#8e8e92;}";
    s += "QScrollBar::add-line:vertical,QScrollBar::sub-line:vertical{height:0;}";
    s += "QScrollBar::add-page:vertical,QScrollBar::sub-page:vertical{background:none;}";
    s += "QScrollBar:horizontal{background:#252526;height:14px;border:0;border-radius:7px;margin:0;}";
    s += "QScrollBar::handle:horizontal{background:#6e6e72;border-radius:6px;min-width:24px;}";
    s += "QScrollBar::handle:horizontal:hover{background:#8e8e92;}";
    s += "QScrollBar::add-line:horizontal,QScrollBar::sub-line:horizontal{width:0;}";
    s += "QScrollBar::add-page:horizontal,QScrollBar::sub-page:horizontal{background:none;}";
    s += "QTextEdit{border:0;border-radius:8px;background:#2d2d30;padding:4px;selection-background-color:" + a + ";font-size:12px;color:#f0f0f0;}";
    s += "QScrollArea{border:0;border-radius:8px;}";
    s += "QSplitter::handle{background:#3e3e42;}";
    s += "QSplitter::handle:horizontal{width:1px;}";
    s += "QSplitter::handle:vertical{height:1px;}";
    s += "QSplitter::handle:hover{background:" + a + ";}";
    s += "QListWidget{border:1px solid #3e3e42;border-radius:8px;background:#2d2d30;selection-background-color:" + a + ";selection-color:#ffffff;font-size:12px;color:#f0f0f0;}";
    s += "QListWidget::item{padding:4px 8px;border-radius:4px;margin:2px 4px;}";
    s += "QListWidget::item:hover{background:#3e3e42;}";
    s += "QListWidget::item:selected{background:" + a + ";color:#ffffff;}";
    s += "QPushButton{padding:6px 16px;border:1px solid #3e3e42;border-radius:6px;background:#2d2d30;color:#f0f0f0;font-size:12px;}";
    s += "QPushButton:hover{background:#3e3e42;border-color:#505053;}";
    s += "QPushButton:pressed{background:#454548;border-color:#606060;}";
    s += "QPushButton:focus{border:2px solid " + a + ";padding:5px 15px;}";
    s += "QLabel{color:#f0f0f0;font-size:12px;}";
    s += "QGroupBox{border:1px solid #3e3e42;border-radius:8px;margin-top:12px;padding-top:24px;font-weight:bold;color:#f0f0f0;}";
    s += "QGroupBox::title{subcontrol-origin:margin;subcontrol-position:top left;left:12px;top:0px;padding:0 4px;}";
    s += "QCheckBox{color:#f0f0f0;font-size:12px;spacing:8px;}";
    s += "QCheckBox::indicator{width:16px;height:16px;border:1px solid #505053;border-radius:4px;background:#2d2d30;}";
    s += "QCheckBox::indicator:hover{border-color:" + a + ";}";
    s += "QCheckBox::indicator:checked{background:" + a + ";border-color:" + a + ";}";
    s += "QRadioButton{color:#f0f0f0;font-size:12px;spacing:6px;}";
    s += "QRadioButton::indicator{width:16px;height:16px;}";
    s += "QRadioButton::indicator::unchecked{border:1px solid #505053;border-radius:8px;background:#2d2d30;}";
    s += "QRadioButton::indicator::checked{border:3px solid " + a + ";border-radius:8px;background:#2d2d30;}";
    s += "QProgressBar{border:1px solid #3e3e42;border-radius:6px;text-align:center;background:#252526;height:8px;color:#f0f0f0;}";
    s += "QProgressBar::chunk{background:" + a + ";border-radius:5px;}";
    s += "QDialog{background:#1e1e1e;}";
    s += "QFrame{border:0;}";
    return s;
}

QString ThemeManager::buildLightQss(const QColor& accent)
{
    return makeLightQss(accent);
}

QString ThemeManager::buildDarkQss(const QColor& accent)
{
    return makeDarkQss(accent);
}

void ThemeManager::apply(QApplication& app, const QColor& accent, Theme theme)
{
    app.setPalette(theme == Theme::Light ? buildLightPalette(accent) : buildDarkPalette(accent));
    app.setStyleSheet(theme == Theme::Light ? buildLightQss(accent) : buildDarkQss(accent));
    current_ = theme;
}

void ThemeManager::toggle(QApplication& app, const QColor& accent)
{
    apply(app, accent, current_ == Theme::Light ? Theme::Dark : Theme::Light);
}

void setTheme(QApplication& app, const QColor& accent, ThemeManager::Theme theme)
{
    ThemeManager::instance().apply(app, accent, theme);
}

} // namespace easy

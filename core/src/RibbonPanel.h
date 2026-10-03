#pragma once

#include <QAction>
#include <QColor>
#include <QGridLayout>
#include <QLabel>
#include <QString>
#include <QVector>
#include <QWidget>

namespace easy {

class RibbonPanel : public QWidget {
    Q_OBJECT

public:
    explicit RibbonPanel(QWidget* parent = nullptr);

    void addGroup(const QString& name);
    QAction* addAction(QAction* action, int group = 0);
    QWidget* addWidget(QWidget* widget, int group = 0);
    void addSeparator(int group = 0);
    void setAccentColor(const QColor& accent);
    void setThemeLight(bool light);

signals:
    void currentPanelChanged(QWidget* panel);

protected:
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    struct Group {
        QString name;
        QVector<QWidget*> widgets;
        bool hasSeparator = false;
    };

    QVector<Group> groups_;
    QColor accent_;
    bool themeLight_ = true;

    void rebuildLayout();
};

} // namespace easy

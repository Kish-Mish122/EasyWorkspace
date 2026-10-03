#pragma once
#include "Slide.h"
#include <ThemeManager.h>

#include <QMainWindow>
#include <QTimer>

class QAction;
class QGraphicsView;
class QLabel;
class QListWidget;
class QSpinBox;
class SlideScene;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    bool openFile(const QString& path);

protected:
    void closeEvent(QCloseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void buildUi();
    void newDeck();
    void open();
    bool save();
    bool saveAs();
    bool maybeSave();
    bool writeFile(const QString& path);
    void exportPdf();
    void startShow();

    void commitCurrent();
    void loadCurrent();
    void showSlide(int index);
    void refreshList();
    void updateThumb();
    void updateStatus();
    Slide makeSlide(bool titled) const;

    void addSlide(bool titled);
    void deleteSlide();
    void duplicateSlide();
    void moveSlide(int delta);

    void addItem(ItemData::Type type);
    void deleteSelected();
    void bringToFront();
    void applyFill();
    void applyTextColor();
    void applyBold(bool on);
    void applyFontSize(int size);
    void applyBackground();
    void applyTheme(int index);
    void onSelectionChanged();
    void onEdited();

    void setModified(bool modified);
    void setPath(const QString& path);
    QString docName() const;
    void zoomIn();
    void zoomOut();
    void zoomTo(double factor);
    void autoFitView();

    SlideScene* scene_ = nullptr;
    QGraphicsView* view_ = nullptr;
    QListWidget* list_ = nullptr;
    QSpinBox* sizeSpin_ = nullptr;
    QAction* boldAct_ = nullptr;
    QLabel* statusLabel_ = nullptr;
    QTimer thumbTimer_;

    QVector<Slide> slides_;
    int current_ = 0;
    int themeIndex_ = 0;
    QString path_;
    bool modified_ = false;
    double zoomLevel_ = 1.0;
};

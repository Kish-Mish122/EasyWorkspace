#pragma once
#include <QMainWindow>

class QLabel;
class QLineEdit;
class QTableView;
class QSpinBox;
class SpreadsheetModel;

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
    void newSheet();
    void open();
    bool save();
    bool saveAs();
    bool maybeSave();
    void importCsv();
    void exportCsv();
    void copySelection(bool cut);
    void pasteSelection();
    void deleteSelection();
    void applyBold(bool on);
    void applyFontSize(int size);
    void applyFill();
    void clearFill();
    void insertChart();
    void zoomIn();
    void zoomOut();
    void autoFitColumnWidth();
    void onCurrentChanged();
    void commitFormulaBar();
    void updateStats();
    void setModified(bool modified);
    void setPath(const QString& path);
    QString docName() const;

    SpreadsheetModel* model_ = nullptr;
    QTableView* view_ = nullptr;
    QLabel* nameLabel_ = nullptr;
    QLineEdit* formulaEdit_ = nullptr;
    QSpinBox* sizeBox_ = nullptr;
    QLabel* statsLabel_ = nullptr;
    QAction* boldAct_ = nullptr;
    QString path_;
    bool modified_ = false;
    double zoomLevel_ = 1.0;
};

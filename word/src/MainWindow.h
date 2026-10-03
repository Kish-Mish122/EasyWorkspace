#pragma once
#include <QMainWindow>
#include <QPointer>
#include <QTextCharFormat>
#include <QTextListFormat>

class QAction;
class QComboBox;
class QDialog;
class QFontComboBox;
class QLabel;
class QTextEdit;

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
    void newDocument();
    void open();
    bool save();
    bool saveAs();
    bool maybeSave();
    bool writeEzw(const QString& path);
    bool readEzw(const QString& path);
    void exportPdf();
    void exportOdt();
    void exportHtml();
    void insertImage();
    void insertTable();
    void showFindReplace();
    void mergeFormat(const QTextCharFormat& format);
    void toggleList(QTextListFormat::Style style);
    void syncToolbar();
    void updateStatus();
    void updateTitle();
    void setPath(const QString& path);
    QString docName() const;
    void zoomIn();
    void zoomOut();
    void zoomTo(double factor);
    void autoFitDocumentWidth();

    QTextEdit* edit_ = nullptr;
    QFontComboBox* fontBox_ = nullptr;
    QComboBox* sizeBox_ = nullptr;
    QAction* bold_ = nullptr;
    QAction* italic_ = nullptr;
    QAction* underline_ = nullptr;
    QAction* alignL_ = nullptr;
    QAction* alignC_ = nullptr;
    QAction* alignR_ = nullptr;
    QAction* alignJ_ = nullptr;
    QLabel* statsLabel_ = nullptr;
    QString path_;
    QPointer<QDialog> findDlg_;
    double zoomLevel_ = 1.0;
};

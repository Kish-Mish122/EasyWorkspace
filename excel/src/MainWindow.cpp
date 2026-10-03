#include "MainWindow.h"

#include "ChartDialog.h"

#include <Common.h>
#include <ThemeManager.h>
#include <FormulaEngine.h>
#include <SpreadsheetModel.h>

#include <QApplication>
#include <QComboBox>
#include <QFont>
#include <QSpinBox>
#include <QTransform>
#include <QWheelEvent>
#include <QClipboard>
#include <QCloseEvent>
#include <QColorDialog>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QJsonDocument>
#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QSaveFile>
#include <QStatusBar>
#include <QTableView>
#include <QToolBar>
#include <QVBoxLayout>
#include <algorithm>
#include <climits>

namespace {
const QString kAppTitle = QStringLiteral("EasySheets");
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), model_(new SpreadsheetModel(this))
{
    buildUi();
    easy::loadWindowState(this, QStringLiteral("EasyWorkspace"), QStringLiteral("EasySheets"));
    if (geometry().isEmpty()) {
        resize(1100, 720);
    }
    setPath({});
    onCurrentChanged();
}

QString MainWindow::docName() const
{
    return path_.isEmpty() ? QStringLiteral("Книга1") : QFileInfo(path_).fileName();
}

void MainWindow::setModified(bool modified)
{
    modified_ = modified;
    setWindowModified(modified);
}

void MainWindow::setPath(const QString& path)
{
    path_ = path;
    setWindowTitle(docName() + QStringLiteral("[*] — ") + kAppTitle);
    setWindowModified(modified_);
}

void MainWindow::buildUi()
{
    view_ = new QTableView;
    view_->setModel(model_);
    view_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    view_->horizontalHeader()->setDefaultSectionSize(96);
    view_->verticalHeader()->setDefaultSectionSize(24);
    view_->setShowGrid(true);
    view_->setStyleSheet(QStringLiteral("QTableView{gridline-color:#d1d5db;}"));

    nameLabel_ = new QLabel(QStringLiteral("A1"));
    nameLabel_->setMinimumWidth(56);
    nameLabel_->setAlignment(Qt::AlignCenter);
    nameLabel_->setStyleSheet(QStringLiteral("font-weight:bold;"));
    formulaEdit_ = new QLineEdit;
    formulaEdit_->setPlaceholderText(QStringLiteral("Значение или формула, например =SUM(A1:A5)"));

    auto* formulaBar = new QWidget;
    auto* fl = new QHBoxLayout(formulaBar);
    fl->setContentsMargins(6, 4, 6, 4);
    fl->addWidget(nameLabel_);
    fl->addWidget(new QLabel(QStringLiteral("fx")));
    fl->addWidget(formulaEdit_, 1);

    auto* central = new QWidget;
    auto* cl = new QVBoxLayout(central);
    cl->setContentsMargins(0, 0, 0, 0);
    cl->setSpacing(0);
    cl->addWidget(formulaBar);
    cl->addWidget(view_, 1);
    setCentralWidget(central);

    auto act = [this](const QString& text, const QKeySequence& shortcut, auto slot) {
        auto* a = new QAction(text, this);
        if (!shortcut.isEmpty())
            a->setShortcut(shortcut);
        connect(a, &QAction::triggered, this, slot);
        return a;
    };

    // ---- Файл ----
    auto* fileMenu = menuBar()->addMenu(QStringLiteral("&Файл"));
    auto* newAct = act(QStringLiteral("Создать"), QKeySequence::New, [this] { newSheet(); });
    auto* openAct = act(QStringLiteral("Открыть…"), QKeySequence::Open, [this] { open(); });
    auto* saveAct = act(QStringLiteral("Сохранить"), QKeySequence::Save, [this] { save(); });
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(act(QStringLiteral("Сохранить как…"), QKeySequence::SaveAs, [this] { saveAs(); }));
    fileMenu->addSeparator();
    fileMenu->addAction(act(QStringLiteral("Импорт CSV…"), {}, [this] { importCsv(); }));
    fileMenu->addAction(act(QStringLiteral("Экспорт в CSV…"), {}, [this] { exportCsv(); }));
    fileMenu->addSeparator();
    fileMenu->addAction(act(QStringLiteral("Выход"), QKeySequence::Quit, [this] { close(); }));

    // ---- Правка ----
    auto* editMenu = menuBar()->addMenu(QStringLiteral("&Правка"));
    editMenu->addAction(act(QStringLiteral("Вырезать"), QKeySequence::Cut, [this] { copySelection(true); }));
    editMenu->addAction(act(QStringLiteral("Копировать"), QKeySequence::Copy, [this] { copySelection(false); }));
    editMenu->addAction(act(QStringLiteral("Вставить"), QKeySequence::Paste, [this] { pasteSelection(); }));
    editMenu->addAction(act(QStringLiteral("Очистить"), QKeySequence::Delete, [this] { deleteSelection(); }));
    editMenu->addSeparator();
    editMenu->addAction(act(QStringLiteral("Выделить всё"), QKeySequence::SelectAll, [this] { view_->selectAll(); }));

    // ---- Формат ----
    auto* fmtMenu = menuBar()->addMenu(QStringLiteral("Фор&мат"));
    boldAct_ = new QAction(QStringLiteral("Ж"), this);
    boldAct_->setCheckable(true);
    boldAct_->setShortcut(QKeySequence::Bold);
    boldAct_->setToolTip(QStringLiteral("Полужирный (Ctrl+B)"));
    connect(boldAct_, &QAction::triggered, this, [this](bool on) { applyBold(on); });
    auto* fillAct = act(QStringLiteral("Цвет заливки…"), {}, [this] { applyFill(); });
    auto* noFillAct = act(QStringLiteral("Убрать заливку"), {}, [this] { clearFill(); });
    fmtMenu->addAction(boldAct_);
    fmtMenu->addAction(fillAct);
    fmtMenu->addAction(noFillAct);

    // ---- Вставка ----
    auto* insMenu = menuBar()->addMenu(QStringLiteral("&Вставка"));
    auto* chartAct = act(QStringLiteral("Диаграмма…"), {}, [this] { insertChart(); });
    insMenu->addAction(chartAct);

    // ---- Справка ----
    auto* helpMenu = menuBar()->addMenu(QStringLiteral("&Справка"));
    helpMenu->addAction(act(QStringLiteral("Функции и формулы"), {}, [this] {
        QMessageBox::information(
            this, kAppTitle,
            QStringLiteral("Формулы начинаются с «=».\\n\\n"
                           "Операции: + − * / ^ и скобки.\\n"
                           "Ссылки: A1, B7. Диапазоны: A1:B5 (внутри функций).\\n"
                           "Функции: SUM, AVG, MIN, MAX, COUNT\\n"
                           "(по-русски: СУММ, СРЗНАЧ, МИН, МАКС, СЧЁТ).\\n\\n"
                           "Пример: =SUM(A1:A5)*2 + B1/4"));
    }));
    helpMenu->addAction(act(QStringLiteral("О программе"), {}, [this] {
        QMessageBox::about(this, kAppTitle,
                           QStringLiteral("EasySheets 1.0 — электронные таблицы из пакета EasyWorkspace.\n"
                                          "Его формат: .ezx. (c) K1sh-M1sh"));
    }));

    // ---- Панель инструментов ----
    auto* tb = addToolBar(QStringLiteral("Главная"));
    tb->setMovable(false);
    tb->addAction(newAct);
    tb->addAction(openAct);
    tb->addAction(saveAct);
    tb->addSeparator();

    // Font size spin box
    sizeBox_ = new QSpinBox;
    sizeBox_->setRange(6, 120);
    sizeBox_->setValue(12);
    sizeBox_->setMaximumWidth(80);
    sizeBox_->setSuffix(QStringLiteral(" px"));
    connect(sizeBox_, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int size) {
        applyFontSize(size);
    });
    tb->addWidget(sizeBox_);

    tb->addAction(boldAct_);
    tb->addSeparator();
    tb->addAction(fillAct);
    tb->addSeparator();
    tb->addAction(chartAct);

    // Zoom actions
    auto* zoomInAct = act(QStringLiteral("Увеличить"), QKeySequence(Qt::CTRL | Qt::Key_Equal), [this] { zoomIn(); });
    auto* zoomOutAct = act(QStringLiteral("Уменьшить"), QKeySequence(Qt::CTRL | Qt::Key_Minus), [this] { zoomOut(); });

    // ---- Вид ----
    auto* viewMenu = menuBar()->addMenu(QStringLiteral("&Вид"));
    viewMenu->addAction(zoomInAct);
    viewMenu->addAction(zoomOutAct);
    viewMenu->addSeparator();
    viewMenu->addAction(act(QStringLiteral("Автоподстройка ширины столбца"), {}, [this] { autoFitColumnWidth(); }));

    statsLabel_ = new QLabel;
    statusBar()->addPermanentWidget(statsLabel_);

    // ---- Связи ----
    connect(view_->selectionModel(), &QItemSelectionModel::currentChanged, this, [this] { onCurrentChanged(); });
    connect(view_->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this] { updateStats(); });
    connect(formulaEdit_, &QLineEdit::returnPressed, this, [this] { commitFormulaBar(); });
    connect(model_, &QAbstractItemModel::dataChanged, this, [this] {
        setModified(true);
        onCurrentChanged();
    });
}

// ---------------------------------------------------------------- состояние

void MainWindow::onCurrentChanged()
{
    const QModelIndex cur = view_->currentIndex();
    if (!cur.isValid()) {
        nameLabel_->setText(QStringLiteral("A1"));
        formulaEdit_->clear();
        return;
    }
    nameLabel_->setText(FormulaEngine::cellName(cur.row(), cur.column()));
    if (!formulaEdit_->hasFocus())
        formulaEdit_->setText(model_->rawText(cur.row(), cur.column()));
    boldAct_->setChecked(model_->isBold(cur.row(), cur.column()));
    updateStats();
}

void MainWindow::commitFormulaBar()
{
    const QModelIndex cur = view_->currentIndex();
    if (!cur.isValid())
        return;
    model_->setData(cur, formulaEdit_->text());
    view_->setFocus();
}

void MainWindow::updateStats()
{
    const QModelIndexList sel = view_->selectionModel()->selectedIndexes();
    int count = 0;
    double sum = 0;
    for (const QModelIndex& i : sel) {
        double v = 0;
        if (model_->numberAt(i.row(), i.column(), v)) {
            sum += v;
            ++count;
        }
    }
    if (count == 0)
        statsLabel_->clear();
    else
        statsLabel_->setText(QStringLiteral("Количество: %1   Сумма: %2   Среднее: %3")
                                 .arg(count)
                                 .arg(sum, 0, 'g', 10)
                                 .arg(sum / count, 0, 'g', 10));
}

// ---------------------------------------------------------------- файлы

bool MainWindow::maybeSave()
{
    if (!modified_)
        return true;
    const auto r = easy::askSave(this, docName());
    if (r == QMessageBox::Save)
        return save();
    return r == QMessageBox::Discard;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (maybeSave()) {
        easy::saveWindowState(this, QStringLiteral("EasyOffice"), QStringLiteral("EasyExcel"));
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::newSheet()
{
    if (!maybeSave())
        return;
    model_->clearAll();
    setModified(false);
    setPath({});
}

void MainWindow::open()
{
    if (!maybeSave())
        return;
    const QString path = QFileDialog::getOpenFileName(
        this, QStringLiteral("Открыть"), {}, QStringLiteral("Таблицы EasyExcel (*.ezx);;Все файлы (*)"));
    if (!path.isEmpty())
        openFile(path);
}

bool MainWindow::openFile(const QString& path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, kAppTitle, QStringLiteral("Не удалось открыть файл:\n%1").arg(path));
        return false;
    }
    if (path.endsWith(QStringLiteral(".csv"), Qt::CaseInsensitive)) {
        model_->fromCsv(QString::fromUtf8(f.readAll()));
        setModified(false);
        setPath({});
        return true;
    }
    QJsonParseError err;
    const QJsonDocument jd = QJsonDocument::fromJson(f.readAll(), &err);
    if (err.error != QJsonParseError::NoError || !jd.isObject() || !model_->fromJson(jd.object())) {
        QMessageBox::warning(this, kAppTitle, QStringLiteral("Файл повреждён или имеет неверный формат."));
        return false;
    }
    setModified(false);
    setPath(path);
    return true;
}

bool MainWindow::save()
{
    if (path_.isEmpty())
        return saveAs();
    QSaveFile f(path_);
    if (!f.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, kAppTitle, QStringLiteral("Не удалось сохранить файл:\n%1").arg(path_));
        return false;
    }
    f.write(QJsonDocument(model_->toJson()).toJson(QJsonDocument::Indented));
    if (!f.commit()) {
        QMessageBox::warning(this, kAppTitle, QStringLiteral("Ошибка записи файла."));
        return false;
    }
    setModified(false);
    return true;
}

bool MainWindow::saveAs()
{
    QString path = QFileDialog::getSaveFileName(this, QStringLiteral("Сохранить как"),
                                                path_.isEmpty() ? docName() : path_,
                                                QStringLiteral("Таблица EasyExcel (*.ezx)"));
    if (path.isEmpty())
        return false;
    if (QFileInfo(path).suffix().isEmpty())
        path += QStringLiteral(".ezx");
    const QString old = path_;
    path_ = path;
    if (!save()) {
        path_ = old;
        return false;
    }
    setPath(path);
    return true;
}

void MainWindow::importCsv()
{
    if (!maybeSave())
        return;
    const QString path = QFileDialog::getOpenFileName(this, QStringLiteral("Импорт CSV"), {},
                                                      QStringLiteral("CSV (*.csv *.txt);;Все файлы (*)"));
    if (path.isEmpty())
        return;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, kAppTitle, QStringLiteral("Не удалось открыть файл."));
        return;
    }
    model_->fromCsv(QString::fromUtf8(f.readAll()));
    setModified(false);
    setPath({});
}

void MainWindow::exportCsv()
{
    QString path = QFileDialog::getSaveFileName(this, QStringLiteral("Экспорт в CSV"), {}, QStringLiteral("CSV (*.csv)"));
    if (path.isEmpty())
        return;
    if (QFileInfo(path).suffix().isEmpty())
        path += QStringLiteral(".csv");
    QSaveFile f(path);
    if (!f.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, kAppTitle, QStringLiteral("Не удалось записать файл."));
        return;
    }
    f.write(model_->toCsv().toUtf8());
    f.commit();
    statusBar()->showMessage(QStringLiteral("CSV сохранён: %1").arg(path), 5000);
}

// ---------------------------------------------------------------- правка

void MainWindow::copySelection(bool cut)
{
    const QModelIndexList sel = view_->selectionModel()->selectedIndexes();
    if (sel.isEmpty())
        return;
    int r0 = INT_MAX, r1 = -1, c0 = INT_MAX, c1 = -1;
    for (const QModelIndex& i : sel) {
        r0 = std::min(r0, i.row());
        r1 = std::max(r1, i.row());
        c0 = std::min(c0, i.column());
        c1 = std::max(c1, i.column());
    }
    QString text;
    for (int r = r0; r <= r1; ++r) {
        QStringList cells;
        for (int c = c0; c <= c1; ++c)
            cells << model_->rawText(r, c);
        text += cells.join('\t');
        if (r < r1)
            text += '\n';
    }
    QApplication::clipboard()->setText(text);
    if (cut)
        model_->clearCells(sel);
}

void MainWindow::pasteSelection()
{
    const QModelIndex cur = view_->currentIndex();
    if (!cur.isValid())
        return;
    const QString text = QApplication::clipboard()->text();
    if (text.isEmpty())
        return;
    const QStringList rows = text.split('\n');
    for (int r = 0; r < rows.size(); ++r) {
        if (r == rows.size() - 1 && rows[r].isEmpty())
            break; // завершающий перевод строки
        const QStringList cols = rows[r].split('\t');
        for (int c = 0; c < cols.size(); ++c) {
            const int rr = cur.row() + r, cc = cur.column() + c;
            if (rr < model_->rowCount() && cc < model_->columnCount())
                model_->setData(model_->index(rr, cc), QString(cols[c]).remove('\r'));
        }
    }
}

void MainWindow::deleteSelection()
{
    model_->clearCells(view_->selectionModel()->selectedIndexes());
}

void MainWindow::applyBold(bool on)
{
    model_->setBold(view_->selectionModel()->selectedIndexes(), on);
}

void MainWindow::applyFill()
{
    const QColor c = QColorDialog::getColor(QColor(0xff, 0xf2, 0x9e), this, QStringLiteral("Цвет заливки"));
    if (c.isValid())
        model_->setFill(view_->selectionModel()->selectedIndexes(), c);
}

void MainWindow::clearFill()
{
    model_->setFill(view_->selectionModel()->selectedIndexes(), QColor());
}

void MainWindow::insertChart()
{
    const QModelIndexList sel = view_->selectionModel()->selectedIndexes();
    if (sel.isEmpty()) {
        QMessageBox::information(this, kAppTitle,
                                 QStringLiteral("Выделите диапазон с числами.\n"
                                                "Если столбцов два, первый станет подписями, второй — значениями."));
        return;
    }
    int r0 = INT_MAX, r1 = -1, c0 = INT_MAX, c1 = -1;
    for (const QModelIndex& i : sel) {
        r0 = std::min(r0, i.row());
        r1 = std::max(r1, i.row());
        c0 = std::min(c0, i.column());
        c1 = std::max(c1, i.column());
    }
    const bool twoCols = c1 > c0;
    const int valueCol = twoCols ? c0 + 1 : c0;

    QStringList labels;
    QVector<double> values;
    for (int r = r0; r <= r1; ++r) {
        double v = 0;
        if (!model_->numberAt(r, valueCol, v))
            continue;
        labels << (twoCols ? model_->displayText(r, c0) : FormulaEngine::cellName(r, valueCol));
        values << v;
    }
    if (values.isEmpty()) {
        QMessageBox::warning(this, kAppTitle, QStringLiteral("В выделенном диапазоне нет чисел."));
        return;
    }
    ChartDialog dlg(labels, values, this);
    dlg.exec();
}

// ---------------------------------------------------------------- шрифт и зум

void MainWindow::applyFontSize(int size)
{
    const QModelIndexList sel = view_->selectionModel()->selectedIndexes();
    if (sel.isEmpty()) return;
    
    model_->setFontSize(sel, size);
}

void MainWindow::zoomIn()
{
    zoomLevel_ = qMin(zoomLevel_ + 0.1, 3.0);
    view_->horizontalHeader()->setDefaultSectionSize(static_cast<int>(96 * zoomLevel_));
    view_->verticalHeader()->setDefaultSectionSize(static_cast<int>(24 * zoomLevel_));
    
    // Also scale the font
    QFont viewFont = view_->font();
    viewFont.setPointSize(static_cast<int>(viewFont.pointSizeF() * zoomLevel_));
    view_->setFont(viewFont);
}

void MainWindow::zoomOut()
{
    zoomLevel_ = qMax(zoomLevel_ - 0.1, 0.3);
    view_->horizontalHeader()->setDefaultSectionSize(static_cast<int>(96 * zoomLevel_));
    view_->verticalHeader()->setDefaultSectionSize(static_cast<int>(24 * zoomLevel_));
    
    // Also scale the font
    QFont viewFont = view_->font();
    viewFont.setPointSize(static_cast<int>(viewFont.pointSizeF() * zoomLevel_));
    view_->setFont(viewFont);
}

void MainWindow::autoFitColumnWidth()
{
    const int colCount = model_->columnCount();
    for (int col = 0; col < colCount; ++col) {
        int maxWidth = 0;
        const int rowCount = model_->rowCount();
        for (int row = 0; row < rowCount; ++row) {
            QString text = model_->displayText(row, col);
            
            // Calculate text width
            QFontMetrics fm(view_->font());
            int textWidth = fm.horizontalAdvance(text);
            maxWidth = qMax(maxWidth, textWidth);
        }
        // Add padding and set column width
        view_->horizontalHeader()->resizeSection(col, qMin(maxWidth + 20, 400));
    }
}

void MainWindow::wheelEvent(QWheelEvent* event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        int delta = event->angleDelta().y();
        if (delta > 0) {
            zoomIn();
        } else if (delta < 0) {
            zoomOut();
        }
        event->accept();
    } else {
        QMainWindow::wheelEvent(event);
    }
}

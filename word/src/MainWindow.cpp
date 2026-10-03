#include "MainWindow.h"

#include <Common.h>
#include <ThemeManager.h>
#include <PageSettingsDialog.h>

#include <QActionGroup>
#include <QStackedWidget>
#include <algorithm>
#include <QBuffer>
#include <QCloseEvent>
#include <QColorDialog>
#include <QComboBox>
#include <QDialog>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QImage>
#include <QInputDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QPageSize>
#include <QPdfWriter>
#include <QPixmap>
#include <QPushButton>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSet>
#include <QSignalBlocker>
#include <QStatusBar>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextDocumentWriter>
#include <QTextEdit>
#include <QTextList>
#include <QTextTable>
#include <QToolBar>
#include <QUrl>
#include <QUuid>
#include <QVBoxLayout>

namespace {
const QString kAppTitle = QStringLiteral("EasyWrite");
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    buildUi();
    easy::loadWindowState(this, QStringLiteral("EasyWorkspace"), QStringLiteral("EasyWrite"));
    if (geometry().isEmpty()) {
        resize(1100, 800);
    }
    setPath({});
    updateStatus();
}

QString MainWindow::docName() const
{
    return path_.isEmpty() ? QStringLiteral("Без названия") : QFileInfo(path_).fileName();
}

void MainWindow::updateTitle()
{
    setWindowTitle(docName() + QStringLiteral("[*] — ") + kAppTitle);
    setWindowModified(edit_->document()->isModified());
}

void MainWindow::setPath(const QString& path)
{
    path_ = path;
    updateTitle();
}

void MainWindow::buildUi()
{
    edit_ = new QTextEdit;
    edit_->setFrameShape(QFrame::NoFrame);
    edit_->setFixedWidth(794); // ширина листа A4 при 96 dpi
    edit_->document()->setDefaultFont(QFont(QStringLiteral("Arial"), 12));
    edit_->document()->setDocumentMargin(56);
    edit_->setAcceptRichText(true);

    auto* host = new QWidget;
    host->setObjectName(QStringLiteral("host"));
    host->setStyleSheet(QStringLiteral("#host{background:#d9dce1;}"));
    auto* hostLayout = new QHBoxLayout(host);
    hostLayout->setContentsMargins(0, 16, 0, 16);
    hostLayout->addStretch();
    hostLayout->addWidget(edit_);
    hostLayout->addStretch();
    setCentralWidget(host);

    auto act = [this](const QString& text, const QKeySequence& shortcut, auto slot) {
        auto* a = new QAction(text, this);
        if (!shortcut.isEmpty())
            a->setShortcut(shortcut);
        connect(a, &QAction::triggered, this, slot);
        return a;
    };

    // ---- Файл ----
    auto* fileMenu = menuBar()->addMenu(QStringLiteral("&Файл"));
    fileMenu->addAction(act(QStringLiteral("Создать"), QKeySequence::New, [this] { newDocument(); }));
    fileMenu->addAction(act(QStringLiteral("Открыть…"), QKeySequence::Open, [this] { open(); }));
    fileMenu->addAction(act(QStringLiteral("Сохранить"), QKeySequence::Save, [this] { save(); }));
    fileMenu->addAction(act(QStringLiteral("Сохранить как…"), QKeySequence::SaveAs, [this] { saveAs(); }));
    fileMenu->addSeparator();
    auto* exportMenu = fileMenu->addMenu(QStringLiteral("Экспорт"));
    exportMenu->addAction(act(QStringLiteral("PDF…"), {}, [this] { exportPdf(); }));
    exportMenu->addAction(act(QStringLiteral("ODT (LibreOffice)…"), {}, [this] { exportOdt(); }));
    exportMenu->addAction(act(QStringLiteral("HTML…"), {}, [this] { exportHtml(); }));
    fileMenu->addSeparator();
    fileMenu->addAction(act(QStringLiteral("Выход"), QKeySequence::Quit, [this] { close(); }));

    // ---- Правка ----
    auto* editMenu = menuBar()->addMenu(QStringLiteral("&Правка"));
    editMenu->addAction(act(QStringLiteral("Отменить"), QKeySequence::Undo, [this] { edit_->undo(); }));
    editMenu->addAction(act(QStringLiteral("Повторить"), QKeySequence::Redo, [this] { edit_->redo(); }));
    editMenu->addSeparator();
    editMenu->addAction(act(QStringLiteral("Вырезать"), QKeySequence::Cut, [this] { edit_->cut(); }));
    editMenu->addAction(act(QStringLiteral("Копировать"), QKeySequence::Copy, [this] { edit_->copy(); }));
    editMenu->addAction(act(QStringLiteral("Вставить"), QKeySequence::Paste, [this] { edit_->paste(); }));
    editMenu->addAction(act(QStringLiteral("Выделить всё"), QKeySequence::SelectAll, [this] { edit_->selectAll(); }));
    editMenu->addSeparator();
    editMenu->addAction(act(QStringLiteral("Найти и заменить…"), QKeySequence::Find, [this] { showFindReplace(); }));

    // ---- Формат ----
    auto* fmtMenu = menuBar()->addMenu(QStringLiteral("Фор&мат"));
    auto* pageSetupAct = act(QStringLiteral("Настройки страницы…"), {}, [this] {
        easy::PageSettingsDialog dlg(this);
        if (dlg.exec() != QDialog::Accepted) return;
        QPageSize pageSize = dlg.pageSize();
        QMargins margins = dlg.margins();
        
        // Apply page settings
        QSizeF pageSizeMM = pageSize.size(QPageSize::Millimeter);
        edit_->document()->setPageSize(pageSizeMM);
        edit_->document()->setDocumentMargin(qMax(margins.left(), margins.right()));
    });

    bold_ = new QAction(QStringLiteral("Ж"), this);
    bold_->setCheckable(true);
    bold_->setShortcut(QKeySequence::Bold);
    bold_->setToolTip(QStringLiteral("Полужирный (Ctrl+B)"));
    connect(bold_, &QAction::triggered, this, [this](bool on) {
        QTextCharFormat f;
        f.setFontWeight(on ? QFont::Bold : QFont::Normal);
        mergeFormat(f);
    });

    italic_ = new QAction(QStringLiteral("К"), this);
    italic_->setCheckable(true);
    italic_->setShortcut(QKeySequence::Italic);
    italic_->setToolTip(QStringLiteral("Курсив (Ctrl+I)"));
    connect(italic_, &QAction::triggered, this, [this](bool on) {
        QTextCharFormat f;
        f.setFontItalic(on);
        mergeFormat(f);
    });

    underline_ = new QAction(QStringLiteral("Ч"), this);
    underline_->setCheckable(true);
    underline_->setShortcut(QKeySequence::Underline);
    underline_->setToolTip(QStringLiteral("Подчёркнутый (Ctrl+U)"));
    connect(underline_, &QAction::triggered, this, [this](bool on) {
        QTextCharFormat f;
        f.setFontUnderline(on);
        mergeFormat(f);
    });

    auto* textColor = act(QStringLiteral("Цвет текста…"), {}, [this] {
        const QColor c = QColorDialog::getColor(Qt::black, this, QStringLiteral("Цвет текста"));
        if (!c.isValid())
            return;
        QTextCharFormat f;
        f.setForeground(c);
        mergeFormat(f);
    });
    auto* highlight = act(QStringLiteral("Выделение цветом…"), {}, [this] {
        const QColor c = QColorDialog::getColor(Qt::yellow, this, QStringLiteral("Цвет выделения"));
        if (!c.isValid())
            return;
        QTextCharFormat f;
        f.setBackground(c);
        mergeFormat(f);
    });
    auto* clearHighlight = act(QStringLiteral("Убрать выделение цветом"), {}, [this] {
        QTextCharFormat f;
        f.setBackground(Qt::transparent);
        mergeFormat(f);
    });

    auto* alignGroup = new QActionGroup(this);
    auto makeAlign = [&](const QString& text, Qt::Alignment al, const QString& tip) {
        auto* a = new QAction(text, this);
        a->setCheckable(true);
        a->setToolTip(tip);
        alignGroup->addAction(a);
        connect(a, &QAction::triggered, this, [this, al] { edit_->setAlignment(al); });
        return a;
    };
    alignL_ = makeAlign(QStringLiteral("⇤"), Qt::AlignLeft | Qt::AlignAbsolute, QStringLiteral("По левому краю"));
    alignC_ = makeAlign(QStringLiteral("↔"), Qt::AlignHCenter, QStringLiteral("По центру"));
    alignR_ = makeAlign(QStringLiteral("⇥"), Qt::AlignRight | Qt::AlignAbsolute, QStringLiteral("По правому краю"));
    alignJ_ = makeAlign(QStringLiteral("☰"), Qt::AlignJustify, QStringLiteral("По ширине"));
    alignL_->setChecked(true);

    auto* bullets = act(QStringLiteral("• Список"), {}, [this] { toggleList(QTextListFormat::ListDisc); });
    auto* numbers = act(QStringLiteral("1. Список"), {}, [this] { toggleList(QTextListFormat::ListDecimal); });
    bullets->setToolTip(QStringLiteral("Маркированный список"));
    numbers->setToolTip(QStringLiteral("Нумерованный список"));

    fmtMenu->addAction(bold_);
    fmtMenu->addAction(italic_);
    fmtMenu->addAction(underline_);
    fmtMenu->addSeparator();
    fmtMenu->addAction(textColor);
    fmtMenu->addAction(highlight);
    fmtMenu->addAction(clearHighlight);
    fmtMenu->addSeparator();
    fmtMenu->addActions(alignGroup->actions());
    fmtMenu->addSeparator();
    fmtMenu->addAction(bullets);
    fmtMenu->addAction(numbers);
    fmtMenu->addSeparator();
    fmtMenu->addAction(pageSetupAct);

    // ---- Вставка ----
    auto* insMenu = menuBar()->addMenu(QStringLiteral("&Вставка"));
    auto* imageAct = act(QStringLiteral("Изображение…"), {}, [this] { insertImage(); });
    auto* tableAct = act(QStringLiteral("Таблица…"), {}, [this] { insertTable(); });
    insMenu->addAction(imageAct);
    insMenu->addAction(tableAct);

    // ---- Вид ----
    auto* viewMenu = menuBar()->addMenu(QStringLiteral("&Вид"));
    auto* zoomInAct = act(QStringLiteral("Увеличить"), QKeySequence::ZoomIn, [this] { zoomIn(); });
    auto* zoomOutAct = act(QStringLiteral("Уменьшить"), QKeySequence::ZoomOut, [this] { zoomOut(); });
    auto* zoomTo100Act = act(QStringLiteral("100%"), {}, [this] { zoomTo(1.0); });
    auto* fitWidthAct = act(QStringLiteral("По ширине страницы"), {}, [this] { autoFitDocumentWidth(); });
    
    viewMenu->addAction(zoomInAct);
    viewMenu->addAction(zoomOutAct);
    viewMenu->addSeparator();
    viewMenu->addAction(zoomTo100Act);
    viewMenu->addAction(fitWidthAct);

    // ---- Справка ----
    auto* helpMenu = menuBar()->addMenu(QStringLiteral("&Справка"));
    helpMenu->addAction(act(QStringLiteral("О программе"), {}, [this] {
        QMessageBox::about(this, QStringLiteral("EasyWrite"),
                           QStringLiteral("EasyWrite 1.0 — текстовый редактор из пакета EasyWorkspace.\n"
                                          "Его формат: .ewd. (c) K1sh-M1sh"));
    }));

    // ---- Панель инструментов ----
    auto* tb = addToolBar(QStringLiteral("Главная"));
    tb->setMovable(false);

    tb->addAction(act(QStringLiteral("Создать"), {}, [this] { newDocument(); }));
    tb->addAction(act(QStringLiteral("Открыть"), {}, [this] { open(); }));
    tb->addAction(act(QStringLiteral("Сохранить"), {}, [this] { save(); }));
    tb->addSeparator();

    fontBox_ = new QFontComboBox;
    fontBox_->setMaximumWidth(180);
    fontBox_->setCurrentFont(edit_->document()->defaultFont());
    connect(fontBox_, &QFontComboBox::currentFontChanged, this, [this](const QFont& font) {
        QTextCharFormat f;
        f.setFontFamilies({font.family()});
        mergeFormat(f);
        edit_->setFocus();
    });
    tb->addWidget(fontBox_);

    sizeBox_ = new QComboBox;
    sizeBox_->setEditable(true);
    sizeBox_->setMaximumWidth(64);
    const int fontSizes[] = {6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 18, 20, 22, 24, 26, 28, 30, 32, 36, 40, 44, 48, 54, 60, 66, 72, 90, 100, 120, 144};
    for (int s : fontSizes)
        sizeBox_->addItem(QString::number(s));
    sizeBox_->setCurrentText(QStringLiteral("12"));
    connect(sizeBox_, &QComboBox::textActivated, this, [this](const QString& text) {
        const double pt = text.toDouble();
        if (pt < 1.0)
            return;
        QTextCharFormat f;
        f.setFontPointSize(pt);
        mergeFormat(f);
        edit_->setFocus();
    });
    tb->addWidget(sizeBox_);
    tb->addSeparator();

    tb->addAction(bold_);
    tb->addAction(italic_);
    tb->addAction(underline_);
    tb->addSeparator();
    tb->addActions(alignGroup->actions());
    tb->addSeparator();
    tb->addAction(bullets);
    tb->addAction(numbers);
    tb->addSeparator();
    tb->addAction(textColor);
    tb->addAction(highlight);
    tb->addSeparator();
    tb->addAction(imageAct);
    tb->addAction(tableAct);

    // ---- Статусная строка ----
    statsLabel_ = new QLabel;
    statusBar()->addPermanentWidget(statsLabel_);

    connect(edit_, &QTextEdit::currentCharFormatChanged, this, [this] { syncToolbar(); });
    connect(edit_, &QTextEdit::cursorPositionChanged, this, [this] { syncToolbar(); });
    connect(edit_, &QTextEdit::textChanged, this, [this] { updateStatus(); });
    connect(edit_->document(), &QTextDocument::modificationChanged, this, [this] { updateTitle(); });
}

void MainWindow::mergeFormat(const QTextCharFormat& format)
{
    QTextCursor cursor = edit_->textCursor();
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(format);
    edit_->mergeCurrentCharFormat(format);
}

void MainWindow::toggleList(QTextListFormat::Style style)
{
    QTextCursor cursor = edit_->textCursor();
    cursor.beginEditBlock();
    if (QTextList* list = cursor.currentList(); list && list->format().style() == style) {
        list->remove(cursor.block());
        QTextBlockFormat bf = cursor.blockFormat();
        bf.setIndent(0);
        cursor.setBlockFormat(bf);
    } else {
        cursor.createList(style);
    }
    cursor.endEditBlock();
}

void MainWindow::syncToolbar()
{
    const QTextCharFormat f = edit_->currentCharFormat();
    bold_->setChecked(f.fontWeight() >= QFont::Bold);
    italic_->setChecked(f.fontItalic());
    underline_->setChecked(f.fontUnderline());

    {
        const QSignalBlocker b1(fontBox_);
        const QSignalBlocker b2(sizeBox_);
        const QFont base = edit_->document()->defaultFont();
        fontBox_->setCurrentFont(f.hasProperty(QTextFormat::FontFamilies) ? f.font() : base);
        const double pt = f.fontPointSize() > 0 ? f.fontPointSize() : base.pointSizeF();
        sizeBox_->setCurrentText(QString::number(pt, 'g', 4));
    }

    const Qt::Alignment al = edit_->alignment();
    if (al.testFlag(Qt::AlignHCenter))
        alignC_->setChecked(true);
    else if (al.testFlag(Qt::AlignRight))
        alignR_->setChecked(true);
    else if (al.testFlag(Qt::AlignJustify))
        alignJ_->setChecked(true);
    else
        alignL_->setChecked(true);
}

void MainWindow::updateStatus()
{
    const QString text = edit_->toPlainText();
    const int words = text.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts).size();
    statsLabel_->setText(QStringLiteral("Слов: %1   Символов: %2").arg(words).arg(text.size()));
}

// ---------------------------------------------------------------- файлы

bool MainWindow::maybeSave()
{
    if (!edit_->document()->isModified())
        return true;
    const auto r = easy::askSave(this, docName());
    if (r == QMessageBox::Save)
        return save();
    return r == QMessageBox::Discard;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (maybeSave()) {
        easy::saveWindowState(this, QStringLiteral("EasyOffice"), QStringLiteral("EasyWord"));
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::newDocument()
{
    if (!maybeSave())
        return;
    edit_->clear();
    edit_->document()->setModified(false);
    setPath({});
}

void MainWindow::open()
{
    if (!maybeSave())
        return;
    const QString path = QFileDialog::getOpenFileName(
        this, QStringLiteral("Открыть"), {},
        QStringLiteral("Документы (*.ezw *.html *.htm *.txt);;Все файлы (*)"));
    if (!path.isEmpty())
        openFile(path);
}

bool MainWindow::openFile(const QString& path)
{
    const QString suffix = QFileInfo(path).suffix().toLower();
    if (suffix == QLatin1String("ezw")) {
        if (!readEzw(path))
            return false;
        edit_->document()->setModified(false);
        setPath(path);
        return true;
    }

    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, kAppTitle, QStringLiteral("Не удалось открыть файл:\n%1").arg(path));
        return false;
    }
    const QString content = QString::fromUtf8(f.readAll());
    if (suffix == QLatin1String("html") || suffix == QLatin1String("htm"))
        edit_->setHtml(content);
    else
        edit_->setPlainText(content);
    edit_->document()->setModified(false);
    setPath({}); // чужой формат: «Сохранить» предложит .ezw
    return true;
}

bool MainWindow::save()
{
    if (path_.isEmpty())
        return saveAs();
    if (!writeEzw(path_))
        return false;
    edit_->document()->setModified(false);
    return true;
}

bool MainWindow::saveAs()
{
    QString path = QFileDialog::getSaveFileName(
        this, QStringLiteral("Сохранить как"), path_.isEmpty() ? docName() : path_,
        QStringLiteral("Документ EasyWord (*.ezw)"));
    if (path.isEmpty())
        return false;
    if (QFileInfo(path).suffix().isEmpty())
        path += QStringLiteral(".ezw");
    if (!writeEzw(path))
        return false;
    edit_->document()->setModified(false);
    setPath(path);
    return true;
}

// Формат .ezw — JSON: HTML документа + картинки в base64 (PNG).
bool MainWindow::writeEzw(const QString& path)
{
    QTextDocument* doc = edit_->document();

    QSet<QString> imageNames;
    for (QTextBlock b = doc->begin(); b.isValid(); b = b.next())
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QTextCharFormat cf = it.fragment().charFormat();
            if (cf.isImageFormat())
                imageNames.insert(cf.toImageFormat().name());
        }

    QJsonObject images;
    for (const QString& name : std::as_const(imageNames)) {
        const QVariant res = doc->resource(QTextDocument::ImageResource, QUrl(name));
        QImage img = res.value<QImage>();
        if (img.isNull())
            img = res.value<QPixmap>().toImage();
        if (img.isNull())
            continue;
        QByteArray bytes;
        QBuffer buf(&bytes);
        buf.open(QIODevice::WriteOnly);
        img.save(&buf, "PNG");
        images.insert(name, QString::fromLatin1(bytes.toBase64()));
    }

    QJsonObject root;
    root.insert(QStringLiteral("format"), QStringLiteral("ezw"));
    root.insert(QStringLiteral("version"), 1);
    root.insert(QStringLiteral("html"), doc->toHtml());
    root.insert(QStringLiteral("images"), images);

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, kAppTitle, QStringLiteral("Не удалось сохранить файл:\n%1").arg(path));
        return false;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Compact));
    if (!file.commit()) {
        QMessageBox::warning(this, kAppTitle, QStringLiteral("Ошибка записи файла:\n%1").arg(path));
        return false;
    }
    return true;
}

bool MainWindow::readEzw(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, kAppTitle, QStringLiteral("Не удалось открыть файл:\n%1").arg(path));
        return false;
    }
    QJsonParseError err;
    const QJsonDocument jd = QJsonDocument::fromJson(file.readAll(), &err);
    if (err.error != QJsonParseError::NoError || !jd.isObject()
        || jd.object().value(QStringLiteral("format")).toString() != QLatin1String("ezw")) {
        QMessageBox::warning(this, kAppTitle, QStringLiteral("Файл повреждён или имеет неверный формат."));
        return false;
    }

    const QJsonObject root = jd.object();
    QTextDocument* doc = edit_->document();
    edit_->setHtml(root.value(QStringLiteral("html")).toString());

    const QJsonObject images = root.value(QStringLiteral("images")).toObject();
    for (auto it = images.begin(); it != images.end(); ++it) {
        QImage img;
        img.loadFromData(QByteArray::fromBase64(it.value().toString().toLatin1()), "PNG");
        if (!img.isNull())
            doc->addResource(QTextDocument::ImageResource, QUrl(it.key()), img);
    }
    if (!images.isEmpty())
        doc->markContentsDirty(0, doc->characterCount()); // перерисовать картинки
    return true;
}

// ---------------------------------------------------------------- экспорт

void MainWindow::exportPdf()
{
    QString path = QFileDialog::getSaveFileName(this, QStringLiteral("Экспорт в PDF"), {},
                                                QStringLiteral("PDF (*.pdf)"));
    if (path.isEmpty())
        return;
    if (QFileInfo(path).suffix().isEmpty())
        path += QStringLiteral(".pdf");

    QPdfWriter writer(path);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageMargins(QMarginsF(20, 20, 20, 20), QPageLayout::Millimeter);
    edit_->document()->print(&writer);
    statusBar()->showMessage(QStringLiteral("PDF сохранён: %1").arg(path), 5000);
}

void MainWindow::exportOdt()
{
    QString path = QFileDialog::getSaveFileName(this, QStringLiteral("Экспорт в ODT"), {},
                                                QStringLiteral("OpenDocument (*.odt)"));
    if (path.isEmpty())
        return;
    if (QFileInfo(path).suffix().isEmpty())
        path += QStringLiteral(".odt");

    QTextDocumentWriter writer(path, "odf");
    if (writer.write(edit_->document()))
        statusBar()->showMessage(QStringLiteral("ODT сохранён: %1").arg(path), 5000);
    else
        QMessageBox::warning(this, kAppTitle, QStringLiteral("Не удалось записать ODT."));
}

void MainWindow::exportHtml()
{
    QString path = QFileDialog::getSaveFileName(this, QStringLiteral("Экспорт в HTML"), {},
                                                QStringLiteral("HTML (*.html)"));
    if (path.isEmpty())
        return;
    if (QFileInfo(path).suffix().isEmpty())
        path += QStringLiteral(".html");

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, kAppTitle, QStringLiteral("Не удалось записать файл."));
        return;
    }
    file.write(edit_->document()->toHtml().toUtf8());
    file.commit();
    statusBar()->showMessage(QStringLiteral("HTML сохранён: %1").arg(path), 5000);
}

// ---------------------------------------------------------------- вставка

void MainWindow::insertImage()
{
    const QString path = QFileDialog::getOpenFileName(
        this, QStringLiteral("Вставить изображение"), {},
        QStringLiteral("Изображения (*.png *.jpg *.jpeg *.bmp *.gif)"));
    if (path.isEmpty())
        return;
    const QImage img(path);
    if (img.isNull()) {
        QMessageBox::warning(this, kAppTitle, QStringLiteral("Не удалось прочитать изображение."));
        return;
    }
    const QString name = QStringLiteral("img_") + QUuid::createUuid().toString(QUuid::Id128) + QStringLiteral(".png");
    edit_->document()->addResource(QTextDocument::ImageResource, QUrl(name), img);

    QTextImageFormat fmt;
    fmt.setName(name);
    fmt.setWidth(std::min(img.width(), 600));
    edit_->textCursor().insertImage(fmt);
}

void MainWindow::insertTable()
{
    bool ok = false;
    const int rows = QInputDialog::getInt(this, QStringLiteral("Таблица"), QStringLiteral("Строк:"), 3, 1, 50, 1, &ok);
    if (!ok)
        return;
    const int cols = QInputDialog::getInt(this, QStringLiteral("Таблица"), QStringLiteral("Столбцов:"), 3, 1, 12, 1, &ok);
    if (!ok)
        return;

    QTextTableFormat tf;
    tf.setBorder(1);
    tf.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
    tf.setCellPadding(4);
    tf.setCellSpacing(0);
    tf.setWidth(QTextLength(QTextLength::PercentageLength, 100));
    edit_->textCursor().insertTable(rows, cols, tf);
}

void MainWindow::showFindReplace()
{
    if (findDlg_) {
        findDlg_->raise();
        findDlg_->activateWindow();
        return;
    }

    auto* dlg = new QDialog(this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setWindowTitle(QStringLiteral("Найти и заменить"));

    auto* findEdit = new QLineEdit;
    auto* replEdit = new QLineEdit;
    auto* form = new QFormLayout;
    form->addRow(QStringLiteral("Найти:"), findEdit);
    form->addRow(QStringLiteral("Заменить на:"), replEdit);

    auto* nextBtn = new QPushButton(QStringLiteral("Найти далее"));
    auto* replBtn = new QPushButton(QStringLiteral("Заменить"));
    auto* allBtn = new QPushButton(QStringLiteral("Заменить все"));
    auto* buttons = new QHBoxLayout;
    buttons->addWidget(nextBtn);
    buttons->addWidget(replBtn);
    buttons->addWidget(allBtn);

    auto* layout = new QVBoxLayout(dlg);
    layout->addLayout(form);
    layout->addLayout(buttons);

    auto findNext = [this, findEdit]() -> bool {
        const QString t = findEdit->text();
        if (t.isEmpty())
            return false;
        if (edit_->find(t))
            return true;
        QTextCursor c = edit_->textCursor();
        c.movePosition(QTextCursor::Start);
        edit_->setTextCursor(c);
        return edit_->find(t);
    };

    connect(nextBtn, &QPushButton::clicked, dlg, [findNext] { findNext(); });
    connect(findEdit, &QLineEdit::returnPressed, dlg, [findNext] { findNext(); });
    connect(replBtn, &QPushButton::clicked, dlg, [this, findEdit, replEdit, findNext] {
        QTextCursor c = edit_->textCursor();
        if (c.hasSelection() && c.selectedText().compare(findEdit->text(), Qt::CaseInsensitive) == 0)
            c.insertText(replEdit->text());
        findNext();
    });
    connect(allBtn, &QPushButton::clicked, dlg, [this, findEdit, replEdit] {
        const QString t = findEdit->text();
        if (t.isEmpty())
            return;
        QTextDocument* d = edit_->document();
        QTextCursor group(d);
        group.beginEditBlock();
        int n = 0;
        int pos = 0;
        for (;;) {
            QTextCursor f = d->find(t, pos);
            if (f.isNull())
                break;
            f.insertText(replEdit->text());
            pos = f.position();
            ++n;
        }
        group.endEditBlock();
        statusBar()->showMessage(QStringLiteral("Заменено: %1").arg(n), 4000);
    });

    findDlg_ = dlg;
    dlg->show();
}

// ---------------------------------------------------------------- зум и масштабирование

void MainWindow::zoomIn()
{
    zoomTo(zoomLevel_ + 0.1);
}

void MainWindow::zoomOut()
{
    zoomTo(zoomLevel_ - 0.1);
}

void MainWindow::zoomTo(double factor)
{
    zoomLevel_ = qBound(0.3, factor, 3.0);
    
    // Apply zoom by scaling the font size
    QFont font = edit_->font();
    const int pointSize = font.pointSizeF() * zoomLevel_;
    font.setPointSizeF(static_cast<double>(pointSize));
    edit_->setFont(font);
    
    // Adjust width to keep document centered
    const int scaledWidth = static_cast<int>(794.0 * zoomLevel_);
    edit_->setFixedWidth(scaledWidth);
}

void MainWindow::autoFitDocumentWidth()
{
    // Fit document width to view (A4 width at 96 DPI is 794 pixels)
    const double docWidth = 794.0;
    const double viewWidth = static_cast<double>(edit_->viewport()->size().width());
    const double neededZoom = viewWidth / docWidth;
    zoomTo(neededZoom);
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

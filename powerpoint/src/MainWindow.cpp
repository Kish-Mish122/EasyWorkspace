#include "MainWindow.h"

#include "SlideItem.h"
#include "SlideScene.h"
#include "SlideShowWindow.h"

#include <Common.h>
#include <ThemeManager.h>

#include <QActionGroup>
#include <QCloseEvent>
#include <QColorDialog>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGraphicsView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLabel>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QPageSize>
#include <QPainter>
#include <QPdfWriter>
#include <QPixmap>
#include <QSaveFile>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QSplitter>
#include <QStatusBar>
#include <QToolBar>
#include <algorithm>

namespace {

const QString kAppTitle = QStringLiteral("EasySlides");

struct DeckTheme {
    QString name;
    QColor bg;
    QColor text;
    QColor accent;
};

const QVector<DeckTheme>& themes()
{
    static const QVector<DeckTheme> list = {
        {QStringLiteral("Светлая"), QColor(0xff, 0xff, 0xff), QColor(0x11, 0x18, 0x27), QColor(0x25, 0x63, 0xeb)},
        {QStringLiteral("Тёмная"), QColor(0x11, 0x18, 0x27), QColor(0xf9, 0xfa, 0xfb), QColor(0xf5, 0x9e, 0x0b)},
        {QStringLiteral("Океан"), QColor(0xe0, 0xf2, 0xfe), QColor(0x0c, 0x4a, 0x6e), QColor(0x02, 0x84, 0xc7)},
        {QStringLiteral("Закат"), QColor(0xff, 0xf7, 0xed), QColor(0x7c, 0x2d, 0x12), QColor(0xea, 0x58, 0x0c)},
    };
    return list;
}

// Вид, который всегда вписывает слайд целиком.
class FitView : public QGraphicsView {
public:
    using QGraphicsView::QGraphicsView;

protected:
    void resizeEvent(QResizeEvent* e) override
    {
        QGraphicsView::resizeEvent(e);
        if (scene())
            fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
    }
};

} // namespace

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    buildUi();
    resize(1200, 760);
    newDeck();
}

QString MainWindow::docName() const
{
    return path_.isEmpty() ? QStringLiteral("Презентация1") : QFileInfo(path_).fileName();
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
    scene_ = new SlideScene(this);
    QGraphicsView* view = new FitView(scene_);
    view->setRenderHint(QPainter::Antialiasing);
    view->setFrameShape(QFrame::NoFrame);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setAlignment(Qt::AlignCenter);

    list_ = new QListWidget;
    list_->setViewMode(QListView::IconMode);
    list_->setFlow(QListView::TopToBottom);
    list_->setWrapping(false);
    list_->setMovement(QListView::Static);
    list_->setResizeMode(QListView::Adjust);
    list_->setIconSize(QSize(192, 108));
    list_->setSpacing(8);
    list_->setMinimumWidth(180);

    auto* splitter = new QSplitter;
    splitter->addWidget(list_);
    splitter->addWidget(view);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes({230, 970});
    setCentralWidget(splitter);

    auto act = [this](const QString& text, const QKeySequence& shortcut, auto slot) {
        auto* a = new QAction(text, this);
        if (!shortcut.isEmpty())
            a->setShortcut(shortcut);
        connect(a, &QAction::triggered, this, slot);
        return a;
    };

    // ---- Файл ----
    auto* fileMenu = menuBar()->addMenu(QStringLiteral("&Файл"));
    auto* newAct = act(QStringLiteral("Создать"), QKeySequence::New, [this] {
        if (maybeSave())
            newDeck();
    });
    auto* openAct = act(QStringLiteral("Открыть…"), QKeySequence::Open, [this] {
        if (!maybeSave())
            return;
        const QString path = QFileDialog::getOpenFileName(
            this, QStringLiteral("Открыть"), {}, QStringLiteral("Презентации EasyPowerPoint (*.ezp);;Все файлы (*)"));
        if (!path.isEmpty())
            openFile(path);
    });
    auto* saveAct = act(QStringLiteral("Сохранить"), QKeySequence::Save, [this] { save(); });
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(act(QStringLiteral("Сохранить как…"), QKeySequence::SaveAs, [this] { saveAs(); }));
    fileMenu->addSeparator();
    fileMenu->addAction(act(QStringLiteral("Экспорт в PDF…"), {}, [this] { exportPdf(); }));
    fileMenu->addSeparator();
    fileMenu->addAction(act(QStringLiteral("Выход"), QKeySequence::Quit, [this] { close(); }));

    // ---- Слайды ----
    auto* slideMenu = menuBar()->addMenu(QStringLiteral("&Слайды"));
    auto* addTitled = act(QStringLiteral("Новый слайд"), QKeySequence(Qt::CTRL | Qt::Key_M), [this] { addSlide(true); });
    auto* addBlank = act(QStringLiteral("Пустой слайд"), {}, [this] { addSlide(false); });
    auto* dupAct = act(QStringLiteral("Дублировать слайд"), {}, [this] { duplicateSlide(); });
    auto* delSlide = act(QStringLiteral("Удалить слайд"), {}, [this] { deleteSlide(); });
    auto* upAct = act(QStringLiteral("Переместить вверх"), {}, [this] { moveSlide(-1); });
    auto* downAct = act(QStringLiteral("Переместить вниз"), {}, [this] { moveSlide(+1); });
    auto* showAct = act(QStringLiteral("Начать показ"), QKeySequence(Qt::Key_F5), [this] { startShow(); });
    slideMenu->addAction(addTitled);
    slideMenu->addAction(addBlank);
    slideMenu->addAction(dupAct);
    slideMenu->addAction(delSlide);
    slideMenu->addSeparator();
    slideMenu->addAction(upAct);
    slideMenu->addAction(downAct);
    slideMenu->addSeparator();
    slideMenu->addAction(showAct);

    // ---- Вставка ----
    auto* insMenu = menuBar()->addMenu(QStringLiteral("&Вставка"));
    auto* textAct = act(QStringLiteral("Текстовое поле"), {}, [this] { addItem(ItemData::Text); });
    auto* rectAct = act(QStringLiteral("Прямоугольник"), {}, [this] { addItem(ItemData::Rect); });
    auto* ellAct = act(QStringLiteral("Эллипс"), {}, [this] { addItem(ItemData::Ellipse); });
    insMenu->addAction(textAct);
    insMenu->addAction(rectAct);
    insMenu->addAction(ellAct);

    // ---- Формат ----
    auto* fmtMenu = menuBar()->addMenu(QStringLiteral("Фор&мат"));
    auto* fillAct = act(QStringLiteral("Заливка фигуры…"), {}, [this] { applyFill(); });
    auto* colorAct = act(QStringLiteral("Цвет текста…"), {}, [this] { applyTextColor(); });
    boldAct_ = new QAction(QStringLiteral("Ж"), this);
    boldAct_->setCheckable(true);
    boldAct_->setToolTip(QStringLiteral("Полужирный"));
    connect(boldAct_, &QAction::triggered, this, [this](bool on) { applyBold(on); });
    auto* frontAct = act(QStringLiteral("На передний план"), {}, [this] { bringToFront(); });
    auto* delItemAct = act(QStringLiteral("Удалить объект"), QKeySequence::Delete, [this] { deleteSelected(); });
    auto* bgAct = act(QStringLiteral("Фон слайда…"), {}, [this] { applyBackground(); });
    fmtMenu->addAction(fillAct);
    fmtMenu->addAction(colorAct);
    fmtMenu->addAction(boldAct_);
    fmtMenu->addSeparator();
    fmtMenu->addAction(frontAct);
    fmtMenu->addAction(delItemAct);
    fmtMenu->addSeparator();
    fmtMenu->addAction(bgAct);
    auto* themeMenu = fmtMenu->addMenu(QStringLiteral("Тема оформления"));
    for (int i = 0; i < themes().size(); ++i)
        themeMenu->addAction(act(themes()[i].name, {}, [this, i] { applyTheme(i); }));

    // ---- Вид ----
    auto* viewMenu = menuBar()->addMenu(QStringLiteral("&Вид"));
    auto* zoomInAct = act(QStringLiteral("Увеличить"), QKeySequence::ZoomIn, [this] { zoomIn(); });
    auto* zoomOutAct = act(QStringLiteral("Уменьшить"), QKeySequence::ZoomOut, [this] { zoomOut(); });
    auto* zoomTo100Act = act(QStringLiteral("100%"), {}, [this] { zoomTo(1.0); });
    auto* fitViewAct = act(QStringLiteral("По размеру окна"), {}, [this] { autoFitView(); });
    
    viewMenu->addAction(zoomInAct);
    viewMenu->addAction(zoomOutAct);
    viewMenu->addSeparator();
    viewMenu->addAction(zoomTo100Act);
    viewMenu->addAction(fitViewAct);

    // ---- Справка ----
    auto* helpMenu = menuBar()->addMenu(QStringLiteral("&Справка"));
    helpMenu->addAction(act(QStringLiteral("Подсказки"), {}, [this] {
        QMessageBox::information(
            this, kAppTitle,
            QStringLiteral("• Перетаскивайте объекты мышью.\n"
                           "• Размер меняется за синий квадрат в правом нижнем углу.\n"
                           "• Двойной щелчок по объекту — редактирование текста.\n"
                           "• F5 — показ; стрелки, пробел или клик — листать; Esc — выход."));
    }));
    helpMenu->addAction(act(QStringLiteral("О программе"), {}, [this] {
        QMessageBox::about(this, kAppTitle,
                           QStringLiteral("EasySlides 1.0 — презентации из пакета EasyWorkspace.\n"
                                          "Его формат: .ezp. (c) K1sh-M1sh"));
    }));

    // ---- Панель инструментов ----
    auto* tb = addToolBar(QStringLiteral("Главная"));
    tb->setMovable(false);
    tb->addAction(newAct);
    tb->addAction(openAct);
    tb->addAction(saveAct);
    tb->addSeparator();
    tb->addAction(addTitled);
    tb->addAction(textAct);
    tb->addAction(rectAct);
    tb->addAction(ellAct);
    tb->addSeparator();
    tb->addAction(fillAct);
    tb->addAction(colorAct);
    tb->addAction(boldAct_);

    sizeSpin_ = new QSpinBox;
    sizeSpin_->setRange(8, 200);
    sizeSpin_->setValue(28);
    sizeSpin_->setSuffix(QStringLiteral(" px"));
    sizeSpin_->setToolTip(QStringLiteral("Размер шрифта"));
    connect(sizeSpin_, &QSpinBox::valueChanged, this, [this](int v) { applyFontSize(v); });
    tb->addWidget(sizeSpin_);
    tb->addSeparator();
    tb->addAction(showAct);

    statusLabel_ = new QLabel;
    statusBar()->addPermanentWidget(statusLabel_);

    // ---- Связи ----
    thumbTimer_.setSingleShot(true);
    thumbTimer_.setInterval(300);
    connect(&thumbTimer_, &QTimer::timeout, this, [this] { updateThumb(); });
    connect(scene_, &SlideScene::edited, this, [this] { onEdited(); });
    connect(scene_, &QGraphicsScene::selectionChanged, this, [this] { onSelectionChanged(); });
    connect(list_, &QListWidget::currentRowChanged, this, [this](int row) { showSlide(row); });
}

// ---------------------------------------------------------------- модель колоды

Slide MainWindow::makeSlide(bool titled) const
{
    const DeckTheme& t = themes()[themeIndex_];
    Slide s;
    s.background = t.bg;
    if (titled) {
        ItemData title;
        title.type = ItemData::Text;
        title.rect = QRectF(60, 40, 840, 100);
        title.text = QStringLiteral("Заголовок слайда");
        title.fontSize = 44;
        title.bold = true;
        title.textColor = t.text;

        ItemData body;
        body.type = ItemData::Text;
        body.rect = QRectF(60, 170, 840, 320);
        body.text = QStringLiteral("Текст слайда");
        body.fontSize = 28;
        body.textColor = t.text;

        s.items.push_back(title);
        s.items.push_back(body);
    }
    return s;
}

void MainWindow::newDeck()
{
    themeIndex_ = 0;
    const DeckTheme& t = themes()[0];

    Slide first;
    first.background = t.bg;
    ItemData title;
    title.rect = QRectF(80, 170, 800, 130);
    title.text = QStringLiteral("Название презентации");
    title.fontSize = 54;
    title.bold = true;
    title.textColor = t.text;
    ItemData sub;
    sub.rect = QRectF(80, 320, 800, 70);
    sub.text = QStringLiteral("Подзаголовок");
    sub.fontSize = 28;
    sub.textColor = t.text;
    first.items = {title, sub};

    slides_.clear();
    slides_.push_back(first);
    current_ = 0;
    refreshList();
    loadCurrent();
    setModified(false);
    setPath({});
}

void MainWindow::commitCurrent()
{
    if (current_ >= 0 && current_ < slides_.size())
        slides_[current_] = scene_->collect();
}

void MainWindow::loadCurrent()
{
    scene_->load(slides_[current_]);
    updateStatus();
    onSelectionChanged();
}

void MainWindow::showSlide(int index)
{
    if (index < 0 || index >= slides_.size() || index == current_)
        return;
    commitCurrent();
    current_ = index;
    loadCurrent();
}

static QPixmap makeThumb(const Slide& slide)
{
    QPixmap px(192, 108);
    px.fill(Qt::white);
    QPainter p(&px);
    SlideScene::paintSlide(&p, slide, QRectF(0, 0, 192, 108));
    return px;
}

void MainWindow::refreshList()
{
    const QSignalBlocker blocker(list_);
    list_->clear();
    for (int i = 0; i < slides_.size(); ++i) {
        auto* item = new QListWidgetItem(QIcon(makeThumb(slides_[i])), QString::number(i + 1));
        item->setTextAlignment(Qt::AlignHCenter);
        list_->addItem(item);
    }
    list_->setCurrentRow(current_);
}

void MainWindow::updateThumb()
{
    commitCurrent();
    if (QListWidgetItem* item = list_->item(current_))
        item->setIcon(QIcon(makeThumb(slides_[current_])));
}

void MainWindow::updateStatus()
{
    statusLabel_->setText(QStringLiteral("Слайд %1 из %2   Тема: %3")
                              .arg(current_ + 1)
                              .arg(slides_.size())
                              .arg(themes()[themeIndex_].name));
}

void MainWindow::onEdited()
{
    setModified(true);
    thumbTimer_.start();
}

void MainWindow::addSlide(bool titled)
{
    commitCurrent();
    slides_.insert(current_ + 1, makeSlide(titled));
    ++current_;
    refreshList();
    loadCurrent();
    setModified(true);
}

void MainWindow::deleteSlide()
{
    if (slides_.size() <= 1) {
        QMessageBox::information(this, kAppTitle, QStringLiteral("Нельзя удалить единственный слайд."));
        return;
    }
    slides_.remove(current_);
    current_ = std::min(current_, int(slides_.size()) - 1);
    refreshList();
    loadCurrent();
    setModified(true);
}

void MainWindow::duplicateSlide()
{
    commitCurrent();
    const Slide copy = slides_[current_];
    slides_.insert(current_ + 1, copy);
    ++current_;
    refreshList();
    loadCurrent();
    setModified(true);
}

void MainWindow::moveSlide(int delta)
{
    commitCurrent();
    const int j = current_ + delta;
    if (j < 0 || j >= slides_.size())
        return;
    std::swap(slides_[current_], slides_[j]);
    current_ = j;
    refreshList();
    updateStatus();
    setModified(true);
}

// ---------------------------------------------------------------- объекты

void MainWindow::addItem(ItemData::Type type)
{
    const DeckTheme& t = themes()[themeIndex_];
    ItemData d;
    d.type = type;
    d.textColor = t.text;
    switch (type) {
    case ItemData::Text:
        d.rect = QRectF(280, 230, 400, 80);
        d.text = QStringLiteral("Новый текст");
        break;
    case ItemData::Rect:
    case ItemData::Ellipse:
        d.rect = QRectF(330, 180, 300, 180);
        d.fill = t.accent;
        d.textColor = Qt::white;
        d.fontSize = 24;
        break;
    }
    scene_->clearSelection();
    SlideItem* item = scene_->addSlideItem(d);
    item->setSelected(true);
    onEdited();
}

void MainWindow::deleteSelected()
{
    const QList<SlideItem*> sel = scene_->selectedSlideItems();
    if (sel.isEmpty())
        return;
    for (SlideItem* it : sel) {
        scene_->removeItem(it);
        delete it;
    }
    onEdited();
}

void MainWindow::bringToFront()
{
    for (SlideItem* it : scene_->selectedSlideItems())
        scene_->bringToFront(it);
}

void MainWindow::applyFill()
{
    const QList<SlideItem*> sel = scene_->selectedSlideItems();
    if (sel.isEmpty())
        return;
    const QColor c = QColorDialog::getColor(sel.first()->data().fill, this, QStringLiteral("Заливка"),
                                            QColorDialog::ShowAlphaChannel);
    if (!c.isValid())
        return;
    for (SlideItem* it : sel) {
        ItemData d = it->data();
        d.fill = c;
        it->setItemData(d);
    }
}

void MainWindow::applyTextColor()
{
    const QList<SlideItem*> sel = scene_->selectedSlideItems();
    if (sel.isEmpty())
        return;
    const QColor c = QColorDialog::getColor(sel.first()->data().textColor, this, QStringLiteral("Цвет текста"));
    if (!c.isValid())
        return;
    for (SlideItem* it : sel) {
        ItemData d = it->data();
        d.textColor = c;
        it->setItemData(d);
    }
}

void MainWindow::applyBold(bool on)
{
    for (SlideItem* it : scene_->selectedSlideItems()) {
        ItemData d = it->data();
        d.bold = on;
        it->setItemData(d);
    }
}

void MainWindow::applyFontSize(int size)
{
    for (SlideItem* it : scene_->selectedSlideItems()) {
        ItemData d = it->data();
        d.fontSize = size;
        it->setItemData(d);
    }
}

void MainWindow::applyBackground()
{
    const QColor c = QColorDialog::getColor(scene_->background(), this, QStringLiteral("Фон слайда"));
    if (c.isValid())
        scene_->setBackground(c);
}

void MainWindow::applyTheme(int index)
{
    commitCurrent();
    themeIndex_ = index;
    const DeckTheme& t = themes()[index];
    for (Slide& s : slides_) {
        s.background = t.bg;
        for (ItemData& d : s.items) {
            if (d.type == ItemData::Text) {
                d.textColor = t.text;
            } else {
                d.fill = t.accent;
                d.textColor = Qt::white;
            }
        }
    }
    refreshList();
    loadCurrent();
    setModified(true);
}

void MainWindow::onSelectionChanged()
{
    const QList<SlideItem*> sel = scene_->selectedSlideItems();
    if (sel.isEmpty())
        return;
    const ItemData d = sel.first()->data();
    {
        const QSignalBlocker b(sizeSpin_);
        sizeSpin_->setValue(d.fontSize);
    }
    boldAct_->setChecked(d.bold);
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
    if (maybeSave())
        event->accept();
    else
        event->ignore();
}

bool MainWindow::writeFile(const QString& path)
{
    commitCurrent();
    QJsonArray arr;
    for (const Slide& s : std::as_const(slides_))
        arr.append(s.toJson());

    QJsonObject root;
    root.insert(QStringLiteral("format"), QStringLiteral("ezp"));
    root.insert(QStringLiteral("version"), 1);
    root.insert(QStringLiteral("theme"), themeIndex_);
    root.insert(QStringLiteral("slides"), arr);

    QSaveFile f(path);
    if (!f.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, kAppTitle, QStringLiteral("Не удалось сохранить файл:\n%1").arg(path));
        return false;
    }
    f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    if (!f.commit()) {
        QMessageBox::warning(this, kAppTitle, QStringLiteral("Ошибка записи файла."));
        return false;
    }
    return true;
}

bool MainWindow::save()
{
    if (path_.isEmpty())
        return saveAs();
    if (!writeFile(path_))
        return false;
    setModified(false);
    return true;
}

bool MainWindow::saveAs()
{
    QString path = QFileDialog::getSaveFileName(this, QStringLiteral("Сохранить как"),
                                                path_.isEmpty() ? docName() : path_,
                                                QStringLiteral("Презентация EasyPowerPoint (*.ezp)"));
    if (path.isEmpty())
        return false;
    if (QFileInfo(path).suffix().isEmpty())
        path += QStringLiteral(".ezp");
    if (!writeFile(path))
        return false;
    setModified(false);
    setPath(path);
    return true;
}

bool MainWindow::openFile(const QString& path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, kAppTitle, QStringLiteral("Не удалось открыть файл:\n%1").arg(path));
        return false;
    }
    QJsonParseError err;
    const QJsonDocument jd = QJsonDocument::fromJson(f.readAll(), &err);
    if (err.error != QJsonParseError::NoError || !jd.isObject()
        || jd.object().value(QStringLiteral("format")).toString() != QLatin1String("ezp")) {
        QMessageBox::warning(this, kAppTitle, QStringLiteral("Файл повреждён или имеет неверный формат."));
        return false;
    }

    QVector<Slide> loaded;
    const QJsonArray arr = jd.object().value(QStringLiteral("slides")).toArray();
    for (const QJsonValue& v : arr)
        loaded.push_back(Slide::fromJson(v.toObject()));
    if (loaded.isEmpty())
        loaded.push_back(makeSlide(true));

    slides_ = loaded;
    themeIndex_ = std::clamp(jd.object().value(QStringLiteral("theme")).toInt(), 0, int(themes().size()) - 1);
    current_ = 0;
    refreshList();
    loadCurrent();
    setModified(false);
    setPath(path);
    return true;
}

void MainWindow::exportPdf()
{
    QString path = QFileDialog::getSaveFileName(this, QStringLiteral("Экспорт в PDF"), {}, QStringLiteral("PDF (*.pdf)"));
    if (path.isEmpty())
        return;
    if (QFileInfo(path).suffix().isEmpty())
        path += QStringLiteral(".pdf");

    commitCurrent();
    QPdfWriter writer(path);
    writer.setResolution(96);
    writer.setPageSize(QPageSize(QSizeF(960 * 25.4 / 96, 540 * 25.4 / 96), QPageSize::Millimeter,
                                 QString(), QPageSize::ExactMatch));
    writer.setPageMargins(QMarginsF(0, 0, 0, 0), QPageLayout::Millimeter);

    QPainter painter(&writer);
    for (int i = 0; i < slides_.size(); ++i) {
        if (i > 0)
            writer.newPage();
        SlideScene::paintSlide(&painter, slides_[i], QRectF(0, 0, writer.width(), writer.height()));
    }
    painter.end();
    statusBar()->showMessage(QStringLiteral("PDF сохранён: %1").arg(path), 5000);
}

void MainWindow::startShow()
{
    commitCurrent();
    auto* show = new SlideShowWindow(slides_, current_);
    show->setAttribute(Qt::WA_DeleteOnClose);
    show->showFullScreen();
    show->setFocus();
}

// ---------------------------------------------------------------- зум и масштабирование

void MainWindow::zoomIn()
{
    zoomLevel_ = qMin(zoomLevel_ + 0.1, 3.0);
    view_->resetTransform();
    view_->scale(zoomLevel_, zoomLevel_);
    if (zoomLevel_ <= 1.0)
        view_->fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
}

void MainWindow::zoomOut()
{
    zoomLevel_ = qMax(zoomLevel_ - 0.1, 0.3);
    view_->resetTransform();
    view_->scale(zoomLevel_, zoomLevel_);
    if (zoomLevel_ <= 1.0)
        view_->fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
}

void MainWindow::zoomTo(double factor)
{
    zoomLevel_ = qBound(0.3, factor, 3.0);
    view_->resetTransform();
    view_->scale(zoomLevel_, zoomLevel_);
    if (zoomLevel_ <= 1.0)
        view_->fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
}

void MainWindow::autoFitView()
{
    zoomLevel_ = 1.0;
    view_->resetTransform();
    view_->fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
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

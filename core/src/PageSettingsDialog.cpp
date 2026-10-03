#include "PageSettingsDialog.h"

#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QRadioButton>
#include <QButtonGroup>

namespace easy {

PageSettingsDialog::PageSettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Настройки страницы"));
    setFixedSize(400, 320);

    auto* mainLayout = new QHBoxLayout(this);

    // Left: Page setup
    auto* pageGroup = new QGroupBox(QStringLiteral("Страница"), this);
    auto* pageForm = new QFormLayout(pageGroup);

    sizeCombo_ = new QComboBox(this);
    sizeCombo_->addItems({
        QStringLiteral("A4 (210 × 297 мм)"),
        QStringLiteral("A3 (297 × 420 мм)"),
        QStringLiteral("A5 (148 × 210 мм)"),
        QStringLiteral("Letter (216 × 279 мм)"),
        QStringLiteral("Legal (216 × 356 мм)")
    });
    sizeCombo_->setCurrentIndex(0); // A4
    pageForm->addRow(QStringLiteral("Размер:"), sizeCombo_);

    auto* orientGroup = new QGroupBox(QStringLiteral("Ориентация"), this);
    auto* orientLayout = new QHBoxLayout(orientGroup);
    portrait_ = new QRadioButton(QStringLiteral("Книжная"), this);
    landscape_ = new QRadioButton(QStringLiteral("Альбомная"), this);
    portrait_->setChecked(true);
    orientLayout->addWidget(portrait_);
    orientLayout->addWidget(landscape_);

    // Right: Margins
    auto* marginGroup = new QGroupBox(QStringLiteral("Поля (мм)"), this);
    auto* marginForm = new QFormLayout(marginGroup);

    marginTop_ = new QSpinBox(this);
    marginTop_->setRange(5, 50);
    marginTop_->setValue(20);
    marginBottom_ = new QSpinBox(this);
    marginBottom_->setRange(5, 50);
    marginBottom_->setValue(20);
    marginLeft_ = new QSpinBox(this);
    marginLeft_->setRange(5, 50);
    marginLeft_->setValue(20);
    marginRight_ = new QSpinBox(this);
    marginRight_->setRange(5, 50);
    marginRight_->setValue(20);

    marginForm->addRow(QStringLiteral("Верх:"), marginTop_);
    marginForm->addRow(QStringLiteral("Низ:"), marginBottom_);
    marginForm->addRow(QStringLiteral("Лево:"), marginLeft_);
    marginForm->addRow(QStringLiteral("Право:"), marginRight_);

    // Buttons
    auto* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    mainLayout->addWidget(pageGroup);
    mainLayout->addWidget(orientGroup);
    mainLayout->addWidget(marginGroup);
    mainLayout->addWidget(buttonBox);
}

QPageSize PageSettingsDialog::pageSize() const
{
    int idx = sizeCombo_->currentIndex();
    switch (idx) {
    case 0: return QPageSize(QPageSize::A4);
    case 1: return QPageSize(QPageSize::A3);
    case 2: return QPageSize(QPageSize::A5);
    case 3: return QPageSize(QPageSize::Letter);
    case 4: return QPageSize(QPageSize::Legal);
    default: return QPageSize(QPageSize::A4);
    }
}

QMargins PageSettingsDialog::margins() const
{
    return QMargins(
        marginLeft_->value(),
        marginTop_->value(),
        marginRight_->value(),
        marginBottom_->value()
    );
}

int PageSettingsDialog::orientation() const
{
    return landscape_->isChecked() ? 1 : 0; // 0=portrait, 1=landscape
}

} // namespace easy

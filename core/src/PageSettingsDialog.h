#pragma once

#include <QDialog>
#include <QMargins>
#include <QPageSize>

class QComboBox;
class QSpinBox;
class QRadioButton;

namespace easy {

class PageSettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit PageSettingsDialog(QWidget* parent = nullptr);

    QPageSize pageSize() const;
    QMargins margins() const;
    int orientation() const; // 0=portrait, 1=landscape

private:
    QComboBox* sizeCombo_ = nullptr;
    QSpinBox* marginTop_ = nullptr;
    QSpinBox* marginBottom_ = nullptr;
    QSpinBox* marginLeft_ = nullptr;
    QSpinBox* marginRight_ = nullptr;
    QRadioButton* portrait_ = nullptr;
    QRadioButton* landscape_ = nullptr;

    void createControls();
};

} // namespace easy

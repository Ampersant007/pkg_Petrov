#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), m_updating(false) {
    m_colorModel = new ColorModel(this);

    QWidget *centralWidget = new QWidget(this);
    QGridLayout *layout = new QGridLayout(centralWidget);

    QGroupBox *previewGroup = createPreviewGroup();
    QGroupBox *rgbGroup = createRgbGroup();
    QGroupBox *hsvGroup = createHsvGroup();
    QGroupBox *xyzGroup = createXyzGroup();

    layout->addWidget(previewGroup, 0, 0, 3, 1);
    layout->addWidget(rgbGroup, 0, 1, 1, 2);
    layout->addWidget(hsvGroup, 1, 1, 1, 2);
    layout->addWidget(xyzGroup, 2, 1, 1, 2);

    setCentralWidget(centralWidget);

    m_statusBar = new QStatusBar(this);
    setStatusBar(m_statusBar);

    connect(m_colorModel, SIGNAL(rgbChanged(const RGB&)), this, SLOT(onRgbChanged(const RGB&)));
    connect(m_colorModel, SIGNAL(hsvChanged(const HSV&)), this, SLOT(onHsvChanged(const HSV&)));
    connect(m_colorModel, SIGNAL(xyzChanged(const XYZ&)), this, SLOT(onXyzChanged(const XYZ&)));
    connect(m_colorModel, SIGNAL(colorChanged(const QColor&)), this, SLOT(onColorChanged(const QColor&)));
    connect(m_colorModel, SIGNAL(validationError(const QString&)), this, SLOT(onValidationError(const QString&)));
    connect(m_colorModel, SIGNAL(roundingNotification(const QString&)), this, SLOT(onRoundingNotification(const QString&)));

    // Connect RGB
    connect(m_rSpin, SIGNAL(valueChanged(int)), this, SLOT(onRgbComponentChanged()));
    connect(m_gSpin, SIGNAL(valueChanged(int)), this, SLOT(onRgbComponentChanged()));
    connect(m_bSpin, SIGNAL(valueChanged(int)), this, SLOT(onRgbComponentChanged()));
    connect(m_rSlider, SIGNAL(valueChanged(int)), this, SLOT(onRgbComponentChanged()));
    connect(m_gSlider, SIGNAL(valueChanged(int)), this, SLOT(onRgbComponentChanged()));
    connect(m_bSlider, SIGNAL(valueChanged(int)), this, SLOT(onRgbComponentChanged()));

    // Connect HSV
    connect(m_hSpin, SIGNAL(valueChanged(double)), this, SLOT(onHsvComponentChanged()));
    connect(m_sSpin, SIGNAL(valueChanged(double)), this, SLOT(onHsvComponentChanged()));
    connect(m_vSpin, SIGNAL(valueChanged(double)), this, SLOT(onHsvComponentChanged()));
    connect(m_hSlider, SIGNAL(valueChanged(int)), this, SLOT(onHsvComponentChanged()));
    connect(m_sSlider, SIGNAL(valueChanged(int)), this, SLOT(onHsvComponentChanged()));
    connect(m_vSlider, SIGNAL(valueChanged(int)), this, SLOT(onHsvComponentChanged()));

    // Connect XYZ
    connect(m_xSpin, SIGNAL(valueChanged(double)), this, SLOT(onXyzComponentChanged()));
    connect(m_ySpin, SIGNAL(valueChanged(double)), this, SLOT(onXyzComponentChanged()));
    connect(m_zSpin, SIGNAL(valueChanged(double)), this, SLOT(onXyzComponentChanged()));
    connect(m_xSlider, SIGNAL(valueChanged(int)), this, SLOT(onXyzComponentChanged()));
    connect(m_ySlider, SIGNAL(valueChanged(int)), this, SLOT(onXyzComponentChanged()));
    connect(m_zSlider, SIGNAL(valueChanged(int)), this, SLOT(onXyzComponentChanged()));

    // Connect color choose
    connect(m_colorChooseButton, SIGNAL(clicked()), this, SLOT(onColorChooseClicked()));

    m_colorModel->setColor(Qt::white);

    setWindowTitle("Color Converter");
    resize(600, 400);
}

MainWindow::~MainWindow() {   }

QGroupBox* MainWindow::createRgbGroup()
{
    QGroupBox *group = new QGroupBox("RGB", this);
    QFormLayout *layout = new QFormLayout(group);

    // Red
    m_rSpin = new QSpinBox;
    m_rSpin->setRange(0, 255);
    m_rSlider = new QSlider(Qt::Horizontal);
    m_rSlider->setRange(0, 255);

    QHBoxLayout *rLayout = new QHBoxLayout;
    rLayout->addWidget(m_rSpin);
    rLayout->addWidget(m_rSlider);

    // Green
    m_gSpin = new QSpinBox;
    m_gSpin->setRange(0, 255);
    m_gSlider = new QSlider(Qt::Horizontal);
    m_gSlider->setRange(0, 255);

    QHBoxLayout *gLayout = new QHBoxLayout;
    gLayout->addWidget(m_gSpin);
    gLayout->addWidget(m_gSlider);

    // Blue
    m_bSpin = new QSpinBox;
    m_bSpin->setRange(0, 255);
    m_bSlider = new QSlider(Qt::Horizontal);
    m_bSlider->setRange(0, 255);

    QHBoxLayout *bLayout = new QHBoxLayout;
    bLayout->addWidget(m_bSpin);
    bLayout->addWidget(m_bSlider);

    layout->addRow("Red:", rLayout);
    layout->addRow("Green:", gLayout);
    layout->addRow("Blue:", bLayout);

    group->setLayout(layout);
    group->setMinimumWidth(300);
    return group;
}

QGroupBox* MainWindow::createHsvGroup()
{
    QGroupBox *group = new QGroupBox("HSV", this);
    QFormLayout *layout = new QFormLayout(group);

    // Hue
    m_hSpin = new QDoubleSpinBox;
    m_hSpin->setRange(0, 360);
    m_hSpin->setDecimals(1);
    m_hSlider = new QSlider(Qt::Horizontal);
    m_hSlider->setRange(0, 360);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(m_hSpin);
    hLayout->addWidget(m_hSlider);

    // Saturation
    m_sSpin = new QDoubleSpinBox;
    m_sSpin->setRange(0, 100);
    m_sSpin->setDecimals(1);
    m_sSlider = new QSlider(Qt::Horizontal);
    m_sSlider->setRange(0, 100);

    QHBoxLayout *sLayout = new QHBoxLayout;
    sLayout->addWidget(m_sSpin);
    sLayout->addWidget(m_sSlider);

    // Value
    m_vSpin = new QDoubleSpinBox;
    m_vSpin->setRange(0, 100);
    m_vSpin->setDecimals(1);
    m_vSlider = new QSlider(Qt::Horizontal);
    m_vSlider->setRange(0, 100);

    QHBoxLayout *vLayout = new QHBoxLayout;
    vLayout->addWidget(m_vSpin);
    vLayout->addWidget(m_vSlider);

    layout->addRow("Hue:", hLayout);
    layout->addRow("Saturation:", sLayout);
    layout->addRow("Value:", vLayout);

    group->setLayout(layout);
    group->setMinimumWidth(300);
    return group;
}

QGroupBox* MainWindow::createXyzGroup()
{
    QGroupBox *group = new QGroupBox("XYZ", this);
    QFormLayout *layout = new QFormLayout(group);

    // X
    m_xSpin = new QDoubleSpinBox;
    m_xSpin->setRange(0, 95.05);
    m_xSpin->setDecimals(3);
    m_xSlider = new QSlider(Qt::Horizontal);
    m_xSlider->setRange(0, 95);

    QHBoxLayout *xLayout = new QHBoxLayout;
    xLayout->addWidget(m_xSpin);
    xLayout->addWidget(m_xSlider);

    // Y
    m_ySpin = new QDoubleSpinBox;
    m_ySpin->setRange(0, 100);
    m_ySpin->setDecimals(3);
    m_ySlider = new QSlider(Qt::Horizontal);
    m_ySlider->setRange(0, 100);

    QHBoxLayout *yLayout = new QHBoxLayout;
    yLayout->addWidget(m_ySpin);
    yLayout->addWidget(m_ySlider);

    // Z
    m_zSpin = new QDoubleSpinBox;
    m_zSpin->setRange(0, 108.9);
    m_zSpin->setDecimals(3);
    m_zSlider = new QSlider(Qt::Horizontal);
    m_zSlider->setRange(0, 108);

    QHBoxLayout *zLayout = new QHBoxLayout;
    zLayout->addWidget(m_zSpin);
    zLayout->addWidget(m_zSlider);

    layout->addRow("X:", xLayout);
    layout->addRow("Y:", yLayout);
    layout->addRow("Z:", zLayout);

    group->setLayout(layout);
    group->setMinimumWidth(300);
    return group;
}

QGroupBox* MainWindow::createPreviewGroup()
{
    QGroupBox *group = new QGroupBox("Color", this);
    QVBoxLayout *layout = new QVBoxLayout(group);

    m_colorPreview = new QLabel;
    m_colorPreview->setMinimumHeight(300);
    m_colorPreview->setFrameStyle(QFrame::Box);
    m_colorPreview->setAutoFillBackground(true);

    m_colorChooseButton = new QPushButton("Choose Color");

    m_colorPreview->setMinimumWidth(300);
    layout->addWidget(m_colorPreview);
    layout->addWidget(m_colorChooseButton);

    group->setLayout(layout);
    return group;
}

void MainWindow::onRgbChanged(const RGB& rgb)
{
    blockSignals(true);

    m_rSpin->setValue(rgb.r);
    m_rSlider->setValue(rgb.r);
    m_gSpin->setValue(rgb.g);
    m_gSlider->setValue(rgb.g);
    m_bSpin->setValue(rgb.b);
    m_bSlider->setValue(rgb.b);

    blockSignals(false);
}

void MainWindow::onHsvChanged(const HSV& hsv)
{
    blockSignals(true);

    m_hSpin->setValue(hsv.h);
    m_hSlider->setValue(static_cast<int>(hsv.h));
    m_sSpin->setValue(hsv.s);
    m_sSlider->setValue(static_cast<int>(hsv.s));
    m_vSpin->setValue(hsv.v);
    m_vSlider->setValue(static_cast<int>(hsv.v));

    blockSignals(false);
}

void MainWindow::onXyzChanged(const XYZ& xyz)
{
    blockSignals(true);

    m_xSpin->setValue(xyz.x);
    m_xSlider->setValue(static_cast<int>(xyz.x));
    m_ySpin->setValue(xyz.y);
    m_ySlider->setValue(static_cast<int>(xyz.y));
    m_zSpin->setValue(xyz.z);
    m_zSlider->setValue(static_cast<int>(xyz.z));

    blockSignals(false);
}

void MainWindow::onColorChanged(const QColor &color)
{
    QPalette palette = m_colorPreview->palette();
    palette.setColor(QPalette::Window, color);
    m_colorPreview->setPalette(palette);
}

void MainWindow::onValidationError(const QString &message)
{
    m_statusBar->showMessage(message, 3000);
}

void MainWindow::onRgbComponentChanged()
{
    if (m_updating) return;

    QObject *sender = QObject::sender();
    if (sender == m_rSpin) {
        m_rSlider->setValue(m_rSpin->value());
    } else if (sender == m_rSlider) {
        m_rSpin->setValue(m_rSlider->value());
    } else if (sender == m_gSpin) {
        m_gSlider->setValue(m_gSpin->value());
    } else if (sender == m_gSlider) {
        m_gSpin->setValue(m_gSlider->value());
    } else if (sender == m_bSpin) {
        m_bSlider->setValue(m_bSpin->value());
    } else if (sender == m_bSlider) {
        m_bSpin->setValue(m_bSlider->value());
    }

    m_colorModel->setRgb(RGB(m_rSpin->value(), m_gSpin->value(), m_bSpin->value()));
}

void MainWindow::onHsvComponentChanged()
{
    if (m_updating) return;

    QObject *sender = QObject::sender();
    if (sender == m_hSpin) {
        m_hSlider->setValue(static_cast<int>(m_hSpin->value()));
    } else if (sender == m_hSlider) {
        m_hSpin->setValue(m_hSlider->value());
    } else if (sender == m_sSpin) {
        m_sSlider->setValue(static_cast<int>(m_sSpin->value()));
    } else if (sender == m_sSlider) {
        m_sSpin->setValue(m_sSlider->value());
    } else if (sender == m_vSpin) {
        m_vSlider->setValue(static_cast<int>(m_vSpin->value()));
    } else if (sender == m_vSlider) {
        m_vSpin->setValue(m_vSlider->value());
    }

    m_colorModel->setHsv(HSV(m_hSpin->value(), m_sSpin->value(), m_vSpin->value()));
}

void MainWindow::onXyzComponentChanged()
{
    if (m_updating) return;

    QObject *sender = QObject::sender();
    if (sender == m_xSpin) {
        m_xSlider->setValue(static_cast<int>(m_xSpin->value()));
    } else if (sender == m_xSlider) {
        m_xSpin->setValue(m_xSlider->value());
    } else if (sender == m_ySpin) {
        m_ySlider->setValue(static_cast<int>(m_ySpin->value()));
    } else if (sender == m_ySlider) {
        m_ySpin->setValue(m_ySlider->value());
    } else if (sender == m_zSpin) {
        m_zSlider->setValue(static_cast<int>(m_zSpin->value()));
    } else if (sender == m_zSlider) {
        m_zSpin->setValue(m_zSlider->value());
    }

    m_colorModel->setXyz(XYZ(m_xSpin->value(), m_ySpin->value(), m_zSpin->value()));
}

void MainWindow::onColorChooseClicked()
{
    QColor color = QColorDialog::getColor(m_colorModel->color(), this, "Choose Color");
    if (color.isValid()) {
        m_colorModel->setColor(color);
    }
}

void MainWindow::blockSignals(bool block)
{
    m_updating = block;

    m_rSpin->blockSignals(block);
    m_gSpin->blockSignals(block);
    m_bSpin->blockSignals(block);
    m_rSlider->blockSignals(block);
    m_gSlider->blockSignals(block);
    m_bSlider->blockSignals(block);

    m_hSpin->blockSignals(block);
    m_sSpin->blockSignals(block);
    m_vSpin->blockSignals(block);
    m_hSlider->blockSignals(block);
    m_sSlider->blockSignals(block);
    m_vSlider->blockSignals(block);

    m_xSpin->blockSignals(block);
    m_ySpin->blockSignals(block);
    m_zSpin->blockSignals(block);
    m_xSlider->blockSignals(block);
    m_ySlider->blockSignals(block);
    m_zSlider->blockSignals(block);
}

void MainWindow::onRoundingNotification(const QString& message){
    m_statusBar->showMessage(message, 1000);
}

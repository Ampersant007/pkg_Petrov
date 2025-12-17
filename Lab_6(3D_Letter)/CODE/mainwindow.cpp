#include "mainwindow.h"
#include "transformations.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QRadioButton>
#include <QCheckBox>
#include <QTextEdit>
#include <QLabel>
#include <QButtonGroup>
#include <QComboBox>
#include <QStatusBar>
#include <QLineEdit>
#include <QFormLayout>
#include <QMessageBox>
#include <QGridLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_rotationAngle(0)
    , m_scaleX(1.0), m_scaleY(1.0), m_scaleZ(1.0)
    , m_translationX(0), m_translationY(0), m_translationZ(0)
    , m_axisPoint(0, 0, 0)
    , m_axisDirection(0, 0, 1) {

    setWindowTitle("3D Буква П");
    resize(1400, 800);

    createUI();
    setupConnections();

    updateTransformMatrix();
    updateMatrixDisplay();
}

MainWindow::~MainWindow() {
}

void MainWindow::createUI() {
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);

    QWidget *controlPanel = new QWidget();
    controlPanel->setMinimumWidth(350);
    controlPanel->setMaximumWidth(400);
    QVBoxLayout *controlLayout = new QVBoxLayout(controlPanel);

    QGroupBox *axisGroup = new QGroupBox("Ось вращения");
    QGridLayout *axisGrid = new QGridLayout(axisGroup);

    axisGrid->addWidget(new QLabel("Точка:"), 0, 0);
    m_axisPointXSpinBox = new QDoubleSpinBox();
    m_axisPointYSpinBox = new QDoubleSpinBox();
    m_axisPointZSpinBox = new QDoubleSpinBox();

    for (auto spinBox : {m_axisPointXSpinBox, m_axisPointYSpinBox, m_axisPointZSpinBox}) {
        spinBox->setRange(-10.0, 10.0);
        spinBox->setSingleStep(0.5);
        spinBox->setDecimals(1);
        spinBox->setMaximumWidth(60);
    }

    m_axisPointXSpinBox->setValue(0);
    m_axisPointYSpinBox->setValue(0);
    m_axisPointZSpinBox->setValue(0);

    QHBoxLayout *pointLayout = new QHBoxLayout();
    pointLayout->addWidget(new QLabel("X:"));
    pointLayout->addWidget(m_axisPointXSpinBox);
    pointLayout->addWidget(new QLabel("Y:"));
    pointLayout->addWidget(m_axisPointYSpinBox);
    pointLayout->addWidget(new QLabel("Z:"));
    pointLayout->addWidget(m_axisPointZSpinBox);
    axisGrid->addLayout(pointLayout, 0, 1);

    axisGrid->addWidget(new QLabel("Вектор:"), 1, 0);
    m_axisDirectionXSpinBox = new QDoubleSpinBox();
    m_axisDirectionYSpinBox = new QDoubleSpinBox();
    m_axisDirectionZSpinBox = new QDoubleSpinBox();

    for (auto spinBox : {m_axisDirectionXSpinBox, m_axisDirectionYSpinBox, m_axisDirectionZSpinBox}) {
        spinBox->setRange(-10.0, 10.0);
        spinBox->setSingleStep(0.5);
        spinBox->setDecimals(1);
        spinBox->setMaximumWidth(60);
    }

    m_axisDirectionXSpinBox->setValue(0);
    m_axisDirectionYSpinBox->setValue(0);
    m_axisDirectionZSpinBox->setValue(1);

    QHBoxLayout *directionLayout = new QHBoxLayout();
    directionLayout->addWidget(new QLabel("X:"));
    directionLayout->addWidget(m_axisDirectionXSpinBox);
    directionLayout->addWidget(new QLabel("Y:"));
    directionLayout->addWidget(m_axisDirectionYSpinBox);
    directionLayout->addWidget(new QLabel("Z:"));
    directionLayout->addWidget(m_axisDirectionZSpinBox);
    axisGrid->addLayout(directionLayout, 1, 1);

    QHBoxLayout *axisButtonsLayout = new QHBoxLayout();
    m_setAxisButton = new QPushButton("Уст. ось");
    m_setAxisButton->setMaximumWidth(80);

    m_resetAxisButton = new QPushButton("Сброс");
    m_resetAxisButton->setMaximumWidth(80);

    axisButtonsLayout->addWidget(m_setAxisButton);
    axisButtonsLayout->addWidget(m_resetAxisButton);
    axisButtonsLayout->addStretch();
    axisGrid->addLayout(axisButtonsLayout, 2, 0, 1, 2);

    QGroupBox *rotationGroup = new QGroupBox("Вращение");
    QVBoxLayout *rotationLayout = new QVBoxLayout(rotationGroup);

    QHBoxLayout *angleLayout = new QHBoxLayout();
    angleLayout->addWidget(new QLabel("Угол:"));
    m_rotationAngleLabel = new QLabel("0°");
    m_rotationAngleLabel->setMinimumWidth(40);
    m_rotationAngleSlider = new QSlider(Qt::Horizontal);

    angleLayout->addWidget(m_rotationAngleLabel);
    angleLayout->addWidget(m_rotationAngleSlider);

    m_resetRotationButton = new QPushButton("Сброс");
    m_resetRotationButton->setMaximumWidth(80);

    QHBoxLayout *rotationButtonsLayout = new QHBoxLayout();
    rotationButtonsLayout->addWidget(m_resetRotationButton);
    rotationButtonsLayout->addStretch();

    rotationLayout->addLayout(angleLayout);
    rotationLayout->addLayout(rotationButtonsLayout);

    QGroupBox *scaleGroup = new QGroupBox("Масштаб");
    QHBoxLayout *scaleLayout = new QHBoxLayout(scaleGroup);

    m_scaleXSpinBox = new QDoubleSpinBox();
    m_scaleYSpinBox = new QDoubleSpinBox();
    m_scaleZSpinBox = new QDoubleSpinBox();

    for (auto spinBox : {m_scaleXSpinBox, m_scaleYSpinBox, m_scaleZSpinBox}) {
        spinBox->setRange(0.1, 5.0);
        spinBox->setSingleStep(0.5);
        spinBox->setDecimals(1);
        spinBox->setMaximumWidth(60);
        spinBox->setValue(1.0);
    }

    scaleLayout->addWidget(new QLabel("X:"));
    scaleLayout->addWidget(m_scaleXSpinBox);
    scaleLayout->addWidget(new QLabel("Y:"));
    scaleLayout->addWidget(m_scaleYSpinBox);
    scaleLayout->addWidget(new QLabel("Z:"));
    scaleLayout->addWidget(m_scaleZSpinBox);
    scaleLayout->addStretch();

    QGroupBox *translationGroup = new QGroupBox("Перенос");
    QHBoxLayout *translationLayout = new QHBoxLayout(translationGroup);

    m_translationXSpinBox = new QDoubleSpinBox();
    m_translationYSpinBox = new QDoubleSpinBox();
    m_translationZSpinBox = new QDoubleSpinBox();

    for (auto spinBox : {m_translationXSpinBox, m_translationYSpinBox, m_translationZSpinBox}) {
        spinBox->setRange(-5.0, 5.0);
        spinBox->setSingleStep(0.5);
        spinBox->setDecimals(1);
        spinBox->setMaximumWidth(60);
        spinBox->setValue(0.0);
    }

    translationLayout->addWidget(new QLabel("X:"));
    translationLayout->addWidget(m_translationXSpinBox);
    translationLayout->addWidget(new QLabel("Y:"));
    translationLayout->addWidget(m_translationYSpinBox);
    translationLayout->addWidget(new QLabel("Z:"));
    translationLayout->addWidget(m_translationZSpinBox);
    translationLayout->addStretch();

    m_resetTransformButton = new QPushButton("Сбросить всё");

    QGroupBox *projectionGroup = new QGroupBox("Проекция");
    QVBoxLayout *projectionLayout = new QVBoxLayout(projectionGroup);

    m_perspectiveRadio = new QRadioButton("Перспективная");
    m_orthoXYRadio = new QRadioButton("Oxy");
    m_orthoXZRadio = new QRadioButton("Oxz");
    m_orthoYZRadio = new QRadioButton("Oyz");

    m_perspectiveRadio->setChecked(true);

    QButtonGroup *projButtonGroup = new QButtonGroup(this);
    projButtonGroup->addButton(m_perspectiveRadio);
    projButtonGroup->addButton(m_orthoXYRadio);
    projButtonGroup->addButton(m_orthoXZRadio);
    projButtonGroup->addButton(m_orthoYZRadio);

    QHBoxLayout *projRow1 = new QHBoxLayout();
    projRow1->addWidget(m_perspectiveRadio);
    projRow1->addStretch();

    QHBoxLayout *projRow2 = new QHBoxLayout();
    projRow2->addWidget(m_orthoXYRadio);
    projRow2->addWidget(m_orthoXZRadio);
    projRow2->addWidget(m_orthoYZRadio);
    projRow2->addStretch();

    projectionLayout->addLayout(projRow1);
    projectionLayout->addLayout(projRow2);

    m_resetViewButton = new QPushButton("Сброс камеры");

    projectionLayout->addWidget(m_resetViewButton);

    QGroupBox *displayGroup = new QGroupBox("Отображение");
    QVBoxLayout *displayLayout = new QVBoxLayout(displayGroup);

    m_showGridCheckBox = new QCheckBox("Сетка");
    m_showGridCheckBox->setChecked(true);

    m_showAxesCheckBox = new QCheckBox("Оси координат");
    m_showAxesCheckBox->setChecked(true);

    m_showRotationAxisCheckBox = new QCheckBox("Ось вращения");
    m_showRotationAxisCheckBox->setChecked(true);

    QHBoxLayout *displayRow1 = new QHBoxLayout();
    displayRow1->addWidget(m_showGridCheckBox);
    displayRow1->addWidget(m_showAxesCheckBox);
    displayRow1->addStretch();

    QHBoxLayout *displayRow2 = new QHBoxLayout();
    displayRow2->addWidget(m_showRotationAxisCheckBox);
    displayRow2->addStretch();

    displayLayout->addLayout(displayRow1);
    displayLayout->addLayout(displayRow2);

    QGroupBox *matrixGroup = new QGroupBox("Матрица преобразования 4×4");
    QVBoxLayout *matrixLayout = new QVBoxLayout(matrixGroup);
    matrixLayout->setContentsMargins(5, 15, 5, 5);

    m_matrixTextEdit = new QTextEdit();
    m_matrixTextEdit->setReadOnly(true);
    m_matrixTextEdit->setFontFamily("Courier New");
    m_matrixTextEdit->setFontPointSize(10);

    m_matrixTextEdit->setStyleSheet(
        "QTextEdit {"
        "  background-color: #f0f0f0;"
        "  border: 1px solid #cccccc;"
        "  border-radius: 3px;"
        "  padding: 2px;"
        "}"
        );

    matrixLayout->addWidget(m_matrixTextEdit);

    controlLayout->addWidget(axisGroup);
    controlLayout->addWidget(rotationGroup);
    controlLayout->addWidget(scaleGroup);
    controlLayout->addWidget(translationGroup);
    controlLayout->addWidget(m_resetTransformButton);
    controlLayout->addWidget(projectionGroup);
    controlLayout->addWidget(displayGroup);
    controlLayout->addWidget(matrixGroup, 1);

    m_glWidget = new OpenGLWidget();

    mainLayout->addWidget(controlPanel);
    mainLayout->addWidget(m_glWidget, 1);
}

void MainWindow::setupConnections() {
    m_rotationAngleSlider->setRange(-180, 180);
    connect(m_rotationAngleSlider, &QSlider::valueChanged, this, &MainWindow::onRotationAngleChanged);

    connect(m_scaleXSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onScaleXChanged);
    connect(m_scaleYSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onScaleYChanged);
    connect(m_scaleZSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onScaleZChanged);

    connect(m_translationXSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onTranslationXChanged);
    connect(m_translationYSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onTranslationYChanged);
    connect(m_translationZSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onTranslationZChanged);

    connect(m_axisPointXSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onAxisPointXChanged);
    connect(m_axisPointYSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onAxisPointYChanged);
    connect(m_axisPointZSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onAxisPointZChanged);

    connect(m_axisDirectionXSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onAxisDirectionXChanged);
    connect(m_axisDirectionYSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onAxisDirectionYChanged);
    connect(m_axisDirectionZSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onAxisDirectionZChanged);

    connect(m_setAxisButton, &QPushButton::clicked, this, &MainWindow::onSetAxisClicked);
    connect(m_resetAxisButton, &QPushButton::clicked, this, &MainWindow::onResetAxisClicked);
    connect(m_resetRotationButton, &QPushButton::clicked, this, &MainWindow::onResetRotationClicked);

    connect(m_perspectiveRadio, &QRadioButton::clicked, this, &MainWindow::onProjectionChanged);
    connect(m_orthoXYRadio, &QRadioButton::clicked, this, &MainWindow::onProjectionChanged);
    connect(m_orthoXZRadio, &QRadioButton::clicked, this, &MainWindow::onProjectionChanged);
    connect(m_orthoYZRadio, &QRadioButton::clicked, this, &MainWindow::onProjectionChanged);

    connect(m_resetViewButton, &QPushButton::clicked, this, &MainWindow::onResetViewClicked);
    connect(m_resetTransformButton, &QPushButton::clicked, this, &MainWindow::onResetTransformClicked);

    connect(m_showGridCheckBox, &QCheckBox::stateChanged, this, &MainWindow::onShowGridChanged);
    connect(m_showAxesCheckBox, &QCheckBox::stateChanged, this, &MainWindow::onShowAxesChanged);
    connect(m_showRotationAxisCheckBox, &QCheckBox::stateChanged, this, &MainWindow::onShowRotationAxisChanged);

    connect(m_glWidget, &OpenGLWidget::cameraChanged, this, &MainWindow::onCameraChanged);
    connect(m_glWidget, &OpenGLWidget::rotationAngleChanged, this, &MainWindow::onRotationAngleChangedFromWidget);
    connect(m_glWidget, &OpenGLWidget::rotationAxisChanged, this, &MainWindow::onRotationAxisChangedFromWidget);
}

void MainWindow::onRotationAngleChanged(int value) {
    m_rotationAngle = value;
    m_rotationAngleLabel->setText(QString("%1°").arg(value));
    updateTransformMatrix();
}

void MainWindow::onScaleXChanged(double value) {
    m_scaleX = value;
    updateTransformMatrix();
}

void MainWindow::onScaleYChanged(double value) {
    m_scaleY = value;
    updateTransformMatrix();
}

void MainWindow::onScaleZChanged(double value) {
    m_scaleZ = value;
    updateTransformMatrix();
}

void MainWindow::onTranslationXChanged(double value) {
    m_translationX = value;
    updateTransformMatrix();
}

void MainWindow::onTranslationYChanged(double value) {
    m_translationY = value;
    updateTransformMatrix();
}

void MainWindow::onTranslationZChanged(double value) {
    m_translationZ = value;
    updateTransformMatrix();
}

void MainWindow::onAxisPointXChanged(double value) {
    m_axisPoint.setX(value);
}

void MainWindow::onAxisPointYChanged(double value) {
    m_axisPoint.setY(value);
}

void MainWindow::onAxisPointZChanged(double value) {
    m_axisPoint.setZ(value);
}

void MainWindow::onAxisDirectionXChanged(double value) {
    m_axisDirection.setX(value);
}

void MainWindow::onAxisDirectionYChanged(double value) {
    m_axisDirection.setY(value);
}

void MainWindow::onAxisDirectionZChanged(double value) {
    m_axisDirection.setZ(value);
}

void MainWindow::onSetAxisClicked() {
    if (m_axisDirection.length() < 0.0001) {
        QMessageBox::warning(this, "Ошибка", "Направляющий вектор не может быть нулевым!");
        return;
    }

    // Устанавливаем новую ось с сохранением положения объекта
    m_glWidget->setRotationAxis(m_axisPoint, m_axisDirection, true);

    // Синхронизируем параметры из виджета
    m_rotationAngle = m_glWidget->getRotationAngle(); // Теперь будет 0
    m_translationX = m_glWidget->getTranslation().x();
    m_translationY = m_glWidget->getTranslation().y();
    m_translationZ = m_glWidget->getTranslation().z();

    // Обновляем UI - сбрасываем слайдер вращения
    m_rotationAngleSlider->blockSignals(true);
    m_rotationAngleSlider->setValue(0); // Сбрасываем на 0
    m_rotationAngleLabel->setText("0°");
    m_rotationAngleSlider->blockSignals(false);

    m_translationXSpinBox->blockSignals(true);
    m_translationYSpinBox->blockSignals(true);
    m_translationZSpinBox->blockSignals(true);
    m_translationXSpinBox->setValue(m_translationX);
    m_translationYSpinBox->setValue(m_translationY);
    m_translationZSpinBox->setValue(m_translationZ);
    m_translationXSpinBox->blockSignals(false);
    m_translationYSpinBox->blockSignals(false);
    m_translationZSpinBox->blockSignals(false);

    // Обновляем матрицу
    updateTransformMatrix();
}

void MainWindow::onResetAxisClicked() {
    m_axisPoint = QVector3D(0, 0, 0);
    m_axisDirection = QVector3D(0, 0, 1);

    updateAxisControlsFromWidget(m_axisPoint, m_axisDirection);

    // Сбрасываем ось БЕЗ сохранения положения (false)
    m_glWidget->setRotationAxis(m_axisPoint, m_axisDirection, false);

    // Обновляем данные из виджета
    m_rotationAngle = 0; // Обнуляем угол

    // Сбрасываем трансляции тоже
    m_translationX = 0;
    m_translationY = 0;
    m_translationZ = 0;

    resetRotationSlider(); // Эта функция уже сбрасывает слайдер на 0
    m_translationXSpinBox->setValue(0.0);
    m_translationYSpinBox->setValue(0.0);
    m_translationZSpinBox->setValue(0.0);

    // Обновляем матрицу
    updateTransformMatrix();
}

void MainWindow::onResetRotationClicked() {
    // Сбрасываем только угол вращения
    m_rotationAngle = 0;
    m_glWidget->resetRotation();
    resetRotationSlider();
}

void MainWindow::onProjectionChanged() {
    if (m_perspectiveRadio->isChecked()) {
        m_glWidget->setPerspectiveProjection();
    } else if (m_orthoXYRadio->isChecked()) {
        m_glWidget->setOrthographicProjection(0);
    } else if (m_orthoXZRadio->isChecked()) {
        m_glWidget->setOrthographicProjection(1);
    } else if (m_orthoYZRadio->isChecked()) {
        m_glWidget->setOrthographicProjection(2);
    }
}

void MainWindow::onResetViewClicked() {
    m_glWidget->resetCamera();
}

void MainWindow::onResetTransformClicked() {
    // Сбрасываем все параметры
    m_rotationAngle = 0;
    m_scaleX = m_scaleY = m_scaleZ = 1.0;
    m_translationX = m_translationY = m_translationZ = 0.0;
    m_axisPoint = QVector3D(0, 0, 0);
    m_axisDirection = QVector3D(0, 0, 1);

    resetRotationSlider();
    m_scaleXSpinBox->setValue(1.0);
    m_scaleYSpinBox->setValue(1.0);
    m_scaleZSpinBox->setValue(1.0);
    m_translationXSpinBox->setValue(0.0);
    m_translationYSpinBox->setValue(0.0);
    m_translationZSpinBox->setValue(0.0);
    updateAxisControlsFromWidget(m_axisPoint, m_axisDirection);

    // Устанавливаем ось с сбросом всех преобразований (false - не сохранять положение)
    m_glWidget->setRotationAxis(m_axisPoint, m_axisDirection, false);
    updateTransformMatrix();
}

void MainWindow::onShowGridChanged(int state) {
    m_glWidget->setShowGrid(state == Qt::Checked);
}

void MainWindow::onShowAxesChanged(int state) {
    m_glWidget->setShowAxes(state == Qt::Checked);
}

void MainWindow::onShowRotationAxisChanged(int state) {
    m_glWidget->setShowRotationAxis(state == Qt::Checked);
}

void MainWindow::onCameraChanged(float pitch, float yaw, float zoom) {
    QString cameraInfo = QString("Камера: Pitch: %1°, Yaw: %2°, Zoom: %3")
                             .arg(pitch, 0, 'f', 1)
                             .arg(yaw, 0, 'f', 1)
                             .arg(zoom, 0, 'f', 2);
    statusBar()->showMessage(cameraInfo, 3000);
}

void MainWindow::onRotationAngleChangedFromWidget(float angle) {
    if (m_rotationAngle != angle) {
        m_rotationAngle = angle;
        m_rotationAngleSlider->blockSignals(true);
        m_rotationAngleSlider->setValue(static_cast<int>(angle));
        m_rotationAngleLabel->setText(QString("%1°").arg(angle));
        m_rotationAngleSlider->blockSignals(false);
        updateMatrixDisplay();
    }
}

void MainWindow::onRotationAxisChangedFromWidget(const QVector3D &point, const QVector3D &direction) {
    updateAxisControlsFromWidget(point, direction);
    updateMatrixDisplay();
}

void MainWindow::updateTransformMatrix() {
    // Сначала обновляем параметры в виджете
    m_glWidget->setTransformParameters(
        m_rotationAngle,
        m_scaleX, m_scaleY, m_scaleZ,
        m_translationX, m_translationY, m_translationZ
        );

    // Теперь обновляем матрицу в MainWindow
    QMatrix4x4 transform;
    transform.setToIdentity();

    // Трансляция
    transform.translate(m_translationX, m_translationY, m_translationZ);

    // Вращение
    if (m_rotationAngle != 0) {
        QMatrix4x4 rotation = Transformations::createRotationAroundAxis(
            m_axisPoint, m_axisDirection, m_rotationAngle);
        transform = transform * rotation;
    }

    // Масштабирование
    transform.scale(m_scaleX, m_scaleY, m_scaleZ);

    m_currentTransform = transform;
    updateMatrixDisplay();
}

void MainWindow::updateMatrixDisplay() {
    QString matrixText;
    matrixText += "Матрица преобразования:\n\n";

    // Компактное отображение матрицы
    for (int row = 0; row < 4; ++row) {
        matrixText += "[";
        for (int col = 0; col < 4; ++col) {
            float value = m_currentTransform(col, row);

            // Форматируем компактно
            if (fabs(value) < 0.001f && value != 0.0f) {
                matrixText += " 0.000";
            } else {
                matrixText += QString("%1").arg(value, 6, 'f', 3);
            }

            if (col < 3) matrixText += " ";
        }
        matrixText += "]\n";
    }

    matrixText += QString("\nУгол: %1°  ").arg(m_rotationAngle, 0, 'f', 1);
    matrixText += QString("Масштаб: %1,%2,%3\n")
                      .arg(m_scaleX, 0, 'f', 1).arg(m_scaleY, 0, 'f', 1).arg(m_scaleZ, 0, 'f', 1);
    matrixText += QString("Перенос: %1,%2,%3")
                      .arg(m_translationX, 0, 'f', 1).arg(m_translationY, 0, 'f', 1).arg(m_translationZ, 0, 'f', 1);

    m_matrixTextEdit->setText(matrixText);
}

void MainWindow::updateAxisControlsFromWidget(const QVector3D &point, const QVector3D &direction) {
    m_axisPoint = point;
    m_axisDirection = direction;

    m_axisPointXSpinBox->blockSignals(true);
    m_axisPointYSpinBox->blockSignals(true);
    m_axisPointZSpinBox->blockSignals(true);
    m_axisDirectionXSpinBox->blockSignals(true);
    m_axisDirectionYSpinBox->blockSignals(true);
    m_axisDirectionZSpinBox->blockSignals(true);

    m_axisPointXSpinBox->setValue(point.x());
    m_axisPointYSpinBox->setValue(point.y());
    m_axisPointZSpinBox->setValue(point.z());
    m_axisDirectionXSpinBox->setValue(direction.x());
    m_axisDirectionYSpinBox->setValue(direction.y());
    m_axisDirectionZSpinBox->setValue(direction.z());

    m_axisPointXSpinBox->blockSignals(false);
    m_axisPointYSpinBox->blockSignals(false);
    m_axisPointZSpinBox->blockSignals(false);
    m_axisDirectionXSpinBox->blockSignals(false);
    m_axisDirectionYSpinBox->blockSignals(false);
    m_axisDirectionZSpinBox->blockSignals(false);
}

void MainWindow::resetRotationSlider() {
    m_rotationAngle = 0;
    m_rotationAngleSlider->blockSignals(true);
    m_rotationAngleSlider->setValue(0);
    m_rotationAngleLabel->setText("0°");
    m_rotationAngleSlider->blockSignals(false);
}

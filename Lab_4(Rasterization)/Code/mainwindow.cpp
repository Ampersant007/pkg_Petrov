#include "MainWindow.h"
#include <QFormLayout>
#include <QIntValidator>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Создание OpenGL виджета
    glWidget = new RasterizationWidget(this);
    glWidget->setMinimumSize(600, 500);

    // ===== СОЗДАНИЕ ЭЛЕМЕНТОВ УПРАВЛЕНИЯ =====

    // Комбо-бокс выбора алгоритма
    algorithmCombo = new QComboBox(this);
    algorithmCombo->addItem("📈 Пошаговый алгоритм");
    algorithmCombo->addItem("📊 Алгоритм ЦДА");
    algorithmCombo->addItem("📐 Алгоритм Брезенхема (линия)");
    algorithmCombo->addItem("⭕ Алгоритм Брезенхема (окружность)");
    algorithmCombo->addItem("🎯 Алгоритм Кастла-Питвея");
    algorithmCombo->addItem("✨ Алгоритм ВУ (сглаживание)");

    // Чек-боксы
    showGridCheck = new QCheckBox("Показывать сетку", this);
    showGridCheck->setChecked(true);

    // Кнопки
    resetViewButton = new QPushButton("🔍 Сброс вида", this);
    centerViewButton = new QPushButton("🎯 Центрировать", this);
    clearButton = new QPushButton("🗑️ Очистить", this);
    applyCoordinatesButton = new QPushButton("➡️ Применить координаты", this);

    // Поля ввода координат
    startXSpin = new QSpinBox(this);
    startYSpin = new QSpinBox(this);
    endXSpin = new QSpinBox(this);
    endYSpin = new QSpinBox(this);

    // Настройка спинбоксов
    startXSpin->setRange(-1000, 1000);
    startYSpin->setRange(-1000, 1000);
    endXSpin->setRange(-1000, 1000);
    endYSpin->setRange(-1000, 1000);
    startXSpin->setValue(0);
    startYSpin->setValue(0);
    endXSpin->setValue(0);
    endYSpin->setValue(0);

    // Информационные метки
    infoLabel = new QLabel("Выберите алгоритм для визуализации", this);
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("QLabel { background-color: #f0f0f0; padding: 8px; border-radius: 4px; }");

    instructionLabel = new QLabel("💡 Первый клик - выделение клетки, второй клик - построение фигуры", this);
    instructionLabel->setWordWrap(true);
    instructionLabel->setStyleSheet("QLabel { background-color: #e3f2fd; padding: 6px; border-radius: 3px; color: #1565c0; }");

    coordinatesLabel = new QLabel("Координаты: (0, 0) -> (0, 0)", this);
    mousePosLabel = new QLabel("Мышь: (0, 0)", this);
    viewInfoLabel = new QLabel("Масштаб: 1.00x", this);
    timeInfoLabel = new QLabel("Время: -", this); // Добавляем метку времени
    timeInfoLabel->setStyleSheet("QLabel { color: #d32f2f; font-weight: bold; }");

    // ===== ГРУППЫ УПРАВЛЕНИЯ =====

    // Группа алгоритмов
    QGroupBox *algorithmGroup = new QGroupBox("🎛️ Выбор алгоритма", this);
    QVBoxLayout *algorithmLayout = new QVBoxLayout;
    algorithmLayout->addWidget(algorithmCombo);
    algorithmGroup->setLayout(algorithmLayout);

    // Группа координат
    QGroupBox *coordinatesGroup = new QGroupBox("📍 Ввод координат", this);
    QFormLayout *coordLayout = new QFormLayout;
    coordLayout->addRow("Начало X:", startXSpin);
    coordLayout->addRow("Начало Y:", startYSpin);
    coordLayout->addRow("Конец X:", endXSpin);
    coordLayout->addRow("Конец Y:", endYSpin);
    coordLayout->addRow(applyCoordinatesButton);
    coordinatesGroup->setLayout(coordLayout);

    // Группа вида
    QGroupBox *viewGroup = new QGroupBox("👁️ Настройки вида", this);
    QFormLayout *viewLayout = new QFormLayout;
    viewLayout->addRow(showGridCheck);
    viewLayout->addRow(resetViewButton);
    viewLayout->addRow(centerViewButton);
    viewLayout->addRow(clearButton);
    viewGroup->setLayout(viewLayout);

    // Группа информации
    QGroupBox *infoGroup = new QGroupBox("ℹ️ Информация", this);
    QVBoxLayout *infoLayout = new QVBoxLayout;
    infoLayout->addWidget(infoLabel);
    infoLayout->addWidget(instructionLabel);
    infoLayout->addWidget(coordinatesLabel);
    infoLayout->addWidget(mousePosLabel);
    infoLayout->addWidget(viewInfoLabel);
    infoLayout->addWidget(timeInfoLabel); // Добавляем в layout
    infoGroup->setLayout(infoLayout);

    // ===== ОСНОВНОЙ LAYOUT =====

    // Левая панель с управлением
    QWidget *controlPanel = new QWidget(this);
    QVBoxLayout *controlLayout = new QVBoxLayout;
    controlLayout->addWidget(algorithmGroup);
    controlLayout->addWidget(coordinatesGroup);
    controlLayout->addWidget(viewGroup);
    controlLayout->addWidget(infoGroup);
    controlLayout->addStretch();

    controlPanel->setLayout(controlLayout);
    controlPanel->setMaximumWidth(350);

    // Главный layout
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(controlPanel);
    mainLayout->addWidget(glWidget, 1);

    centralWidget->setLayout(mainLayout);

    // ===== ПОДКЛЮЧЕНИЕ СИГНАЛОВ =====

    connect(algorithmCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onAlgorithmChanged);
    connect(showGridCheck, &QCheckBox::stateChanged,
            this, &MainWindow::onShowGridChanged);
    connect(resetViewButton, &QPushButton::clicked,
            this, &MainWindow::onResetViewClicked);
    connect(centerViewButton, &QPushButton::clicked,
            this, &MainWindow::onCenterViewClicked);
    connect(clearButton, &QPushButton::clicked,
            this, &MainWindow::onClearClicked);
    connect(applyCoordinatesButton, &QPushButton::clicked,
            this, &MainWindow::onApplyCoordinatesClicked);

    // Подключение сигналов от RasterizationWidget
    connect(glWidget, &RasterizationWidget::lineFinished,
            this, &MainWindow::onLineFinished);
    connect(glWidget, &RasterizationWidget::mouseMovedToGridPos,
            this, &MainWindow::onMouseMovedToGridPos);
    connect(glWidget, &RasterizationWidget::algorithmTimeUpdated,
            this, &MainWindow::onAlgorithmTimeUpdated); // Подключаем новый сигнал

    // Установка начального состояния
    updateCoordinateDisplay();
    onAlgorithmChanged(0);
}

void MainWindow::onAlgorithmChanged(int index)
{
    glWidget->setCurrentAlgorithm(static_cast<RasterizationWidget::Algorithm>(index));

    QString info;
    switch(index) {
    case 0:
        info = "📈 <b>Пошаговый алгоритм</b><br>"
               "Простейший метод растеризации. Вычисляет координаты Y для каждого X.";
        break;
    case 1:
        info = "📊 <b>Алгоритм ЦДА</b><br>"
               "Улучшенная версия пошагового алгоритма. Использует приращения.";
        break;
    case 2:
        info = "📐 <b>Алгоритм Брезенхема (линия)</b><br>"
               "Эффективный целочисленный алгоритм для рисования отрезков.";
        break;
    case 3:
        info = "⭕ <b>Алгоритм Брезенхема (окружность)</b><br>"
               "Адаптация алгоритма Брезенхема для рисования окружностей.";
        break;
    case 4:
        info = "🎯 <b>Алгоритм Кастла-Питвея</b><br>"
               "Целочисленный алгоритм на основе алгоритма Евклида.";
        break;
    case 5:
        info = "✨ <b>Алгоритм ВУ</b><br>"
               "Сглаживание линий (антиалиасинг).";
        break;
    }

    infoLabel->setText(info);
}

void MainWindow::onShowGridChanged(int state)
{
    glWidget->setShowGrid(state == Qt::Checked);
}

void MainWindow::onResetViewClicked()
{
    glWidget->resetView();
    viewInfoLabel->setText("Масштаб: 1.00x");
}

void MainWindow::onCenterViewClicked()
{
    glWidget->centerView();
    viewInfoLabel->setText("Центрировано: (0,0) в центре");
}

void MainWindow::onClearClicked()
{
    glWidget->clearAllLines();
    startXSpin->setValue(0);
    startYSpin->setValue(0);
    endXSpin->setValue(0);
    endYSpin->setValue(0);
    updateCoordinateDisplay();
}

void MainWindow::onApplyCoordinatesClicked()
{
    int startX = startXSpin->value();
    int startY = startYSpin->value();
    int endX = endXSpin->value();
    int endY = endYSpin->value();

    glWidget->setLineStart(QPoint(startX, startY));
    glWidget->setLineEnd(QPoint(endX, endY));
    updateCoordinateDisplay();
}

void MainWindow::onLineFinished(const QPoint& start, const QPoint& end)
{
    // Обновляем поля ввода при завершении отрезка
    startXSpin->setValue(start.x());
    startYSpin->setValue(start.y());
    endXSpin->setValue(end.x());
    endYSpin->setValue(end.y());
    updateCoordinateDisplay();
}

void MainWindow::onMouseMovedToGridPos(const QPoint& gridPos)
{
    mousePosLabel->setText(QString("Мышь: (%1, %2)").arg(gridPos.x()).arg(gridPos.y()));
}

void MainWindow::updateCoordinateDisplay()
{
    QPoint start = glWidget->getLineStart();
    QPoint end = glWidget->getLineEnd();

    coordinatesLabel->setText(
        QString("Координаты: (%1, %2) -> (%3, %4)")
            .arg(start.x()).arg(start.y())
            .arg(end.x()).arg(end.y())
        );
}

// Добавляем реализацию слота
void MainWindow::onAlgorithmTimeUpdated(const QString& timeInfo)
{
    timeInfoLabel->setText("Время: " + timeInfo);
}


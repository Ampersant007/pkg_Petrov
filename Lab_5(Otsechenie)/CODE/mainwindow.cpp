#include "mainwindow.h"
#include <fstream>
#include <sstream>
#include <cmath>
#include <QPen>
#include <QPainterPath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), dataLoaded(false), sceneScale(50.0),
    currentAlgorithm(LIANG_BARSKY), hasTempPoint(false),
    clipRectItem(nullptr), tempPointIndicator(nullptr),
    xAxis(nullptr), yAxis(nullptr)
{
    setupUI();
}

MainWindow::~MainWindow()
{
    if (clipRectItem) {
        delete clipRectItem;
    }
    if (tempPointIndicator) {
        delete tempPointIndicator;
    }
    for (auto item : clipPolygonItems) {
        if (item) delete item;
    }
}

void MainWindow::setupUI()
{
    setWindowTitle("Алгоритмы отсечения отрезков");
    setMinimumSize(1400, 900);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    scene = new QGraphicsScene(this);
    scene->setSceneRect(-1000, -1000, 2000, 2000);

    view = new GraphicsView(this);
    view->setScene(scene);
    view->setRenderHint(QPainter::Antialiasing);
    view->setMinimumSize(800, 600);

    algorithmComboBox = new QComboBox(this);
    algorithmComboBox->addItem("Алгоритм Лианга-Барски");
    algorithmComboBox->addItem("Алгоритм Кируса-Бека");
    // Алгоритм Вейлера-Азертона удалён

    xMinSpin = new QDoubleSpinBox(this);
    xMinSpin->setRange(-20, 20);
    xMinSpin->setValue(0);
    xMinSpin->setSingleStep(0.1);
    xMinSpin->setDecimals(1);

    xMaxSpin = new QDoubleSpinBox(this);
    xMaxSpin->setRange(-20, 20);
    xMaxSpin->setValue(4);
    xMaxSpin->setSingleStep(0.1);
    xMaxSpin->setDecimals(1);

    yMinSpin = new QDoubleSpinBox(this);
    yMinSpin->setRange(-20, 20);
    yMinSpin->setValue(0);
    yMinSpin->setSingleStep(0.1);
    yMinSpin->setDecimals(1);

    yMaxSpin = new QDoubleSpinBox(this);
    yMaxSpin->setRange(-20, 20);
    yMaxSpin->setValue(3);
    yMaxSpin->setSingleStep(0.1);
    yMaxSpin->setDecimals(1);

    loadButton = new QPushButton("Загрузить отрезки из файла", this);
    loadPolygonButton = new QPushButton("Загрузить отсекатель", this);
    showOriginalButton = new QPushButton("Показать исходные данные", this);
    clipButton = new QPushButton("Выполнить отсечение", this);
    clearButton = new QPushButton("Очистить сцену", this);

    statusLabel = new QLabel("Загрузите данные для начала работы", this);
    mousePosLabel = new QLabel("Координаты: (0, 0)", this);

    fileFormatLabel = new QLabel(
        "<b>Формат файла отрезков:</b><br>"
        "n<br>"
        "X1_1 Y1_1 X2_1 Y2_1<br>"
        "...<br>"
        "X1_n Y1_n X2_n Y2_n<br><br>"
        "<b>Формат файла полигона (для Кируса-Бека):</b><br>"
        "m<br>"
        "X1 Y1<br>"
        "...<br>"
        "Xm Ym",
        this
        );
    fileFormatLabel->setWordWrap(true);

    infoLabel = new QLabel(
        "<b>Инструкция:</b><br>"
        "• Выберите алгоритм отсечения<br>"
        "• Для Лианга-Барски задайте параметры отсекателя<br>"
        "• Для Кируса-Бека загрузите выпуклый полигон-отсекатель<br>"
        "• ЛКМ на сетке для создания отрезков<br>"
        "• ПКМ + перемещение - панорамирование<br>"
        "• Колесо мыши для масштабирования<br><br>"
        "<b>Цветовая схема:</b><br>"
        "• <font color='red'>Красный</font> - отсекатель<br>"
        "• <font color='green'>Зелёный</font> - видимая часть<br>"
        "• <font color='black'>Чёрный</font> - отсечённая часть",
        this
        );
    infoLabel->setWordWrap(true);

    clipParamsGroup = new QGroupBox("Параметры отсекателя", this);
    QGridLayout *paramsLayout = new QGridLayout(clipParamsGroup);
    paramsLayout->addWidget(new QLabel("Xmin:"), 0, 0);
    paramsLayout->addWidget(xMinSpin, 0, 1);
    paramsLayout->addWidget(new QLabel("Xmax:"), 0, 2);
    paramsLayout->addWidget(xMaxSpin, 0, 3);
    paramsLayout->addWidget(new QLabel("Ymin:"), 1, 0);
    paramsLayout->addWidget(yMinSpin, 1, 1);
    paramsLayout->addWidget(new QLabel("Ymax:"), 1, 2);
    paramsLayout->addWidget(yMaxSpin, 1, 3);

    controlGroup = new QGroupBox("Управление", this);
    QVBoxLayout *controlLayout = new QVBoxLayout(controlGroup);
    controlLayout->addWidget(new QLabel("Выбор алгоритма:"));
    controlLayout->addWidget(algorithmComboBox);
    controlLayout->addWidget(clipParamsGroup);
    controlLayout->addWidget(loadButton);
    controlLayout->addWidget(loadPolygonButton);
    controlLayout->addWidget(showOriginalButton);
    controlLayout->addWidget(clipButton);
    controlLayout->addWidget(clearButton);
    controlLayout->addWidget(statusLabel);
    controlLayout->addWidget(mousePosLabel);
    controlLayout->addWidget(fileFormatLabel);
    controlLayout->addWidget(infoLabel);
    controlLayout->addStretch();

    connect(loadButton, &QPushButton::clicked, this, &MainWindow::loadData);
    connect(loadPolygonButton, &QPushButton::clicked, this, &MainWindow::loadPolygonPoints);
    connect(showOriginalButton, &QPushButton::clicked, this, &MainWindow::showOriginal);
    connect(clipButton, &QPushButton::clicked, this, &MainWindow::clipSegments);
    connect(clearButton, &QPushButton::clicked, this, &MainWindow::clearScene);
    connect(algorithmComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onAlgorithmChanged);
    connect(view, &GraphicsView::scaleChanged, this, &MainWindow::onViewScaleChanged);
    connect(view, &GraphicsView::mouseMoved, this, &MainWindow::onMouseMoved);
    connect(view, &GraphicsView::mouseClicked, this, &MainWindow::onMouseClicked);
    connect(xMinSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onClipParamsChanged);
    connect(xMaxSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onClipParamsChanged);
    connect(yMinSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onClipParamsChanged);
    connect(yMaxSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onClipParamsChanged);

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->addWidget(controlGroup, 1);
    mainLayout->addWidget(view, 3);

    resetScene();
    updateStatus();

    onClipParamsChanged();
}

void MainWindow::onMouseMoved(const QPointF &scenePos)
{
    double x = scenePos.x() / sceneScale;
    double y = -scenePos.y() / sceneScale;

    x = round(x * 10) / 10.0;
    y = round(y * 10) / 10.0;

    mousePosLabel->setText(QString("Координаты: (%1, %2)").arg(x, 0, 'f', 1).arg(y, 0, 'f', 1));
}

void MainWindow::onMouseClicked(const QPointF &scenePos)
{
    double x = scenePos.x() / sceneScale;
    double y = -scenePos.y() / sceneScale;

    x = round(x * 10) / 10.0;
    y = round(y * 10) / 10.0;

    if (std::isnan(x) || std::isinf(x) || std::isnan(y) || std::isinf(y)) {
        return;
    }

    if (!hasTempPoint) {
        tempPoint = Point(x, y);
        hasTempPoint = true;

        if (tempPointIndicator) {
            tempPointIndicator->setRect(
                x * sceneScale - 3,
                -y * sceneScale - 3,
                6,
                6
                );
            tempPointIndicator->setVisible(true);
        }

        statusLabel->setText(QString("Выбрана точка (%1, %2). Кликните для второй точки").arg(x, 0, 'f', 1).arg(y, 0, 'f', 1));
    } else {
        Point secondPoint(x, y);

        if (std::abs(tempPoint.x - secondPoint.x) < 0.01 && std::abs(tempPoint.y - secondPoint.y) < 0.01) {
            statusLabel->setText("Точки совпадают. Выберите другую точку.");
            return;
        }

        segments.push_back(Segment(tempPoint, secondPoint));
        hasTempPoint = false;

        if (tempPointIndicator) {
            tempPointIndicator->setVisible(false);
        }

        QPen segmentPen(Qt::black);
        segmentPen.setWidth(2);
        scene->addLine(
            tempPoint.x * sceneScale, -tempPoint.y * sceneScale,
            secondPoint.x * sceneScale, -secondPoint.y * sceneScale,
            segmentPen
            );

        dataLoaded = true;
        statusLabel->setText(QString("Создан отрезок (%1,%2)-(%3,%4). Всего отрезков: %5")
                                 .arg(tempPoint.x, 0, 'f', 1).arg(tempPoint.y, 0, 'f', 1)
                                 .arg(secondPoint.x, 0, 'f', 1).arg(secondPoint.y, 0, 'f', 1)
                                 .arg(segments.size()));
    }
}

void MainWindow::onViewScaleChanged(double scale)
{
    statusLabel->setText(QString("Масштаб: %1%").arg(scale * 100, 0, 'f', 0));
}

void MainWindow::onAlgorithmChanged(int index)
{
    AlgorithmType newAlgorithm = static_cast<AlgorithmType>(index);

    if (newAlgorithm == currentAlgorithm) {
        return;
    }

    currentAlgorithm = newAlgorithm;

    if (currentAlgorithm == LIANG_BARSKY) {
        clipParamsGroup->setVisible(true);
        loadPolygonButton->setVisible(false);
        onClipParamsChanged();
    } else if (currentAlgorithm == CYRUS_BECK) {
        clipParamsGroup->setVisible(false);
        loadPolygonButton->setVisible(true);
        clipPolygon.clear();
        updateClipVisualization();
    }

    updateStatus();
    showOriginal();
}

void MainWindow::onClipParamsChanged()
{
    if (currentAlgorithm == LIANG_BARSKY) {
        static double oldXMin = xMinSpin->value();
        static double oldXMax = xMaxSpin->value();
        static double oldYMin = yMinSpin->value();
        static double oldYMax = yMaxSpin->value();

        double newXMin = xMinSpin->value();
        double newXMax = xMaxSpin->value();
        double newYMin = yMinSpin->value();
        double newYMax = yMaxSpin->value();

        bool valuesChanged = (newXMin != oldXMin || newXMax != oldXMax ||
                              newYMin != oldYMin || newYMax != oldYMax);

        if (!valuesChanged) {
            return;
        }

        oldXMin = newXMin;
        oldXMax = newXMax;
        oldYMin = newYMin;
        oldYMax = newYMax;

        clipMin.x = newXMin;
        clipMin.y = newYMin;
        clipMax.x = newXMax;
        clipMax.y = newYMax;

        if (clipMin.x >= clipMax.x) {
            statusLabel->setText("Ошибка: Xmin должен быть меньше Xmax");
            if (clipMin.x >= clipMax.x) {
                clipMax.x = clipMin.x + 0.1;
                xMaxSpin->setValue(clipMax.x);
            }
            return;
        }

        if (clipMin.y >= clipMax.y) {
            statusLabel->setText("Ошибка: Ymin должен быть меньше Ymax");
            if (clipMin.y >= clipMax.y) {
                clipMax.y = clipMin.y + 0.1;
                yMaxSpin->setValue(clipMax.y);
            }
            return;
        }

        clipPolygon = {
            Point(clipMin.x, clipMin.y),
            Point(clipMax.x, clipMin.y),
            Point(clipMax.x, clipMax.y),
            Point(clipMin.x, clipMax.y)
        };

        updateClipVisualization();
        updateStatus();
    }
}

void MainWindow::updateClipVisualization()
{
    if (clipRectItem) {
        scene->removeItem(clipRectItem);
        delete clipRectItem;
        clipRectItem = nullptr;
    }

    for (auto item : clipPolygonItems) {
        if (item) {
            scene->removeItem(item);
            delete item;
        }
    }
    clipPolygonItems.clear();

    if (currentAlgorithm == LIANG_BARSKY) {
        QPen clipPen(Qt::red);
        clipPen.setWidth(3);

        double sceneX1 = clipMin.x * sceneScale;
        double sceneY1 = -clipMax.y * sceneScale;
        double sceneX2 = clipMax.x * sceneScale;
        double sceneY2 = -clipMin.y * sceneScale;

        double width = sceneX2 - sceneX1;
        double height = sceneY2 - sceneY1;

        clipRectItem = scene->addRect(sceneX1, sceneY1, width, height, clipPen);
        clipRectItem->setZValue(1);

    } else if (currentAlgorithm == CYRUS_BECK && !clipPolygon.empty()) {
        QPen clipPen(Qt::red);
        clipPen.setWidth(3);

        for (size_t i = 0; i < clipPolygon.size(); i++) {
            Point p1 = clipPolygon[i];
            Point p2 = clipPolygon[(i + 1) % clipPolygon.size()];
            QGraphicsLineItem* line = scene->addLine(
                p1.x * sceneScale, -p1.y * sceneScale,
                p2.x * sceneScale, -p2.y * sceneScale,
                clipPen
                );
            line->setZValue(1);
            clipPolygonItems.push_back(line);
        }
    }
}

void MainWindow::updateStatus()
{
    if (currentAlgorithm == LIANG_BARSKY) {
        statusLabel->setText(QString("Лианга-Барски. Отсекатель: [%1,%2]×[%3,%4]. Отрезков: %5")
                                 .arg(clipMin.x, 0, 'f', 1).arg(clipMax.x, 0, 'f', 1)
                                 .arg(clipMin.y, 0, 'f', 1).arg(clipMax.y, 0, 'f', 1)
                                 .arg(segments.size()));
    } else if (currentAlgorithm == CYRUS_BECK) {
        statusLabel->setText(QString("Кируса-Бека. Вершин полигона: %1. Отрезков: %2")
                                 .arg(clipPolygon.size()).arg(segments.size()));
    }
}

void MainWindow::drawCoordinateSystem()
{
    QPen axisPen(Qt::blue);
    axisPen.setWidth(3);

    xAxis = scene->addLine(-1000, 0, 1000, 0, axisPen);
    yAxis = scene->addLine(0, -1000, 0, 1000, axisPen);

    QFont font;
    font.setPointSize(8);

    for (int i = -20; i <= 20; i++) {
        if (i == 0) continue;

        QGraphicsLineItem* tick = scene->addLine(i * sceneScale, -5, i * sceneScale, 5, axisPen);
        axisLabels.push_back(tick);

        if (i % 5 == 0) {
            QGraphicsTextItem *text = scene->addText(QString::number(i));
            text->setPos(i * sceneScale - 10, 10);
            text->setFont(font);
            axisLabels.push_back(text);
        }

        tick = scene->addLine(-5, i * sceneScale, 5, i * sceneScale, axisPen);
        axisLabels.push_back(tick);

        if (i % 5 == 0) {
            QGraphicsTextItem *text = scene->addText(QString::number(-i));
            text->setPos(10, i * sceneScale - 10);
            text->setFont(font);
            axisLabels.push_back(text);
        }
    }

    QGraphicsTextItem *xLabel = scene->addText("X");
    xLabel->setPos(980, 20);
    axisLabels.push_back(xLabel);

    QGraphicsTextItem *yLabel = scene->addText("Y");
    yLabel->setPos(20, -980);
    axisLabels.push_back(yLabel);
}

void MainWindow::drawGrid()
{
    QPen gridPen(QColor(200, 200, 200));
    gridPen.setWidth(1);

    for (int i = -20; i <= 20; i++) {
        QGraphicsLineItem* line = scene->addLine(i * sceneScale, -1000, i * sceneScale, 1000, gridPen);
        gridItems.push_back(line);
    }

    for (int i = -20; i <= 20; i++) {
        QGraphicsLineItem* line = scene->addLine(-1000, i * sceneScale, 1000, i * sceneScale, gridPen);
        gridItems.push_back(line);
    }
}

void MainWindow::clearDynamicItems()
{
    QList<QGraphicsItem*> items = scene->items();
    QList<QGraphicsItem*> protectedItems;

    for (QGraphicsItem* gridItem : gridItems) {
        protectedItems.push_back(gridItem);
    }

    for (QGraphicsItem* axisLabel : axisLabels) {
        protectedItems.push_back(axisLabel);
    }

    for (QGraphicsLineItem* polyItem : clipPolygonItems) {
        protectedItems.push_back(polyItem);
    }

    if (clipRectItem) {
        protectedItems.push_back(clipRectItem);
    }

    if (xAxis) protectedItems.push_back(xAxis);
    if (yAxis) protectedItems.push_back(yAxis);

    if (tempPointIndicator) {
        protectedItems.push_back(tempPointIndicator);
    }

    for (QGraphicsItem* item : items) {
        if (!protectedItems.contains(item)) {
            scene->removeItem(item);
            delete item;
        }
    }
}

void MainWindow::clearScene()
{
    scene->clear();

    segments.clear();
    clipPolygon.clear();
    dataLoaded = false;
    hasTempPoint = false;
    tempPointIndicator = nullptr;
    clipRectItem = nullptr;
    xAxis = nullptr;
    yAxis = nullptr;
    gridItems.clear();
    axisLabels.clear();
    clipPolygonItems.clear();

    xMinSpin->setValue(0);
    xMaxSpin->setValue(4);
    yMinSpin->setValue(0);
    yMaxSpin->setValue(3);

    resetScene();
    updateStatus();
}

void MainWindow::resetScene()
{
    scene->clear();

    tempPointIndicator = nullptr;
    clipRectItem = nullptr;
    xAxis = nullptr;
    yAxis = nullptr;
    gridItems.clear();
    axisLabels.clear();
    clipPolygonItems.clear();

    drawGrid();
    drawCoordinateSystem();

    tempPointIndicator = scene->addEllipse(0, 0, 6, 6, QPen(Qt::blue), QBrush(Qt::blue));
    tempPointIndicator->setVisible(false);
    tempPointIndicator->setZValue(10);

    updateClipVisualization();

    if (!segments.empty()) {
        drawSegments();
    }
}

void MainWindow::drawSegments()
{
    QPen segmentPen(Qt::black);
    segmentPen.setWidth(2);

    for (const auto& segment : segments) {
        scene->addLine(
            segment.p1.x * sceneScale, -segment.p1.y * sceneScale,
            segment.p2.x * sceneScale, -segment.p2.y * sceneScale,
            segmentPen
            );
    }
}

void MainWindow::loadData()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Открыть файл с отрезками", "", "Text files (*.txt)");

    if (fileName.isEmpty()) {
        return;
    }

    std::ifstream file(fileName.toStdString());
    if (!file.is_open()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось открыть файл");
        return;
    }

    segments.clear();

    try {
        int n;
        file >> n;

        for (int i = 0; i < n; i++) {
            double x1, y1, x2, y2;
            file >> x1 >> y1 >> x2 >> y2;
            segments.push_back(Segment(Point(x1, y1), Point(x2, y2)));
        }

        file.close();
        dataLoaded = true;
        statusLabel->setText(QString("Загружено %1 отрезков").arg(segments.size()));

        showOriginal();

    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Ошибка", "Ошибка при чтении файла: неверный формат данных");
        dataLoaded = false;
    }
}

void MainWindow::loadPolygonPoints()
{
    if (currentAlgorithm == LIANG_BARSKY) {
        return;
    }

    QString fileName = QFileDialog::getOpenFileName(this, "Открыть файл с полигоном", "", "Text files (*.txt)");

    if (fileName.isEmpty()) {
        return;
    }

    std::ifstream file(fileName.toStdString());
    if (!file.is_open()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось открыть файл");
        return;
    }

    std::vector<Point> loadedPolygon;

    try {
        int m;
        file >> m;

        if (m < 3) {
            throw std::runtime_error("Полигон должен иметь не менее 3 вершин");
        }

        for (int i = 0; i < m; i++) {
            double x, y;
            file >> x >> y;
            loadedPolygon.push_back(Point(x, y));
        }

        file.close();

        clipPolygon = Algorithms::makeCounterClockwise(loadedPolygon);

        if (!Algorithms::isConvexPolygon(clipPolygon)) {
            QMessageBox::warning(this, "Предупреждение",
                                 "Загруженный полигон не является выпуклым. Результаты могут быть некорректными.");
        }

        bool isCCW = Algorithms::isPolygonCounterClockwise(clipPolygon);
        statusLabel->setText(QString("Загружен полигон из %1 вершин (%2)")
                                 .arg(clipPolygon.size())
                                 .arg(isCCW ? "против часовой стрелки" : "по часовой стрелке (исправлено)"));

        updateClipVisualization();
        showOriginal();

    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Ошибка", "Ошибка при чтении файла: неверный формат данных");
    }
}

void MainWindow::clipSegments()
{
    clearDynamicItems();

    if (currentAlgorithm == LIANG_BARSKY) {
        QPen visiblePen(Qt::green);
        visiblePen.setWidth(5); // Увеличиваем толщину
        QPen hiddenPen(Qt::darkGray); // Меняем на более светлый серый
        hiddenPen.setWidth(1);
        hiddenPen.setStyle(Qt::DashLine); // Делаем пунктирными

        for (const auto& seg : segments) {
            double x1c, y1c, x2c, y2c;
            if (Algorithms::liangBarskyClip(seg.p1.x, seg.p1.y, seg.p2.x, seg.p2.y,
                                            clipMin.x, clipMin.y, clipMax.x, clipMax.y,
                                            x1c, y1c, x2c, y2c)) {
                // Сначала рисуем отсечённую часть
                scene->addLine(seg.p1.x * sceneScale, -seg.p1.y * sceneScale,
                               seg.p2.x * sceneScale, -seg.p2.y * sceneScale, hiddenPen);
                // Потом поверх - видимую часть
                scene->addLine(x1c * sceneScale, -y1c * sceneScale,
                               x2c * sceneScale, -y2c * sceneScale, visiblePen);
            } else {
                // Если полностью невидим - только отсечённую часть
                scene->addLine(seg.p1.x * sceneScale, -seg.p1.y * sceneScale,
                               seg.p2.x * sceneScale, -seg.p2.y * sceneScale, hiddenPen);
            }
        }
    } else if (currentAlgorithm == CYRUS_BECK) {
        QPen visiblePen(Qt::green);
        visiblePen.setWidth(5); // Увеличиваем толщину
        QPen hiddenPen(Qt::darkGray);
        hiddenPen.setWidth(1);
        hiddenPen.setStyle(Qt::DashLine); // Делаем пунктирными

        if (clipPolygon.size() < 3) {
            QMessageBox::warning(this, "Ошибка", "Загрузите полигон отсекателя для Кируса-Бека");
            return;
        }

        for (const auto& seg : segments) {
            Segment clipped;
            if (Algorithms::cyrusBeckClip(seg, clipPolygon, clipped)) {
                // Сначала рисуем отсечённую часть
                scene->addLine(seg.p1.x * sceneScale, -seg.p1.y * sceneScale,
                               seg.p2.x * sceneScale, -seg.p2.y * sceneScale, hiddenPen);
                // Потом поверх - видимую часть
                scene->addLine(clipped.p1.x * sceneScale, -clipped.p1.y * sceneScale,
                               clipped.p2.x * sceneScale, -clipped.p2.y * sceneScale, visiblePen);
            } else {
                // Если полностью невидим - только отсечённую часть
                scene->addLine(seg.p1.x * sceneScale, -seg.p1.y * sceneScale,
                               seg.p2.x * sceneScale, -seg.p2.y * sceneScale, hiddenPen);
            }
        }
    }
}

void MainWindow::showOriginal()
{
    clearDynamicItems();
    updateClipVisualization();
    drawSegments();
}

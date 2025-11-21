#include "RasterizationWidget.h"
#include <QPainter>
#include <QWheelEvent>
#include <QKeyEvent>
#include <algorithm>

RasterizationWidget::RasterizationWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    , currentAlgorithm(BRESENHAM_LINE)
    , inputMode(CLICK_INPUT)
    , drawing(false)
    , showGrid(true)
    , gridSize(20)
    , firstClickDone(false)
    , scale(1.0)
    , offset(0, 0)
    , panning(false)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

RasterizationWidget::~RasterizationWidget()
{
}

void RasterizationWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // Белый фон
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Центрируем вид при инициализации
    centerView();
}

void RasterizationWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, h, 0, -1, 1); // Ось Y вниз
    glMatrixMode(GL_MODELVIEW);

    // Перецентрируем при изменении размера
    centerView();
}

void RasterizationWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Применяем трансформации
    glScalef(scale, scale, 1.0f);
    glTranslatef(offset.x(), offset.y(), 0.0f);

    // Рисуем сетку
    if (showGrid) {
        drawGrid();
    }

    // Рисуем систему координат
    drawCoordinateSystem();

    // Рисуем координаты
    drawCoordinates();

    // Рисуем все сохраненные линии
    drawAllLines();

    // Если первый клик сделан, закрашиваем первую клетку
    if (firstClickDone) {
        drawGridCell(firstClickPoint.x(), firstClickPoint.y(), QColor(255, 200, 200));
    }

    // Рисуем текущий отрезок (предпросмотр)
    if ((drawing || (lineStart != lineEnd))) {
        switch(currentAlgorithm) {
        case STEP_BY_STEP:
            drawStepByStepLine(lineStart, lineEnd);
            break;
        case DDA:
            drawDDALine(lineStart, lineEnd);
            break;
        case BRESENHAM_LINE:
            drawBresenhamLine(lineStart, lineEnd);
            break;
        case BRESENHAM_CIRCLE:
            drawBresenhamCircle(lineStart,
                                static_cast<int>(sqrt(pow(lineEnd.x() - lineStart.x(), 2) +
                                                      pow(lineEnd.y() - lineStart.y(), 2))));
            break;
        case CASTLE_PITWAY:
            drawCastlePitwayLine(lineStart, lineEnd);
            break;
        case WU_LINE:
            drawWuLine(lineStart, lineEnd);
            break;
        default:
            break;
        }
    }
}

// Отрисовка всех сохраненных линий
void RasterizationWidget::drawAllLines()
{
    for (const auto& drawnLine : drawnLines) {
        switch(drawnLine.algorithm) {
        case STEP_BY_STEP:
            drawStepByStepLine(drawnLine.start, drawnLine.end);
            break;
        case DDA:
            drawDDALine(drawnLine.start, drawnLine.end);
            break;
        case BRESENHAM_LINE:
            drawBresenhamLine(drawnLine.start, drawnLine.end);
            break;
        case BRESENHAM_CIRCLE:
            drawBresenhamCircle(drawnLine.start,
                                static_cast<int>(sqrt(pow(drawnLine.end.x() - drawnLine.start.x(), 2) +
                                                      pow(drawnLine.end.y() - drawnLine.start.y(), 2))));
            break;
        case CASTLE_PITWAY:
            drawCastlePitwayLine(drawnLine.start, drawnLine.end);
            break;
        case WU_LINE:
            drawWuLine(drawnLine.start, drawnLine.end);
            break;
        default:
            break;
        }
    }
}

void RasterizationWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        // Начало перемещения
        lastMousePos = event->pos();
        panning = true;
        setCursor(Qt::ClosedHandCursor);
        return;
    }

    if (event->button() == Qt::LeftButton) {
        if (inputMode == CLICK_INPUT) {
            QPoint gridPos = screenToGrid(event->pos());

            if (!firstClickDone) {
                // Первый клик - закрашиваем одну клетку
                firstClickPoint = gridPos;
                firstClickDone = true;
                drawGridCell(gridPos.x(), gridPos.y(), QColor(255, 200, 200));
            } else {
                // Второй клик - рисуем отрезок между первой и второй точкой
                lineStart = firstClickPoint;
                lineEnd = gridPos;
                drawing = true;
                firstClickDone = false;

                // Сохраняем линию в список отрисованных
                DrawnLine newLine;
                newLine.start = lineStart;
                newLine.end = lineEnd;
                newLine.algorithm = currentAlgorithm;
                drawnLines.push_back(newLine);

                // Испускаем сигнал о завершении отрезка
                emit lineFinished(lineStart, lineEnd);
            }
            update();
        }
    }
}

void RasterizationWidget::mouseMoveEvent(QMouseEvent *event)
{
    // Обновляем текущую позицию мыши в координатах сетки
    currentMouseGridPos = screenToGrid(event->pos());
    emit mouseMovedToGridPos(currentMouseGridPos);

    if (panning) {
        // Перемещение вида
        QPoint delta = event->pos() - lastMousePos;
        offset += QPointF(delta.x() / scale, delta.y() / scale);
        lastMousePos = event->pos();
        update();
    } else if (firstClickDone && inputMode == CLICK_INPUT) {
        // Предпросмотр отрезка от первой точки до текущей позиции мыши
        lineStart = firstClickPoint;
        lineEnd = screenToGrid(event->pos());
        drawing = true;
        update();
    }
}

void RasterizationWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        panning = false;
        setCursor(Qt::ArrowCursor);
    }
}

void RasterizationWidget::wheelEvent(QWheelEvent *event)
{
    double zoomFactor = 1.1;

    // Сохраняем позицию мыши до масштабирования
    QPointF mouseBeforeZoom = screenToGrid(event->position());

    if (event->angleDelta().y() > 0) {
        // Приближение
        scale *= zoomFactor;
    } else {
        // Отдаление
        scale /= zoomFactor;
    }

    // Ограничиваем масштаб
    scale = std::max(0.1, std::min(scale, 10.0));

    // Корректируем смещение для zoom к курсору
    QPointF mouseAfterZoom = screenToGrid(event->position());
    offset += (mouseBeforeZoom - mouseAfterZoom);

    update();
}

void RasterizationWidget::keyPressEvent(QKeyEvent *event)
{
    // Управление клавишами для перемещения по сетке
    double moveStep = gridSize; // Один шаг = одна клетка сетки

    switch(event->key()) {
    case Qt::Key_Left:
        offset.setX(offset.x() - moveStep);
        break;
    case Qt::Key_Right:
        offset.setX(offset.x() + moveStep);
        break;
    case Qt::Key_Up:
        offset.setY(offset.y() - moveStep);
        break;
    case Qt::Key_Down:
        offset.setY(offset.y() + moveStep);
        break;
    case Qt::Key_Home:
        resetView();
        break;
    case Qt::Key_C:
        centerView();
        break;
    case Qt::Key_Plus:
    case Qt::Key_Equal:
        scale = std::min(scale * 1.1, 10.0);
        break;
    case Qt::Key_Minus:
        scale = std::max(scale / 1.1, 0.1);
        break;
    default:
        QOpenGLWidget::keyPressEvent(event);
        return;
    }

    update();
}



// Преобразование экранных координат в координаты сетки
QPoint RasterizationWidget::screenToGrid(const QPoint& screenPos) const
{
    QPointF worldPos((screenPos.x() / scale) - offset.x(),
                     (screenPos.y() / scale) - offset.y());

    int gridX = static_cast<int>(std::round(worldPos.x() / gridSize));
    int gridY = static_cast<int>(std::round(worldPos.y() / gridSize));

    return QPoint(gridX, gridY);
}

// Преобразование экранных координат в координаты сетки (для QPointF)
QPoint RasterizationWidget::screenToGrid(const QPointF& screenPos) const
{
    QPointF worldPos((screenPos.x() / scale) - offset.x(),
                     (screenPos.y() / scale) - offset.y());

    int gridX = static_cast<int>(std::round(worldPos.x() / gridSize));
    int gridY = static_cast<int>(std::round(worldPos.y() / gridSize));

    return QPoint(gridX, gridY);
}

//Преобразование координат сетки в экранные координаты
QPointF RasterizationWidget::gridToScreen(const QPoint& gridPos) const
{
    return QPointF(gridPos.x() * gridSize, gridPos.y() * gridSize);
}

void RasterizationWidget::drawGrid()
{
    glColor4f(0.0f, 0.0f, 0.0f, 0.3f); // Чёрная сетка с прозрачностью
    glBegin(GL_LINES);

    // Вычисляем видимую область в мировых координатах
    double left = -offset.x();
    double right = width() / scale - offset.x();
    double top = -offset.y();
    double bottom = height() / scale - offset.y();

    // Преобразуем в координаты сетки
    int startX = static_cast<int>(std::floor(left / gridSize)) - 1;
    int endX = static_cast<int>(std::ceil(right / gridSize)) + 1;
    int startY = static_cast<int>(std::floor(top / gridSize)) - 1;
    int endY = static_cast<int>(std::ceil(bottom / gridSize)) + 1;

    // Вертикальные линии
    for (int x = startX; x <= endX; ++x) {
        float screenX = x * gridSize;
        glVertex2f(screenX, startY * gridSize);
        glVertex2f(screenX, endY * gridSize);
    }

    // Горизонтальные линии
    for (int y = startY; y <= endY; ++y) {
        float screenY = y * gridSize;
        glVertex2f(startX * gridSize, screenY);
        glVertex2f(endX * gridSize, screenY);
    }

    glEnd();
}

void RasterizationWidget::drawCoordinateSystem()
{
    // Рисуем оси координат более толстыми и контрастными линиями
    glLineWidth(3.0f);

    // Ось X - красная
    glColor3f(0.8f, 0.2f, 0.2f);
    glBegin(GL_LINES);
    glVertex2f(-10000, 0);
    glVertex2f(10000, 0);
    glEnd();

    // Ось Y - зеленая
    glColor3f(0.2f, 0.6f, 0.2f);
    glBegin(GL_LINES);
    glVertex2f(0, -10000);
    glVertex2f(0, 10000);
    glEnd();

    // Восстанавливаем толщину линии
    glLineWidth(1.0f);

    // === ДОБАВЛЯЕМ ПОДПИСИ ДЕЛЕНИЙ НА ОСЯХ ===

    // Используем QPainter для текста
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::black);

    // Устанавливаем шрифт с учетом масштаба
    QFont font = painter.font();
    font.setPointSizeF(8.0 * scale); // Размер шрифта адаптируется к масштабу
    painter.setFont(font);

    // Вычисляем видимую область в мировых координатах
    double left = -offset.x();
    double right = width() / scale - offset.x();
    double top = -offset.y();
    double bottom = height() / scale - offset.y();

    // Преобразуем в координаты сетки
    int startX = static_cast<int>(std::floor(left / gridSize));
    int endX = static_cast<int>(std::ceil(right / gridSize));
    int startY = static_cast<int>(std::floor(top / gridSize));
    int endY = static_cast<int>(std::ceil(bottom / gridSize));

    // Определяем шаг для подписей в зависимости от масштаба
    int labelStep = 1;
    if (scale < 0.3) labelStep = 5;
    else if (scale < 0.1) labelStep = 10;
    else if (scale < 0.05) labelStep = 20;

    // Подписи на оси X
    for (int x = startX; x <= endX; ++x) {
        if (x % labelStep != 0) continue; // Пропускаем некоторые деления при маленьком масштабе

        QPointF screenPos = gridToScreen(QPoint(x, 0));
        QString label = QString::number(x);

        // Преобразуем мировые координаты в экранные для QPainter
        QPoint screenPoint(screenPos.x() * scale + offset.x() * scale,
                           screenPos.y() * scale + offset.y() * scale);

        // Рисуем засечку на оси (в мировых координатах)
        glColor3f(0.0f, 0.0f, 0.0f);
        glBegin(GL_LINES);
        glVertex2f(screenPos.x(), -3 / scale);
        glVertex2f(screenPos.x(), 3 / scale);
        glEnd();

        // Рисуем текст под осью (в экранных координатах)
        QRect textRect(screenPoint.x() - 15, screenPoint.y() + 5, 30, 15);
        painter.drawText(textRect, Qt::AlignCenter, label);
    }

    // Подписи на оси Y
    for (int y = startY; y <= endY; ++y) {
        if (y % labelStep != 0) continue; // Пропускаем некоторые деления при маленьком масштабе
        if (y == 0) continue; // Ноль уже обработан на оси X

        QPointF screenPos = gridToScreen(QPoint(0, y));
        QString label = QString::number(y);

        // Преобразуем мировые координаты в экранные для QPainter
        QPoint screenPoint(screenPos.x() * scale + offset.x() * scale,
                           screenPos.y() * scale + offset.y() * scale);

        // Рисуем засечку на оси (в мировых координатах)
        glColor3f(0.0f, 0.0f, 0.0f);
        glBegin(GL_LINES);
        glVertex2f(-3 / scale, screenPos.y());
        glVertex2f(3 / scale, screenPos.y());
        glEnd();

        // Рисуем текст слева от оси (в экранных координатах)
        QRect textRect(screenPoint.x() - 25, screenPoint.y() - 8, 20, 15);
        painter.drawText(textRect, Qt::AlignRight | Qt::AlignVCenter, label);
    }

    // Подпись начала координат (0,0)
    QPointF origin = gridToScreen(QPoint(0, 0));
    QPoint originScreen(origin.x() * scale + offset.x() * scale,
                        origin.y() * scale + offset.y() * scale);

    QRect originRect(originScreen.x() + 5, originScreen.y() - 15, 15, 15);
    painter.drawText(originRect, Qt::AlignCenter, "0");
}

void RasterizationWidget::drawCoordinates()
{
    // Отображаем координаты в углу экрана
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Получаем центр в координатах сетки
    QPoint centerGrid = screenToGrid(QPoint(width() / 2, height() / 2));

    // Информация о виде - РАЗДЕЛИМ на две строки
    QString scaleInfo = QString("Масштаб: %1x").arg(scale, 0, 'f', 2);
    QString offsetInfo = QString("Смещение: (%1, %2)")
                             .arg(offset.x(), 0, 'f', 1)
                             .arg(offset.y(), 0, 'f', 1);

    // Текущие координаты мыши
    QString mouseInfo = QString("Мышь: (%1, %2)")
                            .arg(currentMouseGridPos.x())
                            .arg(currentMouseGridPos.y());

    // Координаты отрезка
    QString lineInfo = QString("Отрезок: (%1, %2) -> (%3, %4)")
                           .arg(lineStart.x())
                           .arg(lineStart.y())
                           .arg(lineEnd.x())
                           .arg(lineEnd.y());

    // Количество отрисованных линий
    QString linesInfo = QString("Линий: %1").arg(drawnLines.size());

    // Алгоритм
    QString algorithmInfo;
    switch(currentAlgorithm) {
    case STEP_BY_STEP:
        algorithmInfo = "Алгоритм: Пошаговый";
        break;
    case DDA:
        algorithmInfo = "Алгоритм: ЦДА";
        break;
    case BRESENHAM_LINE:
        algorithmInfo = "Алгоритм: Брезенхем (линия)";
        break;
    case BRESENHAM_CIRCLE:
        algorithmInfo = "Алгоритм: Брезенхем (окружность)";
        break;
    case CASTLE_PITWAY:
        algorithmInfo = "Алгоритм: Кастла-Питвея";
        break;
    case WU_LINE:
        algorithmInfo = "Алгоритм: ВУ (сглаживание)";
        break;
    }

    // Рисуем информационную панель в левом верхнем углу
    QFont font = painter.font();
    font.setPointSize(9);
    painter.setFont(font);

    // Фон для информационной панели (немного увеличим высоту)
    QRect infoRect(10, 10, 200, 130);
    painter.fillRect(infoRect, QColor(255, 255, 255, 220));
    painter.setPen(Qt::darkGray);
    painter.drawRect(infoRect);

    // Текст информации
    painter.setPen(Qt::black);
    int yPos = 25;
    painter.drawText(15, yPos, algorithmInfo); yPos += 20;
    painter.drawText(15, yPos, scaleInfo); yPos += 20;
    painter.drawText(15, yPos, offsetInfo); yPos += 20;
    painter.drawText(15, yPos, mouseInfo); yPos += 20;
    painter.drawText(15, yPos, lineInfo); yPos += 20;
    painter.drawText(15, yPos, linesInfo);

    // Управление в правом нижнем углу
    QString controlsInfo = "Управление:\n"
                           "ЛКМ - точки\n"
                           "ПКМ - перемещение\n"
                           "Колесо - масштаб\n"
                           "Стрелки - движение\n"
                           "+/- - масштаб\n"
                           "Home - сброс\n"
                           "C - центр";

    QRect controlsRect(width() - 150, height() - 150, 140, 140);
    painter.fillRect(controlsRect, QColor(255, 255, 255, 200));
    painter.setPen(Qt::darkGray);
    painter.drawRect(controlsRect);
    painter.setPen(Qt::black);
    painter.drawText(controlsRect.adjusted(5, 5, -5, -5), controlsInfo);
}

// Рисование закрашенной клетки сетки
void RasterizationWidget::drawGridCell(int gridX, int gridY, const QColor& color)
{
    QPointF screenPos = gridToScreen(QPoint(gridX, gridY));

    glColor3f(color.redF(), color.greenF(), color.blueF());
    glBegin(GL_QUADS);
    glVertex2f(screenPos.x(), screenPos.y());
    glVertex2f(screenPos.x() + gridSize, screenPos.y());
    glVertex2f(screenPos.x() + gridSize, screenPos.y() + gridSize);
    glVertex2f(screenPos.x(), screenPos.y() + gridSize);
    glEnd();

    // Рамка вокруг клетки
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(screenPos.x(), screenPos.y());
    glVertex2f(screenPos.x() + gridSize, screenPos.y());
    glVertex2f(screenPos.x() + gridSize, screenPos.y() + gridSize);
    glVertex2f(screenPos.x(), screenPos.y() + gridSize);
    glEnd();
}

void RasterizationWidget::drawStepByStepLine(const QPoint& start, const QPoint& end)
{
    algorithmTimer.start();

    int x1 = start.x(), y1 = start.y();
    int x2 = end.x(), y2 = end.y();

    if (x1 == x2) {
        // Вертикальная линия
        int minY = std::min(y1, y2);
        int maxY = std::max(y1, y2);
        for (int y = minY; y <= maxY; y++) {
            drawGridCell(x1, y, QColor(0, 100, 200)); // Синий цвет
        }
    } else {
        float m = float(y2 - y1) / (x2 - x1);
        if(abs(m) > 1) {
            float b = y1 - m * x1;

            int startY = std::min(y1, y2);
            int endY = std::max(y1, y2);

            for (int y = startY; y <= endY; y++) {
                int x = static_cast<int>((y - b)/m);
                drawGridCell(x, y, QColor(0, 100, 200)); // Синий цвет
            }
        } else{
            float b = y1 - m * x1;

            int startX = std::min(x1, x2);
            int endX = std::max(x1, x2);

            for (int x = startX; x <= endX; x++) {
                int y = static_cast<int>(m * x + b);
                drawGridCell(x, y, QColor(0, 100, 200)); // Синий цвет
            }
        }
    }
    lastAlgorithmTime = QString("Пошаговый: %1 мс").arg(algorithmTimer.elapsed());
    emit algorithmTimeUpdated(lastAlgorithmTime);
}

void RasterizationWidget::drawDDALine(const QPoint& start, const QPoint& end)
{
    algorithmTimer.start();

    int x1 = start.x(), y1 = start.y();
    int x2 = end.x(), y2 = end.y();

    int dx = x2 - x1;
    int dy = y2 - y1;
    int steps = std::max(abs(dx), abs(dy));

    if (steps == 0) {
        drawGridCell(x1, y1, QColor(200, 100, 0)); // Оранжевый цвет
        return;
    }

    float xInc = dx / (float)steps;
    float yInc = dy / (float)steps;

    float x = x1;
    float y = y1;

    for (int i = 0; i <= steps; i++) {
        drawGridCell(static_cast<int>(round(x)), static_cast<int>(round(y)), QColor(200, 100, 0)); // Оранжевый цвет
        x += xInc;
        y += yInc;
    }

    lastAlgorithmTime = QString("ЦДА: %1 мс").arg(algorithmTimer.elapsed());
    emit algorithmTimeUpdated(lastAlgorithmTime);
}

void RasterizationWidget::drawBresenhamLine(const QPoint& start, const QPoint& end)
{
    algorithmTimer.start();

    int x1 = start.x(), y1 = start.y();
    int x2 = end.x(), y2 = end.y();

    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    int x = x1;
    int y = y1;

    while (true) {
        drawGridCell(x, y, QColor(0, 150, 0)); // Зеленый цвет

        if (x == x2 && y == y2) break;

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x += sx;
        }
        if (e2 < dx) {
            err += dx;
            y += sy;
        }
    }

    lastAlgorithmTime = QString("Брезенхем: %1 мс").arg(algorithmTimer.elapsed());
    emit algorithmTimeUpdated(lastAlgorithmTime);
}

void RasterizationWidget::drawBresenhamCircle(const QPoint& center, int radius)
{
    algorithmTimer.start();

    int xc = center.x(), yc = center.y();
    int x = 0;
    int y = radius;
    int d = 3 - 2 * radius;

    while (x <= y) {
        // Отрисовка 8 точек симметрии
        drawGridCell(xc + x, yc + y, QColor(150, 0, 150)); // Фиолетовый цвет
        drawGridCell(xc - x, yc + y, QColor(150, 0, 150));
        drawGridCell(xc + x, yc - y, QColor(150, 0, 150));
        drawGridCell(xc - x, yc - y, QColor(150, 0, 150));
        drawGridCell(xc + y, yc + x, QColor(150, 0, 150));
        drawGridCell(xc - y, yc + x, QColor(150, 0, 150));
        drawGridCell(xc + y, yc - x, QColor(150, 0, 150));
        drawGridCell(xc - y, yc - x, QColor(150, 0, 150));

        x++;
        if (d >= 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
    }

    lastAlgorithmTime = QString("Брезенхем (окр.): %1 мс").arg(algorithmTimer.elapsed());
    emit algorithmTimeUpdated(lastAlgorithmTime);
}

void RasterizationWidget::drawWuLine(const QPoint& start, const QPoint& end)
{
    algorithmTimer.start();

    const QColor col(100, 100, 200); // Синий цвет линии
    int x1 = start.x(), y1 = start.y();
    int x2 = end.x(), y2 = end.y();

    bool steep = abs(y2 - y1) > abs(x2 - x1);

    if (steep) {
        std::swap(x1, y1);
        std::swap(x2, y2);
    }

    if (x1 > x2) {
        std::swap(x1, x2);
        std::swap(y1, y2);
    }

    int dx = x2 - x1;
    int dy = y2 - y1;

    if (dx == 0) {
        // Вертикальная линия
        int minY = std::min(y1, y2);
        int maxY = std::max(y1, y2);
        for (int y = minY; y <= maxY; y++) {
            if (steep) {
                drawGridCell(y, x1, col);
            } else {
                drawGridCell(x1, y, col);
            }
        }
        return;
    }

    // Определяем направление по Y
    int y_step = (dy >= 0) ? 1 : -1;
    dy = abs(dy);

    // Параметры алгоритма Ву
    const int M = 16; // Количество оттенков серого
    const int N = 256; // Параметр дробления

    int d = round((float)dy * N / dx);
    int D = 0;

    // Первый и последний пиксели
    if (steep) {
        drawGridCell(y1, x1, col);
        drawGridCell(y2, x2, col);
    } else {
        drawGridCell(x1, y1, col);
        drawGridCell(x2, y2, col);
    }

    int xf = x1, xl = x2;
    int yf = y1, yl = y2;

    // Основной цикл алгоритма Ву
    while (xf < xl - 1) {
        xf++;
        xl--;
        D += d;

        if (D >= N) {
            D -= N;
            yf += y_step;
            yl -= y_step;
        }

        // Вычисляем интенсивности
        int alpha1 = (D * M) / N;
        int alpha2 = M - 1 - alpha1;

        // Смешиваем с белым фоном
        QColor backgroundColor(255, 255, 255); // Белый фон
        QColor color1 = blendColors(backgroundColor, col, alpha1 / float(M - 1));
        QColor color2 = blendColors(backgroundColor, col, alpha2 / float(M - 1));

        // Рисуем пиксели с учетом направления
        if (steep) {
            drawGridCell(yf, xf, color1);
            drawGridCell(yf + y_step, xf, color2);
            drawGridCell(yl, xl, color1);
            drawGridCell(yl - y_step, xl, color2);
        } else {
            drawGridCell(xf, yf, color1);
            drawGridCell(xf, yf + y_step, color2);
            drawGridCell(xl, yl, color1);
            drawGridCell(xl, yl - y_step, color2);
        }
    }

    // Обработка среднего пикселя для нечетного количества шагов
    if (xf == xl - 1) {
        xf++;
        D += d;

        if (D >= N) {
            D -= N;
            yf += y_step;
        }

        int alpha1 = (D * M) / N;
        int alpha2 = M - 1 - alpha1;

        QColor backgroundColor(255, 255, 255); // Белый фон
        QColor color1 = blendColors(backgroundColor, col, alpha1 / float(M - 1));
        QColor color2 = blendColors(backgroundColor, col, alpha2 / float(M - 1));

        if (steep) {
            drawGridCell(yf, xf, color1);
            drawGridCell(yf + y_step, xf, color2);
        } else {
            drawGridCell(xf, yf, color1);
            drawGridCell(xf, yf + y_step, color2);
        }
    }

    lastAlgorithmTime = QString("ВУ: %1 мс").arg(algorithmTimer.elapsed());
    emit algorithmTimeUpdated(lastAlgorithmTime);
}

void RasterizationWidget::drawCastlePitwayLine(const QPoint& start, const QPoint& end)
{
    algorithmTimer.start();

    int x1 = start.x(), y1 = start.y();
    int x2 = end.x(), y2 = end.y();
    QString ans = "";
    // Вычисляем разности
    int dx = x2 - x1;
    int dy = y2 - y1;

    // Определяем октант и нормализуем
    int sx = (dx > 0) ? 1 : -1;
    int sy = (dy > 0) ? 1 : -1;

    dx = abs(dx);
    dy = abs(dy);
    bool change = (dx < dy);

    // Специальные случаи
    if (dx == 0 && dy == 0) {
        // Точка
        drawGridCell(x1, y1, QColor(200, 0, 100));
        return;
    }

    if (dy == 0) {
        // Горизонтальная линия
        for (int i = 0; i <= dx; i++) {
            drawGridCell(x1 + i * sx, y1, QColor(200, 0, 100));
        }
        return;
    }

    if (dx == 0) {
        // Вертикальная линия
        for (int i = 0; i <= dy; i++) {
            drawGridCell(x1, y1 + i * sy, QColor(200, 0, 100));
        }
        return;
    }
    if (dx == dy) {
        // диагональная линия
        for (int i = 0; i <= dy; i++) {
            drawGridCell(x1 + i*sx, y1 + i * sy, QColor(200, 0, 100));
        }
        return;
    }

    if(change){
        std::swap(dx, dy);
    }

    // Инициализация как в описании
    int x = dx - dy;
    int y = dy;

    QString m1 = "s";  // Горизонтальный шаг
    QString m2 = "d";  //диагональ шаг
    QString m = "";

    // Основной цикл алгоритма Кастла-Питвея
    while (x != y) {
        if (x > y) {
            x = x - y;
            m = m2;
            std::reverse(m.begin(), m.end());
            m2 = m1 + m;
        } else {
            y = y - x;
            m = m1;
            std::reverse(m.begin(), m.end());
            m1 = m2 + m;
        }
    }

    // x-кратная последовательность m2 (+) ~m1 задает отрезок
    m = m1;
    std::reverse(m.begin(), m.end());
    QString rep = m2 + m;
    for(int i =0; i < x; ++i) ans += rep;


    // Воспроизводим последовательность для построения отрезка
    x1 = start.x();
    y1 = start.y();

    // Рисуем начальную точку
    drawGridCell(x1, y1, QColor(200, 0, 100));

    // Проходим по последовательности и строим отрезок
    for (int i = 0; i < ans.size(); i++) {
        QChar step = ans[i];
        if (step == 's') {  //  шаг по сетке
            if(change){
                y1 += sy;
            } else x1 += sx;
        } else if (step == 'd') {  // Горизонтальный шаг
            x1 += sx;
            y1 += sy;
        }
        drawGridCell(x1, y1, QColor(200, 0, 100));
    }

    lastAlgorithmTime = QString("Кастла-Питвея: %1 мс").arg(algorithmTimer.elapsed());
    emit algorithmTimeUpdated(lastAlgorithmTime);
}

QColor RasterizationWidget::blendColors(const QColor& background, const QColor& foreground, float intensity)
{
    // intensity = 0 -> полностью фон (белый)
    // intensity = 1 -> полностью цвет линии
    int r = static_cast<int>(background.red() * intensity + foreground.red() * (1 - intensity));
    int g = static_cast<int>(background.green() * intensity  + foreground.green()* (1 - intensity));
    int b = static_cast<int>(background.blue() * intensity + foreground.blue() * (1 - intensity));

    return QColor(r, g, b);
}

void RasterizationWidget::drawPixel(int x, int y, const QColor& color)
{
    QPointF screenPos = gridToScreen(QPoint(x, y));
    glColor3f(color.redF(), color.greenF(), color.blueF());
    glBegin(GL_POINTS);
    glVertex2f(screenPos.x(), screenPos.y());
    glEnd();
}

// Public methods implementation
void RasterizationWidget::setCurrentAlgorithm(Algorithm algo)
{
    currentAlgorithm = algo;
    firstClickDone = false;
    drawing = false;
    update();
}

void RasterizationWidget::setInputMode(InputMode mode)
{
    inputMode = mode;
    if (mode == KEYBOARD_INPUT) {
        drawing = false;
        firstClickDone = false;
    }
}

void RasterizationWidget::setLineStart(const QPoint& point)
{
    lineStart = point;
    update();
}

void RasterizationWidget::setLineEnd(const QPoint& point)
{
    lineEnd = point;
    update();
}

void RasterizationWidget::clearCurrentLine()
{
    lineStart = QPoint(0, 0);
    lineEnd = QPoint(0, 0);
    drawing = false;
    firstClickDone = false;
    update();
}

void RasterizationWidget::clearAllLines()
{
    drawnLines.clear();
    clearCurrentLine();
    update();
}

void RasterizationWidget::setShowGrid(bool show)
{
    showGrid = show;
    update();
}

void RasterizationWidget::resetView()
{
    scale = 1.0;
    centerView(); // Используем центрирование вместо offset = QPointF(0, 0)
    update();
}

void RasterizationWidget::centerView()
{
    // Устанавливаем (0, 0) в центр виджета
    offset = QPointF(width() / (2.0 * scale), height() / (2.0 * scale));
    update();
}

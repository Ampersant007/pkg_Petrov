#ifndef RASTERIZATIONWIDGET_H
#define RASTERIZATIONWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QColor>
#include <QPoint>
#include <QPointF>
#include <QPainter>
#include<QTime>
#include <QElapsedTimer>
#include <vector>
#include <cmath>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QString>

class RasterizationWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    enum Algorithm {
        STEP_BY_STEP,
        DDA,
        BRESENHAM_LINE,
        BRESENHAM_CIRCLE,
        CASTLE_PITWAY,
        WU_LINE
    };

    enum InputMode {
        CLICK_INPUT,    // Ввод точек кликом
        KEYBOARD_INPUT  // Ввод координат через поля
    };

    // Структура для хранения отрисованных линий
    struct DrawnLine {
        QPoint start;
        QPoint end;
        Algorithm algorithm;
    };

    explicit RasterizationWidget(QWidget *parent = nullptr);
    ~RasterizationWidget();

    // Управление алгоритмами
    void setCurrentAlgorithm(Algorithm algo);
    void setInputMode(InputMode mode);

    // Управление точками отрезка
    void setLineStart(const QPoint& point);
    void setLineEnd(const QPoint& point);
    void clearCurrentLine();
    void clearAllLines();

    // Управление видом
    void setShowGrid(bool show);
    void resetView();
    void centerView();

    // Получение состояния
    QPoint getLineStart() const { return lineStart; }
    QPoint getLineEnd() const { return lineEnd; }
    bool isDrawing() const { return drawing; }
    QPoint getCurrentMouseGridPos() const { return currentMouseGridPos; }

signals:
    // Сигналы для связи с MainWindow
    void lineFinished(const QPoint& start, const QPoint& end);
    void mouseMovedToGridPos(const QPoint& gridPos);
    void algorithmTimeUpdated(const QString& timeInfo); // Добавляем новый сигнал

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    // Обработка мыши
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    // Алгоритмы растеризации
    void drawStepByStepLine(const QPoint& start, const QPoint& end);
    void drawDDALine(const QPoint& start, const QPoint& end);
    void drawBresenhamLine(const QPoint& start, const QPoint& end);
    void drawBresenhamCircle(const QPoint& center, int radius);
    void drawWuLine(const QPoint& start, const QPoint& end);
    void drawCastlePitwayLine(const QPoint& start, const QPoint& end);

    // Вспомогательные функции
    void drawPixel(int x, int y, const QColor& color = Qt::black);
    void drawGridCell(int gridX, int gridY, const QColor& color = Qt::blue);
    QColor blendColors(const QColor& background, const QColor& foreground, float intensity);
    void drawGrid();
    void drawCoordinateSystem();
    void drawCoordinates();
    void drawAllLines();

    // Таймер для измерения времени выполнения алгоритмов
    QElapsedTimer algorithmTimer;
    QString lastAlgorithmTime;

    // Преобразования координат
    QPointF gridToScreen(const QPoint& gridPos) const;
    QPoint screenToGrid(const QPoint& screenPos) const;
    QPoint screenToGrid(const QPointF& screenPos) const;

    // Состояние
    Algorithm currentAlgorithm;
    InputMode inputMode;
    bool drawing;
    bool showGrid;
    int gridSize; // Фиксированный размер сетки

    // Точки отрезка
    QPoint lineStart;
    QPoint lineEnd;
    QPoint tempPoint;

    // Для отслеживания первого и второго клика
    bool firstClickDone;
    QPoint firstClickPoint;

    // Текущая позиция мыши в координатах сетки
    QPoint currentMouseGridPos;

    // Трансформации
    double scale;
    QPointF offset;
    QPoint lastMousePos;
    bool panning;

    // Хранение всех отрисованных линий
    std::vector<DrawnLine> drawnLines;
};

#endif // RASTERIZATIONWIDGET_H

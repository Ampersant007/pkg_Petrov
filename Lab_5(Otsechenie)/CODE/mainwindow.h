#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <vector>
#include "graphicsview.h"
#include "algorithms.h"

enum AlgorithmType {
    LIANG_BARSKY,
    CYRUS_BECK
    // WEILER_ATHERTON удалён
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void loadData();
    void loadPolygonPoints();
    void clipSegments();
    void showOriginal();
    void onAlgorithmChanged(int index);
    void onViewScaleChanged(double scale);
    void onClipParamsChanged();
    void onMouseMoved(const QPointF &scenePos);
    void onMouseClicked(const QPointF &scenePos);
    void clearScene();

private:
    void setupUI();
    void drawCoordinateSystem();
    void drawGrid();
    void drawSegments();
    void clearDynamicItems();
    void updateStatus();
    void updateClipVisualization();
    void resetScene();

    // Элементы сцены и отображения
    QGraphicsScene *scene;
    GraphicsView *view;

    // Элементы управления
    QGroupBox *controlGroup;
    QGroupBox *clipParamsGroup;
    QComboBox *algorithmComboBox;
    QPushButton *loadButton;
    QPushButton *loadPolygonButton;
    QPushButton *showOriginalButton;
    QPushButton *clipButton;
    QPushButton *clearButton;
    QLabel *statusLabel;
    QLabel *infoLabel;
    QLabel *fileFormatLabel;
    QLabel *mousePosLabel;

    // Спины для параметров отсечения
    QDoubleSpinBox *xMinSpin;
    QDoubleSpinBox *xMaxSpin;
    QDoubleSpinBox *yMinSpin;
    QDoubleSpinBox *yMaxSpin;

    // Данные
    std::vector<Segment> segments;
    Point clipMin, clipMax;
    std::vector<Point> clipPolygon;
    bool dataLoaded;
    double sceneScale;
    AlgorithmType currentAlgorithm;

    // Для интерактивного ввода отрезков
    Point tempPoint;
    bool hasTempPoint;
    QGraphicsEllipseItem *tempPointIndicator;

    // Для визуализации отсекателя
    QGraphicsRectItem *clipRectItem;
    std::vector<QGraphicsLineItem*> clipPolygonItems;

    // Элементы системы координат
    QGraphicsLineItem *xAxis;
    QGraphicsLineItem *yAxis;
    std::vector<QGraphicsItem*> gridItems;
    std::vector<QGraphicsItem*> axisLabels;
};

#endif // MAINWINDOW_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QSlider>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QSpinBox>
#include "RasterizationWidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onAlgorithmChanged(int index);
    void onShowGridChanged(int state);
    void onResetViewClicked();
    void onCenterViewClicked();
    void onClearClicked();
    void onApplyCoordinatesClicked();
    void onLineFinished(const QPoint& start, const QPoint& end);
    void onMouseMovedToGridPos(const QPoint& gridPos);
    void onAlgorithmTimeUpdated(const QString& timeInfo); // Добавляем новый слот

private:
    void setupUI();
    void updateCoordinateDisplay();

    RasterizationWidget *glWidget;

    // Элементы управления
    QComboBox *algorithmCombo;
    QCheckBox *showGridCheck;
    QPushButton *resetViewButton;
    QPushButton *centerViewButton;
    QPushButton *clearButton;
    QPushButton *applyCoordinatesButton;

    // Поля ввода координат
    QSpinBox *startXSpin;
    QSpinBox *startYSpin;
    QSpinBox *endXSpin;
    QSpinBox *endYSpin;

    // Информационные метки
    QLabel *infoLabel;
    QLabel *coordinatesLabel;
    QLabel *instructionLabel;
    QLabel *mousePosLabel;
    QLabel *viewInfoLabel;
    QLabel *timeInfoLabel; // Добавляем метку для времени
};

#endif // MAINWINDOW_H

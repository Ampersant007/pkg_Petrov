#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QSpinBox>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QGroupBox>
#include <QStatusBar>
#include <QFormLayout>
#include <QHBoxLayout>
#include "colormodel.h"
#include <QColorDialog>

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    ColorModel *m_colorModel;

    QSpinBox *m_rSpin;
    QSpinBox *m_gSpin;
    QSpinBox *m_bSpin;
    QSlider *m_rSlider;
    QSlider *m_gSlider;
    QSlider *m_bSlider;

    // HSV widgets
    QDoubleSpinBox *m_hSpin;
    QDoubleSpinBox *m_sSpin;
    QDoubleSpinBox *m_vSpin;
    QSlider *m_hSlider;
    QSlider *m_sSlider;
    QSlider *m_vSlider;

    // XYZ widgets
    QDoubleSpinBox *m_xSpin;
    QDoubleSpinBox *m_ySpin;
    QDoubleSpinBox *m_zSpin;
    QSlider *m_xSlider;
    QSlider *m_ySlider;
    QSlider *m_zSlider;

    // Other widgets
    QLabel *m_colorPreview;
    QPushButton *m_colorChooseButton;
    QStatusBar *m_statusBar;

    bool m_updating;

    QGroupBox* createRgbGroup();
    QGroupBox* createHsvGroup();
    QGroupBox* createXyzGroup();
    QGroupBox* createPreviewGroup();
    void setupConnections();
    void blockSignals(bool block);
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onRgbChanged(const RGB&);
    void onHsvChanged(const HSV&);
    void onXyzChanged(const XYZ&);
    void onColorChanged(const QColor &color);
    void onValidationError(const QString &message);

    void onRgbComponentChanged();
    void onHsvComponentChanged();
    void onXyzComponentChanged();
    void onColorChooseClicked();

    void onRoundingNotification(const QString &message);


};

#endif // MAINWINDOW_H

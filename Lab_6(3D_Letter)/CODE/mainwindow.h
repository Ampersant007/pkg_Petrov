#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMatrix4x4>
#include "openglwidget.h"

QT_BEGIN_NAMESPACE
class QSlider;
class QDoubleSpinBox;
class QPushButton;
class QRadioButton;
class QCheckBox;
class QTextEdit;
class QLabel;
class QGroupBox;
class QComboBox;
class QLineEdit;
class QSpinBox;
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onRotationAngleChanged(int value);
    void onScaleXChanged(double value);
    void onScaleYChanged(double value);
    void onScaleZChanged(double value);
    void onTranslationXChanged(double value);
    void onTranslationYChanged(double value);
    void onTranslationZChanged(double value);

    void onAxisPointXChanged(double value);
    void onAxisPointYChanged(double value);
    void onAxisPointZChanged(double value);
    void onAxisDirectionXChanged(double value);
    void onAxisDirectionYChanged(double value);
    void onAxisDirectionZChanged(double value);

    void onSetAxisClicked();
    void onResetAxisClicked();
    void onResetRotationClicked();

    void onProjectionChanged();
    void onResetViewClicked();
    void onResetTransformClicked();

    void onShowGridChanged(int state);
    void onShowAxesChanged(int state);
    void onShowRotationAxisChanged(int state);

    void onCameraChanged(float pitch, float yaw, float zoom);
    void onRotationAngleChangedFromWidget(float angle);
    void onRotationAxisChangedFromWidget(const QVector3D &point, const QVector3D &direction);

    void updateTransformMatrix();
    void updateMatrixDisplay();

private:
    void createUI();
    void setupConnections();
    void updateAxisControlsFromWidget(const QVector3D &point, const QVector3D &direction);
    void resetRotationSlider();

    OpenGLWidget *m_glWidget;

    // Элементы для вращения
    QSlider *m_rotationAngleSlider;
    QLabel *m_rotationAngleLabel;

    // Элементы для масштабирования
    QDoubleSpinBox *m_scaleXSpinBox;
    QDoubleSpinBox *m_scaleYSpinBox;
    QDoubleSpinBox *m_scaleZSpinBox;

    // Элементы для переноса
    QDoubleSpinBox *m_translationXSpinBox;
    QDoubleSpinBox *m_translationYSpinBox;
    QDoubleSpinBox *m_translationZSpinBox;

    // Элементы для оси вращения
    QDoubleSpinBox *m_axisPointXSpinBox;
    QDoubleSpinBox *m_axisPointYSpinBox;
    QDoubleSpinBox *m_axisPointZSpinBox;
    QDoubleSpinBox *m_axisDirectionXSpinBox;
    QDoubleSpinBox *m_axisDirectionYSpinBox;
    QDoubleSpinBox *m_axisDirectionZSpinBox;

    QPushButton *m_setAxisButton;
    QPushButton *m_resetAxisButton;
    QPushButton *m_resetRotationButton;

    // Проекции
    QRadioButton *m_perspectiveRadio;
    QRadioButton *m_orthoXYRadio;
    QRadioButton *m_orthoXZRadio;
    QRadioButton *m_orthoYZRadio;

    // Кнопки сброса
    QPushButton *m_resetViewButton;
    QPushButton *m_resetTransformButton;

    // Отображение
    QCheckBox *m_showGridCheckBox;
    QCheckBox *m_showAxesCheckBox;
    QCheckBox *m_showRotationAxisCheckBox;

    // Матрица
    QTextEdit *m_matrixTextEdit;

    // Параметры
    float m_rotationAngle;
    float m_scaleX;
    float m_scaleY;
    float m_scaleZ;
    float m_translationX;
    float m_translationY;
    float m_translationZ;

    QVector3D m_axisPoint;
    QVector3D m_axisDirection;

    QMatrix4x4 m_currentTransform;
};

#endif // MAINWINDOW_H

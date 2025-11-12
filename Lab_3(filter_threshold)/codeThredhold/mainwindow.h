#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QProgressBar>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QResizeEvent>
#include <QApplication>
#include <QTimer>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void loadImage();
    void applyThreshold();
    void saveImage();
    void updateImageDisplay();
    void onAlgorithmChanged(int index);

private:
    // Алгоритмы обработки
    QImage applyAdaptiveThreshold(const QImage &input, int K, double alpha);
    QImage applyGlobalHistogramThreshold(const QImage &input, double epsilon);
    QImage applyGlobalGradientThreshold(const QImage &input);

    // Вспомогательные функции
    double calculateGlobalThresholdHistogram(const QImage &input, double epsilon);
    double calculateGlobalThresholdGradient(const QImage &input);

    QImage originalImage;
    QImage processedImage;

    QLabel *originalLabel;
    QLabel *processedLabel;
    QPushButton *loadButton;
    QPushButton *applyButton;
    QPushButton *saveButton;  // Новая кнопка сохранения

    QComboBox *algorithmComboBox;
    QSpinBox *windowSizeSpinBox;
    QDoubleSpinBox *alphaSpinBox;
    QDoubleSpinBox *epsilonSpinBox;
    QSlider *alphaSlider;
    QProgressBar *progressBar;

    QLabel *infoLabel;
};

#endif // MAINWINDOW_H

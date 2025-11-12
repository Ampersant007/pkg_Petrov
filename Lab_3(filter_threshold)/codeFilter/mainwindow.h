#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QResizeEvent>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void loadImage();
    void applyFilter();
    void saveImage();
    void updateImageDisplay();
    void onFilterChanged(int index);

private:
    QImage applyFilterWithKernel(const QImage &input);

    QImage originalImage;
    QImage processedImage;

    QLabel *originalLabel;
    QLabel *processedLabel;
    QPushButton *loadButton;
    QPushButton *applyButton;
    QPushButton *saveButton;  // Новая кнопка сохранения
    QComboBox *filterComboBox;
    QLabel *infoLabel;

    // Доступные фильтры
    enum FilterType {
        LAPLACIAN_BASIC,
        LAPLACIAN_SHARP,
        STRONG_SHARP,
        CUSTOM_SHARP,
        SOBEL_X,
        SOBEL_Y
    };

    QVector<QVector<QVector<double>>> kernels;
    QVector<QString> filterNames;
    QVector<QString> filterDescriptions;
};

#endif // MAINWINDOW_H

#include "mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Фильтры повышения резкости");
    setMinimumSize(800, 600);

    // Инициализация фильтров
    kernels = {
        // Лапласиан (базовый)
        {
            {0, -1, 0},
            {-1, 5, -1},
            {0, -1, 0}
        },
        // Сильное повышение резкости
        {
            {-1, -1, -1},
            {-1, 9, -1},
            {-1, -1, -1}
        },
        // Пользовательский фильтр
        {
            {1, -2, 1},
            {-2, 5, -2},
            {1, -2, 1}
        },
        // Лапласиан гауссиана
        {
            {0, 0, -1, 0, 0},
            {0, -1, -2, -1, 0},
            {-1, -2, 17, -2, -1},
            {0, -1, -2, -1, 0},
            {0, 0, -1, 0, 0}
        }
    };

    filterNames = {
        "Лапласиан (базовый)",
        "Сильное повышение резкости",
        "Крестообразный фильтр",
        "Лапласиан гауссиана LoG"
    };

    filterDescriptions = {
                          "Лапласиан (базовый):\n[ 0, -1,  0]\n[-1,  5, -1]\n[ 0, -1,  0]\n\nВыделяет границы объектов",
                          "Сильное повышение резкости:\n[-1, -1, -1]\n[-1,  9, -1]\n[-1, -1, -1]\n\nАгрессивное повышение резкости",
                          "Крестообразный фильтр:\n[ 1, -2,  1]\n[-2,  5, -2]\n[ 1, -2,  1]\n\nАльтернативный метод повышения резкости",
                          "Лапласиан гауссиана Log:\n[ 0,  0, -1,  0,  0]\n[ 0, -1, -2, -1,  0]\n[-1, -2, 16, -2, -1]\n[ 0, -1, -2, -1,  0]\n\
[ 0,  0, -1,  0,  0]\n\nОдновременное сглаживание и выделение границ"
    };

    // Создание виджетов
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    originalLabel = new QLabel("Исходное изображение");
    processedLabel = new QLabel("Обработанное изображение");
    originalLabel->setAlignment(Qt::AlignCenter);
    processedLabel->setAlignment(Qt::AlignCenter);
    originalLabel->setFrameStyle(QFrame::Box);
    processedLabel->setFrameStyle(QFrame::Box);
    originalLabel->setMinimumSize(300, 300);
    processedLabel->setMinimumSize(300, 300);

    // Устанавливаем политику размера для автоматического растягивания
    originalLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    processedLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    loadButton = new QPushButton("Загрузить изображение");
    applyButton = new QPushButton("Применить фильтр");
    saveButton = new QPushButton("Сохранить результат");
    saveButton->setEnabled(false); // Изначально отключена

    filterComboBox = new QComboBox();
    for (const QString &name : filterNames) {
        filterComboBox->addItem(name);
    }

    infoLabel = new QLabel(filterDescriptions[0]);
    infoLabel->setAlignment(Qt::AlignCenter);
    infoLabel->setWordWrap(true);

    // Соединение сигналов и слотов
    connect(loadButton, &QPushButton::clicked, this, &MainWindow::loadImage);
    connect(applyButton, &QPushButton::clicked, this, &MainWindow::applyFilter);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveImage);
    connect(filterComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onFilterChanged);

    // Layout
    QHBoxLayout *imageLayout = new QHBoxLayout();
    imageLayout->addWidget(originalLabel);
    imageLayout->addWidget(processedLabel);

    QHBoxLayout *controlLayout = new QHBoxLayout();
    controlLayout->addWidget(new QLabel("Фильтр:"));
    controlLayout->addWidget(filterComboBox);
    controlLayout->addWidget(loadButton);
    controlLayout->addWidget(applyButton);
    controlLayout->addWidget(saveButton); // Добавляем кнопку сохранения

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addLayout(imageLayout, 3); // 3 - больший вес для области с изображениями
    mainLayout->addLayout(controlLayout);
    mainLayout->addWidget(infoLabel);

    centralWidget->setLayout(mainLayout);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    updateImageDisplay();
}

void MainWindow::loadImage()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Загрузить изображение", "",
                                                    "Images (*.png *.jpg *.jpeg *.bmp *.tiff *.gif)");

    if (!fileName.isEmpty()) {
        originalImage.load(fileName);
        if (originalImage.isNull()) {
            QMessageBox::warning(this, "Ошибка", "Не удалось загрузить изображение");
            return;
        }

        // Отключаем кнопку сохранения при загрузке нового изображения
        saveButton->setEnabled(false);

        updateImageDisplay();
    }
}

void MainWindow::applyFilter()
{
    if (originalImage.isNull()) {
        QMessageBox::warning(this, "Предупреждение", "Сначала загрузите изображение");
        return;
    }

    processedImage = applyFilterWithKernel(originalImage);
    updateImageDisplay();

    // Включаем кнопку сохранения после применения фильтра
    saveButton->setEnabled(true);
}

void MainWindow::saveImage()
{
    if (processedImage.isNull()) {
        QMessageBox::warning(this, "Предупреждение", "Нет обработанного изображения для сохранения");
        return;
    }

    // Диалог выбора файла для сохранения
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Сохранить обработанное изображение",
                                                    "filtered_image",
                                                    "Images (*.png *.jpg *.jpeg *.bmp *.tiff)");

    if (!fileName.isEmpty()) {
        // Определяем формат по расширению файла
        QString extension = QFileInfo(fileName).suffix().toLower();
        const char* format = "PNG"; // формат по умолчанию

        if (extension == "jpg" || extension == "jpeg") {
            format = "JPEG";
        } else if (extension == "bmp") {
            format = "BMP";
        } else if (extension == "tiff" || extension == "tif") {
            format = "TIFF";
        } else {
            format = "PNG";
            // Если расширение не указано, добавляем .png
            if (extension.isEmpty()) {
                fileName += ".png";
            }
        }

        // Сохраняем изображение
        if (!processedImage.save(fileName, format)) {
            QMessageBox::warning(this, "Ошибка", "Не удалось сохранить изображение");
        }
    }
}

void MainWindow::onFilterChanged(int index)
{
    if (index >= 0 && index < filterDescriptions.size()) {
        infoLabel->setText(filterDescriptions[index]);
    }
}

void MainWindow::updateImageDisplay()
{
    // Обновляем отображение исходного изображения
    if (!originalImage.isNull()) {
        QPixmap originalPixmap = QPixmap::fromImage(originalImage);
        originalPixmap = originalPixmap.scaled(originalLabel->size(),
                                               Qt::KeepAspectRatio,
                                               Qt::SmoothTransformation);
        originalLabel->setPixmap(originalPixmap);
    }

    // Обновляем отображение обработанного изображения
    if (!processedImage.isNull()) {
        QPixmap processedPixmap = QPixmap::fromImage(processedImage);
        processedPixmap = processedPixmap.scaled(processedLabel->size(),
                                                 Qt::KeepAspectRatio,
                                                 Qt::SmoothTransformation);
        processedLabel->setPixmap(processedPixmap);
    }
}

QImage MainWindow::applyFilterWithKernel(const QImage &input)
{
    int currentFilter = filterComboBox->currentIndex();
    if (currentFilter < 0 || currentFilter >= kernels.size()) {
        return input;
    }

    const QVector<QVector<double>> &kernel = kernels[currentFilter];
    QImage output = input.convertToFormat(QImage::Format_RGB32);
    int width = input.width();
    int height = input.height();

    int kSize = kernel.size();
    int kHalf = kSize / 2;

    // Проходим по всем пикселям, кроме граничных
    for (int y = kHalf; y < height - kHalf; ++y) {
        for (int x = kHalf; x < width - kHalf; ++x) {
            int r = 0, g = 0, b = 0;

            // Применяем свертку с выбранным ядром
            for (int j = -kHalf; j <= kHalf; ++j) {
                for (int i = -kHalf; i <= kHalf; ++i) {
                    QColor color = input.pixelColor(x + i, y + j);
                    int weight = kernel[j + kHalf][i + kHalf];
                    r += color.red() * weight;
                    g += color.green() * weight;
                    b += color.blue() * weight;
                }
            }

            output.setPixel(x, y, qRgb(
                                      qBound(0, r, 255),
                                      qBound(0, g, 255),
                                      qBound(0, b, 255)
                                      ));
        }
    }

    return output;
}

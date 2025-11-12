#include "mainwindow.h"
#include <QMessageBox>
#include <algorithm>
#include <vector>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Пороговая обработка изображений");
    setMinimumSize(800, 600);

    // Создание виджетов
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    originalLabel = new QLabel("Исходное изображение");
    processedLabel = new QLabel("Бинаризованное изображение");
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
    applyButton = new QPushButton("Применить порог");
    saveButton = new QPushButton("Сохранить результат");  // Новая кнопка
    saveButton->setEnabled(false);  // Изначально отключена

    // Выбор алгоритма
    algorithmComboBox = new QComboBox();
    algorithmComboBox->addItem("Адаптивная обработка (8 ориентиров)");
    algorithmComboBox->addItem("Глобальная обработка (гистограмма)");
    algorithmComboBox->addItem("Глобальная обработка (градиент)");

    // Параметры обработки
    windowSizeSpinBox = new QSpinBox();
    windowSizeSpinBox->setRange(3, 11);
    windowSizeSpinBox->setValue(7);
    windowSizeSpinBox->setSingleStep(2);
    windowSizeSpinBox->setSuffix(" px");

    alphaSpinBox = new QDoubleSpinBox();
    alphaSpinBox->setRange(0.1, 1.00);
    alphaSpinBox->setValue(0.67);
    alphaSpinBox->setSingleStep(0.1);
    alphaSpinBox->setDecimals(2);

    epsilonSpinBox = new QDoubleSpinBox();
    epsilonSpinBox->setRange(0.1, 10.0);
    epsilonSpinBox->setValue(1.0);
    epsilonSpinBox->setSingleStep(0.1);
    epsilonSpinBox->setDecimals(2);
    epsilonSpinBox->setSuffix(" ε");
    epsilonSpinBox->setVisible(false);

    alphaSlider = new QSlider(Qt::Horizontal);
    alphaSlider->setRange(10, 100);
    alphaSlider->setValue(67);

    // Progress Bar - скрытый по умолчанию
    progressBar = new QProgressBar();
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setTextVisible(true);
    progressBar->setFormat("%p%");
    progressBar->setVisible(false);

    infoLabel = new QLabel(
        "Адаптивная пороговая обработка с 8 ориентирами:\n\n"
        "• Размер окна: область для вычисления статистики вокруг каждого ориентира\n"
        "• Коэффициент α: регулирует чувствительность порога"
        );
    infoLabel->setAlignment(Qt::AlignLeft);
    infoLabel->setWordWrap(true);

    // Соединение сигналов и слотов
    connect(loadButton, &QPushButton::clicked, this, &MainWindow::loadImage);
    connect(applyButton, &QPushButton::clicked, this, &MainWindow::applyThreshold);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveImage);  // Новое соединение
    connect(algorithmComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onAlgorithmChanged);

    connect(alphaSlider, &QSlider::valueChanged, this, [this](int value) {
        alphaSpinBox->setValue(value / 100.0);
    });

    connect(alphaSpinBox, &QDoubleSpinBox::valueChanged, this, [this](double value) {
        alphaSlider->setValue(static_cast<int>(value * 100));
    });

    // Layout
    QHBoxLayout *imageLayout = new QHBoxLayout();
    imageLayout->addWidget(originalLabel);
    imageLayout->addWidget(processedLabel);

    QHBoxLayout *algorithmLayout = new QHBoxLayout();
    algorithmLayout->addWidget(new QLabel("Алгоритм:"));
    algorithmLayout->addWidget(algorithmComboBox);

    QHBoxLayout *paramLayout = new QHBoxLayout();
    paramLayout->addWidget(new QLabel("Размер окна:"));
    paramLayout->addWidget(windowSizeSpinBox);
    paramLayout->addWidget(new QLabel("Коэффициент α:"));
    paramLayout->addWidget(alphaSpinBox);
    paramLayout->addWidget(alphaSlider);
    paramLayout->addWidget(new QLabel("Точность ε:"));
    paramLayout->addWidget(epsilonSpinBox);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(loadButton);
    buttonLayout->addWidget(applyButton);
    buttonLayout->addWidget(saveButton);  // Добавляем кнопку сохранения

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addLayout(imageLayout, 3);
    mainLayout->addLayout(algorithmLayout);
    mainLayout->addLayout(paramLayout);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(progressBar);
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
                                                    "Images (*.png *.jpg *.jpeg *.bmp)");

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

void MainWindow::saveImage()
{
    if (processedImage.isNull()) {
        QMessageBox::warning(this, "Предупреждение", "Нет обработанного изображения для сохранения");
        return;
    }

    // Диалог выбора файла для сохранения
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Сохранить обработанное изображение",
                                                    "",
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
        if (!processedImage.save(fileName, format)){
            QMessageBox::warning(this, "Ошибка", "Не удалось сохранить изображение");
        }
    }
}

void MainWindow::applyThreshold()
{
    if (originalImage.isNull()) {
        QMessageBox::warning(this, "Предупреждение", "Сначала загрузите изображение");
        return;
    }

    // Показываем ProgressBar перед началом обработки
    progressBar->setVisible(true);
    progressBar->setValue(0);

    int algorithmIndex = algorithmComboBox->currentIndex();

    switch (algorithmIndex) {
    case 0: // Адаптивная обработка
    {
        int K = (windowSizeSpinBox->value() - 1) / 2;
        double alpha = alphaSpinBox->value();
        processedImage = applyAdaptiveThreshold(originalImage, K, alpha);
        break;
    }
    case 1: // Глобальная гистограмма
    {
        double epsilon = epsilonSpinBox->value();
        processedImage = applyGlobalHistogramThreshold(originalImage, epsilon);
        break;
    }
    case 2: // Глобальный градиент
    {
        processedImage = applyGlobalGradientThreshold(originalImage);
        break;
    }
    }

    updateImageDisplay();

    // Завершаем прогресс и скрываем ProgressBar
    progressBar->setValue(100);
    QTimer::singleShot(500, this, [this]() {
        progressBar->setVisible(false);
    });

    // Включаем кнопку сохранения после успешной обработки
    saveButton->setEnabled(true);

    // Завершаем прогресс и скрываем ProgressBar
    progressBar->setValue(100);
    QTimer::singleShot(500, this, [this]() {
        progressBar->setVisible(false);
    });
}

void MainWindow::onAlgorithmChanged(int index)
{
    // Обновляем описание и видимость параметров в зависимости от выбранного алгоритма
    switch (index) {
    case 0: // Адаптивная обработка
        infoLabel->setText(
            "Адаптивная пороговая обработка с 8 ориентирами:\n\n"
            "• Размер окна: область для вычисления статистики вокруг каждого ориентира\n"
            "• Коэффициент α: регулирует чувствительность порога\n"
            "• Радиус фиксированный = 1 пиксель\n"
            "• Алгоритм выделяет пиксели, отличающиеся от соседних областей"
            );
        windowSizeSpinBox->setVisible(true);
        alphaSpinBox->setVisible(true);
        alphaSlider->setVisible(true);
        epsilonSpinBox->setVisible(false);
        break;

    case 1: // Глобальная гистограмма
        infoLabel->setText(
            "Глобальная пороговая обработка (метод гистограммы):\n\n"
            "• Итеративный алгоритм на основе гистограммы яркостей\n"
            "• Точность ε: критерий остановки итераций\n"
            "• Вычисляет один порог для всего изображения\n"
            "• Эффективен для изображений с бимодальной гистограммой"
            );
        windowSizeSpinBox->setVisible(false);
        alphaSpinBox->setVisible(false);
        alphaSlider->setVisible(false);
        epsilonSpinBox->setVisible(true);
        break;

    case 2: // Глобальный градиент
        infoLabel->setText(
            "Глобальная пороговая обработка (метод градиента):\n\n"
            "• Использует информацию о градиентах изображения\n"
            "• Взвешенный порог по величине градиента\n"
            "• Эффективен для выделения границ объектов\n"
            "• Автоматический расчет порога"
            );
        windowSizeSpinBox->setVisible(false);
        alphaSpinBox->setVisible(false);
        alphaSlider->setVisible(false);
        epsilonSpinBox->setVisible(false);
        break;
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

// ==================== АДАПТИВНАЯ ОБРАБОТКА ====================
QImage MainWindow::applyAdaptiveThreshold(const QImage &input, int K, double alpha)
{
    QImage output(input.size(), QImage::Format_Mono);
    output.fill(0);

    int width = input.width();
    int height = input.height();

    QImage grayImage = input.convertToFormat(QImage::Format_Grayscale8);

    // 8 направлений для ориентиров
    QVector<QPoint> directions = {
        QPoint(-1, 0), QPoint(-1, -1), QPoint(0, -1), QPoint(1, -1),
        QPoint(1, 0), QPoint(1, 1), QPoint(0, 1), QPoint(-1, 1)
    };

    int totalPixels = (height - 2*(K + 1)) * (width - 2*(K + 1));
    int processedPixels = 0;
    int lastProgress = 0;

    for (int y = K + 1; y < height - K - 1; ++y) {
        for (int x = K + 1; x < width - K - 1; ++x) {

            int current_pixel = qGray(grayImage.pixel(x, y));
            bool is_foreground = false;

            int f_max = 0, f_min = 255;
            double sum = 0;

            for (int j = -K; j <= K; ++j) {
                for (int i = -K; i <= K; ++i) {
                    int pixel_value = qGray(grayImage.pixel(x + i, y + j));
                    f_max = qMax(f_max, pixel_value);
                    f_min = qMin(f_min, pixel_value);
                    sum += pixel_value;
                }
            }

            double P_hat = sum / ((2*K+1) * (2*K+1));
            double dfmax = std::fabs(f_max - P_hat);
            double dfmin = std::fabs(f_min - P_hat);
            double t;

            if (dfmax > dfmin) {
                t = alpha * (2.0/3 * f_min + 1.0/3 * P_hat);
            } else if (dfmax < dfmin) {
                t = alpha * (1.0/3 * f_min + 2.0/3 * P_hat);
            } else {
                t = alpha * P_hat;
            }

            for (int l = 0; l < 8; ++l) {
                QPoint dir = directions[l];
                int px = x + dir.x();
                int py = y + dir.y();

                double sum_local = 0;
                for (int j = -K; j <= K; ++j) {
                    for (int i = -K; i <= K; ++i) {
                        sum_local += qGray(grayImage.pixel(px + i, py + j));
                    }
                }
                double P_hat_local = sum_local / ((2*K + 1) * (2*K + 1));

                if (std::fabs(P_hat_local - current_pixel) > t) {
                    is_foreground = true;
                    break;
                }
            }

            output.setPixel(x, y, is_foreground ? 0 : 1);

            // Обновление прогресса
            processedPixels++;
            int currentProgress = (processedPixels * 100) / totalPixels;
            if (currentProgress > lastProgress) {
                progressBar->setValue(currentProgress);
                lastProgress = currentProgress;
                QApplication::processEvents();
            }
        }
    }
    return output;
}

// ==================== ГЛОБАЛЬНАЯ ГИСТОГРАММА ====================
double MainWindow::calculateGlobalThresholdHistogram(const QImage &input, double epsilon)
{
    QImage grayImage = input.convertToFormat(QImage::Format_Grayscale8);
    int width = grayImage.width();
    int height = grayImage.height();

    // Начальная оценка порога - средняя яркость
    double t = 128.0;
    double t_prev;

    do {
        t_prev = t;

        // Шаг 2: Сегментация на две группы
        double sum1 = 0, sum2 = 0;
        int count1 = 0, count2 = 0;

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int pixel = qGray(grayImage.pixel(x, y));
                if (pixel > t) {
                    sum1 += pixel;
                    count1++;
                } else {
                    sum2 += pixel;
                    count2++;
                }
            }
        }

        // Шаг 3-4: Вычисление новых средних и порога
        double mu1 = (count1 > 0) ? sum1 / count1 : 0;
        double mu2 = (count2 > 0) ? sum2 / count2 : 0;
        t = (mu1 + mu2) / 2.0;

    } while (std::fabs(t - t_prev) > epsilon);

    return t;
}

QImage MainWindow::applyGlobalHistogramThreshold(const QImage &input, double epsilon)
{
    QImage output(input.size(), QImage::Format_Mono);

    // Вычисляем глобальный порог
    double threshold = calculateGlobalThresholdHistogram(input, epsilon);

    // Применяем порог ко всему изображению
    QImage grayImage = input.convertToFormat(QImage::Format_Grayscale8);
    int width = grayImage.width();
    int height = grayImage.height();

    progressBar->setValue(50);
    QApplication::processEvents();

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int pixel = qGray(grayImage.pixel(x, y));
            output.setPixel(x, y, (pixel > threshold) ? 1 : 0);
        }
    }

    return output;
}

// ==================== ГЛОБАЛЬНЫЙ ГРАДИЕНТ ====================
double MainWindow::calculateGlobalThresholdGradient(const QImage &input)
{
    QImage grayImage = input.convertToFormat(QImage::Format_Grayscale8);
    int width = grayImage.width();
    int height = grayImage.height();

    double sum_weighted = 0.0;
    double sum_gradient = 0.0;

    for (int y = 0; y < height - 1; ++y) {
        for (int x = 0; x < width - 1; ++x) {
            // Вычисление градиентов
            int Gx = qGray(grayImage.pixel(x + 1, y)) - qGray(grayImage.pixel(x, y));
            int Gy = qGray(grayImage.pixel(x, y + 1)) - qGray(grayImage.pixel(x, y));

            // Модуль градиента (максимум из |Gx| и |Gy|)
            int G = std::max(std::abs(Gx), std::abs(Gy));

            int pixel_value = qGray(grayImage.pixel(x, y));

            sum_weighted += pixel_value * G;
            sum_gradient += G;
        }

        // Обновление прогресса
        if (y % 10 == 0) {
            int progress = (y * 50) / height;
            progressBar->setValue(progress);
            QApplication::processEvents();
        }
    }

    // Вычисление порога по формуле
    return (sum_gradient > 0) ? (sum_weighted / sum_gradient) : 0.0;
}

QImage MainWindow::applyGlobalGradientThreshold(const QImage &input)
{
    QImage output(input.size(), QImage::Format_Mono);

    // Вычисляем глобальный порог по градиенту
    double threshold = calculateGlobalThresholdGradient(input);

    // Применяем порог ко всему изображению
    QImage grayImage = input.convertToFormat(QImage::Format_Grayscale8);
    int width = grayImage.width();
    int height = grayImage.height();

    progressBar->setValue(75);
    QApplication::processEvents();

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int pixel = qGray(grayImage.pixel(x, y));
            output.setPixel(x, y, (pixel > threshold) ? 1 : 0);
        }
    }

    return output;
}

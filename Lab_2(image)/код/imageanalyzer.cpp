#include "imageanalyzer.h"
#include <QHeaderView>
#include <algorithm>

ImageAnalyzer::ImageAnalyzer(QWidget *parent)
    : QMainWindow(parent)
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Панель управления
    QHBoxLayout *controlLayout = new QHBoxLayout();

    selectFolderButton = new QPushButton("Выбрать папку...", this);
    selectFilesButton = new QPushButton("Выбрать файлы...", this);
    clearButton = new QPushButton("Очистить список", this);
    analyzeButton = new QPushButton("Анализировать", this);
    analyzeButton->setEnabled(false);

    folderPath = new QLineEdit(this);
    folderPath->setReadOnly(true);
    folderPath->setPlaceholderText("Выберите папку или файлы с изображениями...");

    controlLayout->addWidget(new QLabel("Файлы:"));
    controlLayout->addWidget(folderPath, 2);
    controlLayout->addWidget(selectFolderButton);
    controlLayout->addWidget(selectFilesButton);
    controlLayout->addWidget(clearButton);
    controlLayout->addWidget(analyzeButton);

    // Splitter для разделения списка файлов и таблицы
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);

    // Список файлов
    fileList = new QListWidget(this);
    fileList->setMaximumWidth(300);

    // Таблица результатов
    table = new QTableWidget(this);
    table->setColumnCount(6);
    table->setHorizontalHeaderLabels(QStringList()
                                     << "Имя файла" << "Размер (px)" << "Разрешение (DPI)"
                                     << "Глубина цвета" << "Сжатие" << "Дополнительная информация");

    // Устанавливаем tooltip'ы для заголовков столбцов
    table->horizontalHeaderItem(1)->setToolTip(
        "<b>Размер изображения</b><br>"
        "<p>Ширина и высота изображения<br>"
        "в пикселях.</p>"
        );

    table->horizontalHeaderItem(2)->setToolTip(
        "<b>Разрешение (DPI)</b><p>"
        "Единица измерения разрешения изображения,<br>"
        "указывающая, сколько точек (пикселей)<br>"
        "будет напечатано на одном дюйме.<br>"
        "Чем выше значение DPI, тем более<br>"
        "детализированным и четким будет<br>"
        "распечатанное изображение.</p>"
        );

    table->horizontalHeaderItem(3)->setToolTip(
        "<b>Глубина цвета</b><p>"
        "Количество бит, используемых для<br>"
        "представления цвета каждого пикселя<br>"
        "в изображении.<br>"
        "Чем выше глубина цвета, тем больше<br>"
        "уникальных цветов может отобразить<br>"
        "изображение или монитор.</p>"
        );

    table->horizontalHeaderItem(4)->setToolTip(
        "Алгоритмы, которыми происходит сжатие<br>"
        "(Уменьшение размера файла)<br>"
        "• <b>Без потерь</b> - PNG, GIF, PCX<br>"
        "• <b>С потерями</b> - JPEG<br>"
        "• <b>Без сжатия</b> - BMP"
        );

    table->horizontalHeaderItem(5)->setToolTip(
        "<b>Размер файла</b><p>"
        "Количество КБ, которое занимает<br>"
        "файл на диске.</p><p>"
        "<b>Поддержка прозрачности</b></p><p>"
        "Возможность просмотра объектов<br>"
        "сквозь другие объекты</p>"
        "<p><b>Цветовая модель</b></p>"
        "<p>Способ представления цветов<br>"
        "в виде числовых значений:<br>"
        "• <b>цветное</b><br>"
        "• <b>чёрно-белое</b><br></p>"
        );

    // Настройка внешнего вида таблицы
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setWordWrap(true);
    table->setTextElideMode(Qt::ElideNone);
    table->setAlternatingRowColors(true);

    // ВАЖНО: Отключаем встроенную сортировку Qt
    table->setSortingEnabled(false);

    // Установка начальных размеров колонок
    table->setColumnWidth(0, 150);
    table->setColumnWidth(1, 120);
    table->setColumnWidth(2, 120);
    table->setColumnWidth(3, 120);
    table->setColumnWidth(4, 120);

    splitter->addWidget(fileList);
    splitter->addWidget(table);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);

    // Прогресс-бар
    progressBar = new QProgressBar(this);
    progressBar->setVisible(false);

    // Статус
    statusLabel = new QLabel("Готов к работе", this);

    mainLayout->addLayout(controlLayout);
    mainLayout->addWidget(splitter, 1);
    mainLayout->addWidget(progressBar);
    mainLayout->addWidget(statusLabel);

    // Подключение сигналов
    connect(selectFolderButton, &QPushButton::clicked, this, &ImageAnalyzer::selectFolder);
    connect(selectFilesButton, &QPushButton::clicked, this, &ImageAnalyzer::selectFiles);
    connect(clearButton, &QPushButton::clicked, this, &ImageAnalyzer::clearFiles);
    connect(analyzeButton, &QPushButton::clicked, this, &ImageAnalyzer::analyzeImages);
    connect(table->horizontalHeader(), &QHeaderView::sectionClicked, this, &ImageAnalyzer::sortTable);

    setWindowTitle("Анализатор графических файлов");
    setMinimumSize(1200, 700);
}

ImageAnalyzer::~ImageAnalyzer() {}

// Функция для парсинга размера "ширина x высота"
int ImageAnalyzer::parseSize(const QString sizeText) const{
    QStringList l = sizeText.split(" x ");
    if(l.size() == 2) return l[0].toInt() * l[1].toInt();
    else return 0;
}

void ImageAnalyzer::sortTable(int column)
{
    static int lastSortedColumn = -1;
    static Qt::SortOrder currentSortOrder = Qt::AscendingOrder;

    // Очищаем индикаторы у всех столбцов
    for (int i = 0; i < table->columnCount(); ++i) {
        QString headerText = table->horizontalHeaderItem(i)->text();
        // Убираем индикаторы (▲ ▼) если они есть
        if (headerText.endsWith(" ▲") || headerText.endsWith(" ▼")) {
            headerText = headerText.left(headerText.length() - 2);
            table->horizontalHeaderItem(i)->setText(headerText);
        }
    }

    // Определяем порядок сортировки
    if (column == lastSortedColumn) {
        currentSortOrder = (currentSortOrder == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;
    } else {
        currentSortOrder = Qt::AscendingOrder;
        lastSortedColumn = column;
    }

    // Создаем вектор пар: значение для сортировки + указатель на строку
    QVector<QPair<QString, int>> stringSortData;    // для текстовой сортировки
    QVector<QPair<int, int>> intSortData;           // для числовой сортировки

    bool useNumericSort = (column == 1 || column == 2 || column == 3); // Столбцы с числами

    // Собираем данные для сортировки
    for (int row = 0; row < table->rowCount(); ++row) {
        QTableWidgetItem* item = table->item(row, column);
        if (item) {
            if (useNumericSort) {
                int sortValue = 0;
                switch (column) {
                case 1: // Размер
                    sortValue = parseSize(item->text());
                    break;
                case 2: // DPI
                    sortValue = item->text().toInt();
                    break;
                case 3: // Глубина цвета
                    sortValue = item->text().split(" ")[0].toInt();
                    break;
                }
                intSortData.append(qMakePair(sortValue, row));
            } else {
                stringSortData.append(qMakePair(item->text(), row));
            }
        }
    }

    // Сортируем данные
    if (useNumericSort) {
        if (currentSortOrder == Qt::AscendingOrder) {
            std::sort(intSortData.begin(), intSortData.end(),
                      [](const QPair<int, int> &a, const QPair<int, int> &b) {
                          return a.first < b.first;
                      });
        } else {
            std::sort(intSortData.begin(), intSortData.end(),
                      [](const QPair<int, int> &a, const QPair<int, int> &b) {
                          return a.first > b.first;
                      });
        }
    } else {
        if (currentSortOrder == Qt::AscendingOrder) {
            std::sort(stringSortData.begin(), stringSortData.end(),
                      [](const QPair<QString, int> &a, const QPair<QString, int> &b) {
                          return a.first < b.first;
                      });
        } else {
            std::sort(stringSortData.begin(), stringSortData.end(),
                      [](const QPair<QString, int> &a, const QPair<QString, int> &b) {
                          return a.first > b.first;
                      });
        }
    }

    // Создаем новый порядок строк
    QVector<int> newRowOrder;
    if (useNumericSort) {
        for (const auto &pair : intSortData) {
            newRowOrder.append(pair.second);
        }
    } else {
        for (const auto &pair : stringSortData) {
            newRowOrder.append(pair.second);
        }
    }

    // Создаем временную таблицу с отсортированными данными
    QTableWidget tempTable(0, table->columnCount());
    for (int i = 0; i < newRowOrder.size(); ++i) {
        int originalRow = newRowOrder[i];
        tempTable.insertRow(i);
        for (int col = 0; col < table->columnCount(); ++col) {
            QTableWidgetItem *originalItem = table->item(originalRow, col);
            QTableWidgetItem *newItem = new QTableWidgetItem();
            if (originalItem) {
                newItem->setText(originalItem->text());
                newItem->setToolTip(originalItem->toolTip());
            }
            tempTable.setItem(i, col, newItem);
        }
    }

    // Заменяем данные в основной таблице
    table->clearContents();
    table->setRowCount(tempTable.rowCount());
    for (int row = 0; row < tempTable.rowCount(); ++row) {
        for (int col = 0; col < tempTable.columnCount(); ++col) {
            QTableWidgetItem *item = tempTable.takeItem(row, col);
            table->setItem(row, col, item);
        }
    }

    // Обновляем статус
    QString columnName = table->horizontalHeaderItem(column)->text();
    QString orderText = (currentSortOrder == Qt::AscendingOrder) ? "по возрастанию" : "по убыванию";
    statusLabel->setText(QString("Сортировка по столбцу '%1' (%2)").arg(columnName).arg(orderText));

    for (int i = 0; i < table->columnCount(); ++i) {
        table->horizontalHeaderItem(i)->setBackground(QBrush());
    }

    // Устанавливаем стиль для текущего столбца
    QColor sortColor = QColor(200, 220, 255); // Светло-голубой фон
    table->horizontalHeaderItem(column)->setBackground(sortColor);

    // Добавляем текст с индикатором направления
    QString headerText = table->horizontalHeaderItem(column)->text();
    QString indicator = (currentSortOrder == Qt::AscendingOrder) ? " ▲" : " ▼";

    // Убираем предыдущие индикаторы
    if (headerText.endsWith(" ▲") || headerText.endsWith(" ▼")) {
        headerText = headerText.left(headerText.length() - 2);
    }

    table->horizontalHeaderItem(column)->setText(headerText + indicator);
}

void ImageAnalyzer::selectFolder()
{
    QString folder = QFileDialog::getExistingDirectory(this, "Выберите папку с изображениями");
    if (!folder.isEmpty()) {
        folderPath->setText(folder);

        QStringList filters;
        filters << "*.jpg" << "*.jpeg" << "*.gif" << "*.tif" << "*.tiff"
                << "*.bmp" << "*.png" << "*.pcx";

        QDir dir(folder);
        QFileInfoList files = dir.entryInfoList(filters, QDir::Files);

        for (const QFileInfo &fileInfo : files) {
            addImageFile(fileInfo.absoluteFilePath());
        }

        analyzeButton->setEnabled(!imageFiles.isEmpty());
        statusLabel->setText(QString("Найдено файлов: %1").arg(imageFiles.count()));
    }
}

void ImageAnalyzer::selectFiles()
{
    QStringList files = QFileDialog::getOpenFileNames(
        this,
        "Выберите графические файлы",
        "",
        "Графические файлы (*.jpg *.jpeg *.gif *.tif *.tiff *.bmp *.png *.pcx)"
        );

    if (!files.isEmpty()) {
        for (const QString &file : files) {
            addImageFile(file);
        }

        folderPath->setText(QString("Выбрано файлов: %1").arg(files.count()));
        analyzeButton->setEnabled(!imageFiles.isEmpty());
        statusLabel->setText(QString("Всего файлов: %1").arg(imageFiles.count()));
    }
}

void ImageAnalyzer::clearFiles()
{
    imageFiles.clear();
    fileList->clear();
    table->setRowCount(0);
    folderPath->clear();
    analyzeButton->setEnabled(false);
    statusLabel->setText("Список файлов очищен");
}

void ImageAnalyzer::addImageFile(const QString &filePath)
{
    if (!imageFiles.contains(filePath)) {
        imageFiles.append(filePath);
        QFileInfo fileInfo(filePath);
        fileList->addItem(fileInfo.fileName());
    }
}

void ImageAnalyzer::analyzeImages()
{
    if (imageFiles.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Нет файлов для анализа");
        return;
    }

    table->setRowCount(0);
    progressBar->setVisible(true);
    progressBar->setRange(0, imageFiles.count());
    progressBar->setValue(0);

    analyzeButton->setEnabled(false);
    selectFolderButton->setEnabled(false);
    selectFilesButton->setEnabled(false);
    clearButton->setEnabled(false);

    for (int i = 0; i < imageFiles.count(); ++i) {
        analyzeImage(imageFiles[i]);
        progressBar->setValue(i + 1);
        qApp->processEvents();
        table->resizeRowToContents(i);
    }

    progressBar->setVisible(false);
    analyzeButton->setEnabled(true);
    selectFolderButton->setEnabled(true);
    selectFilesButton->setEnabled(true);
    clearButton->setEnabled(true);
    statusLabel->setText(QString("Анализ завершен. Обработано файлов: %1").arg(imageFiles.count()));
}

void ImageAnalyzer::analyzeImage(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName();
    QString format = fileInfo.suffix().toUpper();
    qint64 fileSize = fileInfo.size();

    QImageReader reader(filePath);
    if (!reader.canRead()) {
        qDebug() << "Не удалось прочитать файл:" << filePath;

        int row = table->rowCount();
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(fileName));
        table->setItem(row, 1, new QTableWidgetItem("Ошибка чтения"));
        table->setItem(row, 2, new QTableWidgetItem("N/A"));
        table->setItem(row, 3, new QTableWidgetItem("N/A"));
        table->setItem(row, 4, new QTableWidgetItem(getCompressionInfo(format)));
        table->setItem(row, 5, new QTableWidgetItem("Формат не поддерживается"));
        return;
    }

    QSize size = reader.size();

    // Загрузка изображения только для получения глубины цвета и DPI
    QImage image;
    if (!reader.read(&image)) {
        qDebug() << "Ошибка загрузки изображения:" << filePath;

        int row = table->rowCount();
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(fileName));
        table->setItem(row, 1, new QTableWidgetItem(QString("%1 x %2").arg(size.width()).arg(size.height())));
        table->setItem(row, 2, new QTableWidgetItem("N/A"));
        table->setItem(row, 3, new QTableWidgetItem("N/A"));
        table->setItem(row, 4, new QTableWidgetItem(getCompressionInfo(format)));
        table->setItem(row, 5, new QTableWidgetItem("Ошибка загрузки данных"));
        return;
    }

    // Получение DPI
    int dpi = (image.dotsPerMeterX() > 0 &&  image.dotsPerMeterY() > 0) ? qRound(image.dotsPerMeterX() / 39.3701) : 0;
    int row = table->rowCount();
    table->insertRow(row);

    // Имя файла
    QTableWidgetItem *fileNameItem = new QTableWidgetItem(fileName);

    QString tooltipHtml = QString(
                              "<b>%1</b><br>"
                              "<font color='gray'>%2</font><br><br>"
                              "<img src='file:///%3' width='150' style='border: 1px solid #ccc;'>"
                              ).arg(fileName).arg(filePath).arg(filePath);

    fileNameItem->setToolTip(tooltipHtml);

    // Размер изображения
    QTableWidgetItem *sizeItem = new QTableWidgetItem(QString("%1 x %2").arg(size.width()).arg(size.height()));

    // Разрешение DPI
    QTableWidgetItem *dpiItem = new QTableWidgetItem(QString("%1").arg(dpi));

    // Глубина цвета
    QTableWidgetItem *depthItem = new QTableWidgetItem(getColorDepthInfo(image, format));

    // Сжатие
    QTableWidgetItem *compressionItem = new QTableWidgetItem(getCompressionInfo(format));

    // Дополнительная информация
    QString extraInfo = getExtraInfo(reader, image, format, fileSize);
    QTableWidgetItem *extraItem = new QTableWidgetItem(extraInfo);

    table->setItem(row, 0, fileNameItem);
    table->setItem(row, 1, sizeItem);
    table->setItem(row, 2, dpiItem);
    table->setItem(row, 3, depthItem);
    table->setItem(row, 4, compressionItem);
    table->setItem(row, 5, extraItem);
}

QString ImageAnalyzer::getExtraInfo(QImageReader &reader, const QImage &image, const QString &format, qint64 fileSize)
{
    QStringList info;

    info << QString("Размер файла: %1 КБ").arg(fileSize / 1024);

    if (reader.supportsAnimation()) {
        info << "Анимация: поддерживается";
    }

    if (image.hasAlphaChannel()) {
        info << "Прозрачность: да";
    } else {
        info << "Прозрачность: нет";
    }

    if (image.isGrayscale()) {
        info << "Цветовая модель: градации серого";
    } else {
        info << "Цветовая модель: цветное";
    }

    // Специфичная информация для форматов
    if (format == "GIF") {
        int colors = image.colorCount();
        if (colors > 0) {
            info << QString("Цветов в палитре: %1").arg(colors);
        }
    }
    return info.join("\n");
}

QString ImageAnalyzer::getCompressionInfo(const QString &format)
{
    if (format == "JPG" || format == "JPEG") {
        return "JPEG (с потерями)";
    } else if (format == "PNG") {
        return "Deflate (без потерь)";
    } else if (format == "GIF") {
        return "LZW (без потерь)";
    } else if (format == "TIFF" || format == "TIF") {
        return "LZW/JPEG/None";
    } else if (format == "BMP") {
        return "Без сжатия";
    } else if (format == "PCX") {
        return "RLE (без потерь)";
    }
    return "Неизвестно";
}

QString ImageAnalyzer::getColorDepthInfo(const QImage &image, const QString &format)
{
    int depth = image.depth();
    QString info = QString("%1 бит").arg(depth);

    // Только базовая информация, которую можно получить из QImage
    if (depth == 1) {
        info += " (монохром)";
    } else if (depth == 8) {
        if (image.colorCount() > 0) {
            info += " (индексированные цвета)";
        } else {
            info += " (градации серого)";
        }
    } else if (depth == 24) {
        info += " (True Color)";
    } else if (depth == 32) {
        info += image.hasAlphaChannel() ? " (True Color + Alpha)" : " (True Color)";
    }

    return info;
}

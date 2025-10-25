#ifndef IMAGEANALYZER_H
#define IMAGEANALYZER_H

#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QListWidget>
#include <QTableWidget>
#include <QProgressBar>
#include <QLabel>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QImageReader>
#include <QHeaderView>

class ImageAnalyzer : public QMainWindow
{
    Q_OBJECT

public:
    ImageAnalyzer(QWidget *parent = nullptr);
    ~ImageAnalyzer();

private slots:
    void selectFolder();
    void selectFiles();
    void clearFiles();
    void analyzeImages();
    void sortTable(int column);

private:
    void addImageFile(const QString &filePath);
    void analyzeImage(const QString &filePath);
    QString getExtraInfo(QImageReader &reader, const QImage &image, const QString &format, qint64 fileSize);
    QString getCompressionInfo(const QString &format);
    QString getColorDepthInfo(const QImage &image, const QString &format);
    int parseSize(const QString sizeText) const;

    QPushButton *selectFolderButton;
    QPushButton *selectFilesButton;
    QPushButton *clearButton;
    QPushButton *analyzeButton;
    QLineEdit *folderPath;
    QListWidget *fileList;
    QTableWidget *table;
    QProgressBar *progressBar;
    QLabel *statusLabel;
    QStringList imageFiles;
};

#endif // IMAGEANALYZER_H

#include <QApplication>
#include "imageanalyzer.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Image Analyzer");

    ImageAnalyzer analyzer;
    analyzer.show();

    return app.exec();
}

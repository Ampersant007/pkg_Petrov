#ifndef COLORMODEL_H
#define COLORMODEL_H

#include <QObject>
#include <QColor>
#include <QtMath>

struct RGB{
    int r, g, b;
    RGB(int red = 0, int green = 0, int blue = 0) : r(red), g(green), b(blue) {}
};
struct HSV{
    double h, s, v;
    HSV(double hue = 0, double saturation = 0, double value = 0) : h(hue), s(saturation), v(value) {}
};
struct XYZ{
    double x, y, z;
    XYZ(double xVal = 0, double yVal = 0, double zVal = 0) : x(xVal), y(yVal), z(zVal) {}
};

class ColorModel : public QObject
{
    Q_OBJECT
private:
    RGB m_rgb;
    HSV m_hsv;
    XYZ m_xyz;
    bool m_isValid;

    HSV rgbToHsv(const RGB&) const;
    RGB hsvToRgb(const HSV&) const;
    XYZ rgbToXyz(const RGB&) const;
    RGB xyzToRgb(const XYZ&) const;

    void update();

    double clamp(double value, double min, double max) const;

public:
    explicit ColorModel(QObject *parent = nullptr);

    // Get
    RGB rgb() const;
    HSV hsv() const;
    XYZ xyz() const;
    QColor color() const;
    bool isValid() const;

public slots:
    void setRgb(const RGB&);
    void setHsv(const HSV&);
    void setXyz(const XYZ&);
    void setColor(const QColor &color);

signals:
    void colorChanged(const QColor &color);
    void rgbChanged(const RGB&);
    void hsvChanged(const HSV&);
    void xyzChanged(const XYZ&);
    void validationError(const QString &message);
    void roundingNotification(const QString& message);
};

#endif // COLORMODEL_H

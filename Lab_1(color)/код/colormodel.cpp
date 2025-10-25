#include "colormodel.h"
#include <QDebug>

ColorModel::ColorModel(QObject *parent) :
    QObject(parent),
    m_rgb(255, 255, 255),
    m_hsv(0, 0, 100),
    m_xyz(95.05, 100.0, 108.9),
    m_isValid(true)
{}

RGB ColorModel::rgb() const { return m_rgb;}

HSV ColorModel::hsv() const { return m_hsv; }

XYZ ColorModel::xyz() const { return m_xyz; }

QColor ColorModel::color() const { return QColor(m_rgb.r, m_rgb.g, m_rgb.b); }

bool ColorModel::isValid() const { return m_isValid; }

void ColorModel::setRgb(const RGB& rgb)
{
    RGB clampedRgb;
    clampedRgb.r = clamp(rgb.r, 0, 255);
    clampedRgb.g = clamp(rgb.g, 0, 255);
    clampedRgb.b = clamp(rgb.b, 0, 255);

    // Check rounding
    if (rgb.r != clampedRgb.r || rgb.g != clampedRgb.g || rgb.b != clampedRgb.b) {
        emit roundingNotification("RGB values were clamped to 0-255 range");
    }

    m_rgb = clampedRgb;
    m_hsv = rgbToHsv(m_rgb);
    m_xyz = rgbToXyz(m_rgb);
    m_isValid = true;
    update();
}

void ColorModel::setHsv(const HSV& hsv)
{
    HSV clampedHsv = hsv;
    clampedHsv.h = clamp(hsv.h, 0, 360);
    clampedHsv.s = clamp(hsv.s, 0, 100);
    clampedHsv.v = clamp(hsv.v, 0, 100);

    RGB newRgb = hsvToRgb(clampedHsv);

    // Check Rounding
    HSV convertedBack = rgbToHsv(newRgb);
    if (qAbs(convertedBack.h - clampedHsv.h) > 0.5 ||
        qAbs(convertedBack.s - clampedHsv.s) > 0.5 ||
        qAbs(convertedBack.v - clampedHsv.v) > 0.5) {
        emit roundingNotification("Note: Some values were rounded due to HSV -> RGB conversion");
    }

    m_rgb = newRgb;
    m_hsv = clampedHsv;
    m_xyz = rgbToXyz(m_rgb);
    m_isValid = true;
    update();
}

void ColorModel::setXyz(const XYZ& xyz)
{
    XYZ clampedXyz;
    clampedXyz.x = clamp(xyz.x, 0, 95.05);
    clampedXyz.y = clamp(xyz.y, 0, 100.0);
    clampedXyz.z = clamp(xyz.z, 0, 108.9);

    RGB newRgb = xyzToRgb(clampedXyz);
    XYZ convertedBack = rgbToXyz(newRgb);

    if (qAbs(convertedBack.x - clampedXyz.x) > 0.5 ||
        qAbs(convertedBack.y - clampedXyz.y) > 0.5 ||
        qAbs(convertedBack.z - clampedXyz.z) > 0.5) {
        emit roundingNotification("Note: Some values were rounded due to XYZ -> RGB conversion");
    }

    m_rgb = newRgb;
    m_hsv = rgbToHsv(m_rgb);
    m_xyz = clampedXyz;
    m_isValid = true;
    update();
}

void ColorModel::setColor(const QColor &color)
{
    if (color.isValid()) {
        m_rgb.r = color.red();
        m_rgb.g = color.green();
        m_rgb.b = color.blue();
        m_hsv = rgbToHsv(m_rgb);
        m_xyz = rgbToXyz(m_rgb);
        m_isValid = true;
        update();
    } else {
        m_isValid = false;
        emit validationError("Invalid QColor provided");
    }
}

void ColorModel::update()
{
    emit colorChanged(color());
    emit rgbChanged(m_rgb);
    emit hsvChanged(m_hsv);
    emit xyzChanged(m_xyz);
}

HSV ColorModel::rgbToHsv(const RGB& rgb) const
{
    double red = rgb.r / 255.0;
    double green = rgb.g / 255.0;
    double blue = rgb.b / 255.0;

    double max = qMax(red, qMax(green, blue));
    double min = qMin(red, qMin(green, blue));
    double delta = max - min;

    HSV hsv;

    hsv.v = max * 100.0;

    if (max < 0.0001) {
        hsv.s = 0;
    } else {
        hsv.s = (delta / max) * 100.0;
    }

    if (delta < 0.0001) {
        hsv.h = 0;
    } else if (red >= max) {
        hsv.h = 60.0 * fmod(((green - blue) / delta), 6.0);
    } else if (green >= max) {
        hsv.h = 60.0 * (((blue - red) / delta) + 2.0);
    } else {
        hsv.h = 60.0 * (((red - green) / delta) + 4.0);
    }

    if(hsv.h < 0) hsv.h +=360.0;

    return hsv;
}

RGB ColorModel::hsvToRgb(const HSV& hsv) const
{
    double h = clamp(hsv.h, 0, 360);
    double s = clamp(hsv.s, 0, 100) / 100.0;
    double v = clamp(hsv.v, 0, 100) / 100.0;

    double c =v * s;
    double x = c * (1 - fabs(fmod(h / 60.0, 2) - 1));
    double m = v - c;

    double r, g, b;

    if (h < 60) {
        r = c; g = x; b = 0;
    } else if (h < 120) {
        r = x; g = c; b = 0;
    } else if (h < 180) {
        r = 0; g = c; b = x;
    } else if (h < 240) {
        r = 0; g = x; b = c;
    } else if (h < 300) {
        r = x; g = 0; b = c;
    } else {
        r = c; g = 0; b = x;
    }

    return RGB(
        clamp((r + m) * 255, 0, 255),
        clamp((g + m) * 255, 0, 255),
        clamp((b + m) * 255, 0, 255)
        );
}

XYZ ColorModel::rgbToXyz(const RGB& rgb) const
{
    double red = rgb.r / 255.0;
    double green = rgb.g / 255.0;
    double blue = rgb.b / 255.0;

    red = ((red >= 0.04045) ? pow((red + 0.055) / 1.055, 2.4) : red / 12.92) * 100;
    green = ((green >= 0.04045) ? pow((green + 0.055) / 1.055, 2.4) : green / 12.92) * 100;
    blue = ((blue >= 0.04045) ? pow((blue + 0.055) / 1.055, 2.4) : blue / 12.92) * 100;

    double x = red * 0.412453 + green * 0.35758 + blue * 0.180423;
    double y = red * 0.212671 + green * 0.71516 + blue * 0.072169;
    double z = red * 0.019334 + green * 0.119193 + blue * 0.950227;

    return XYZ(x, y, z);
}

RGB ColorModel::xyzToRgb(const XYZ& xyz) const
{
    double x = xyz.x / 100.0;
    double y = xyz.y / 100.0;
    double z = xyz.z / 100.0;

    double red = x * 3.2406 + y * -1.5372 + z * -0.4986;
    double green = x * -0.9689 + y * 1.8758 + z * 0.0415;
    double blue = x * 0.0557 + y * -0.2040 + z * 1.0570;

    red = (red >= 0.0031308) ? (1.055 * pow(red, 1/2.4) - 0.055) : (12.92 * red);
    green = (green >= 0.0031308) ? (1.055 * pow(green, 1/2.4) - 0.055) : (12.92 * green);
    blue = (blue >= 0.0031308) ? (1.055 * pow(blue, 1/2.4) - 0.055) : (12.92 * blue);

    return RGB(
        clamp(red * 255, 0, 255),
        clamp(green * 255, 0, 255),
        clamp(blue * 255, 0, 255)
        );
}

double ColorModel::clamp(double value, double min, double max) const
{
    if (value < min) return min;
    if (value > max) return max;
    return value;
}


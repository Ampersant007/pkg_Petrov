#ifndef TRANSFORMATIONS_H
#define TRANSFORMATIONS_H

#include <QMatrix4x4>
#include <QVector3D>

class Transformations {
public:
    static QMatrix4x4 createRotation(float angleX, float angleY, float angleZ);
    static QMatrix4x4 createScale(float scaleX, float scaleY, float scaleZ);
    static QMatrix4x4 createTranslation(float tx, float ty, float tz);
    static QMatrix4x4 createPerspective(float fov, float aspect, float nearPlane, float farPlane);
    static QMatrix4x4 createOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);
    static QMatrix4x4 createViewMatrix(const QVector3D &eye, const QVector3D &center, const QVector3D &up);

    // Новые функции для вращения вокруг произвольной оси
    static QMatrix4x4 createRotationAroundAxis(const QVector3D &axis, float angle, const QVector3D &center = QVector3D(0, 0, 0));
    static QMatrix4x4 createRotationAroundAxis(const QVector3D &point, const QVector3D &direction, float angle);
};

#endif // TRANSFORMATIONS_H

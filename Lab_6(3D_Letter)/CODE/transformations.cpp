#include "transformations.h"
#include <cmath>

QMatrix4x4 Transformations::createRotation(float angleX, float angleY, float angleZ) {
    QMatrix4x4 rotation;
    rotation.setToIdentity();

    if (angleX != 0) rotation.rotate(angleX, 1.0f, 0.0f, 0.0f);
    if (angleY != 0) rotation.rotate(angleY, 0.0f, 1.0f, 0.0f);
    if (angleZ != 0) rotation.rotate(angleZ, 0.0f, 0.0f, 1.0f);

    return rotation;
}

QMatrix4x4 Transformations::createScale(float scaleX, float scaleY, float scaleZ) {
    QMatrix4x4 scale;
    scale.setToIdentity();
    scale.scale(scaleX, scaleY, scaleZ);
    return scale;
}

QMatrix4x4 Transformations::createTranslation(float tx, float ty, float tz) {
    QMatrix4x4 translation;
    translation.setToIdentity();
    translation.translate(tx, ty, tz);
    return translation;
}

QMatrix4x4 Transformations::createPerspective(float fov, float aspect, float nearPlane, float farPlane) {
    QMatrix4x4 perspective;
    perspective.setToIdentity();
    perspective.perspective(fov, aspect, nearPlane, farPlane);
    return perspective;
}

QMatrix4x4 Transformations::createOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
    QMatrix4x4 ortho;
    ortho.setToIdentity();
    ortho.ortho(left, right, bottom, top, nearPlane, farPlane);
    return ortho;
}

QMatrix4x4 Transformations::createViewMatrix(const QVector3D &eye, const QVector3D &center, const QVector3D &up) {
    QMatrix4x4 view;
    view.setToIdentity();
    view.lookAt(eye, center, up);
    return view;
}

QMatrix4x4 Transformations::createRotationAroundAxis(const QVector3D &axis, float angle, const QVector3D &center) {
    QMatrix4x4 rotation;
    rotation.setToIdentity();

    if (axis.length() > 0) {
        QVector3D a = axis.normalized();
        float radians = angle * M_PI / 180.0f;
        float c = cos(radians);
        float s = sin(radians);
        float t = 1.0f - c;

        // Матрица вращения Родригеса
        QMatrix4x4 R;
        R(0, 0) = t * a.x() * a.x() + c;
        R(0, 1) = t * a.x() * a.y() - s * a.z();
        R(0, 2) = t * a.x() * a.z() + s * a.y();
        R(0, 3) = 0;

        R(1, 0) = t * a.x() * a.y() + s * a.z();
        R(1, 1) = t * a.y() * a.y() + c;
        R(1, 2) = t * a.y() * a.z() - s * a.x();
        R(1, 3) = 0;

        R(2, 0) = t * a.x() * a.z() - s * a.y();
        R(2, 1) = t * a.y() * a.z() + s * a.x();
        R(2, 2) = t * a.z() * a.z() + c;
        R(2, 3) = 0;

        R(3, 0) = 0;
        R(3, 1) = 0;
        R(3, 2) = 0;
        R(3, 3) = 1;

        // Матрица вращения вокруг произвольной точки:
        // M = T * R * T⁻¹, где T - трансляция в точку center
        QMatrix4x4 T;
        T.setToIdentity();
        T.translate(center);

        QMatrix4x4 T_inv;
        T_inv.setToIdentity();
        T_inv.translate(-center);

        rotation = T * R * T_inv;
    }

    return rotation;
}

QMatrix4x4 Transformations::createRotationAroundAxis(const QVector3D &point, const QVector3D &direction, float angle) {
    return createRotationAroundAxis(direction, angle, point);
}

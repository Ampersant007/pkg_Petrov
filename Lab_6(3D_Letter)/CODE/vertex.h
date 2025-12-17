#ifndef VERTEX_H
#define VERTEX_H

#include <QVector3D>

struct Vertex {
    QVector3D position;
    QVector3D color;

    Vertex() = default;
    Vertex(const QVector3D &pos, const QVector3D &col)
        : position(pos), color(col) {}
};

#endif // VERTEX_H

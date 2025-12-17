#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMatrix4x4>
#include <QVector3D>
#include <QMouseEvent>
#include <QWheelEvent>
#include "vertex.h"

class OpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
public:
    explicit OpenGLWidget(QWidget *parent = nullptr);
    ~OpenGLWidget();

    void setTransformParameters(float angle,
                                float scaleX, float scaleY, float scaleZ,
                                float transX, float transY, float transZ);

    void setRotationAxis(const QVector3D &point, const QVector3D &direction, bool preservePosition = true);
    void resetRotation();

    void setPerspectiveProjection();
    void setOrthographicProjection(int type);

    void resetCamera();
    void setCameraPosition(float pitch, float yaw, float zoom);

    void setShowGrid(bool show);
    void setShowAxes(bool show);
    void setShowRotationAxis(bool show);

    QMatrix4x4 getTransformMatrix() const { return m_transformMatrix; }
    QVector3D getRotationAxisPoint() const { return m_rotationAxisPoint; }
    QVector3D getRotationAxisDirection() const { return m_rotationAxisDirection; }
    QVector3D getTranslation() const;
    float getRotationAngle() const { return m_rotationAngle; }
    float getScaleX() const { return m_scaleX; }
    float getScaleY() const { return m_scaleY; }
    float getScaleZ() const { return m_scaleZ; }
    float getPitch() const { return m_pitch; }
    float getYaw() const { return m_yaw; }
    float getZoom() const { return m_zoom; }

signals:
    void cameraChanged(float pitch, float yaw, float zoom);
    void rotationAngleChanged(float angle);
    void rotationAxisChanged(const QVector3D &point, const QVector3D &direction);
    void transformChanged(float angle,
                          float scaleX, float scaleY, float scaleZ,
                          float transX, float transY, float transZ);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void initShaders();
    void initLetterVertices();  // Новая функция для создания буквы "П"
    void initGeometry();
    void updateTransformedVertices();
    void updateViewMatrix();
    void updateProjectionMatrix();
    QMatrix4x4 calculateTransformMatrix();
    void updateAxisGeometry();

    float calculateNewRotationAngle(const QVector3D &newAxisPoint,
                                    const QVector3D &newAxisDirection);

    QOpenGLShaderProgram *m_program;
    QOpenGLVertexArrayObject m_letterVAO;
    QOpenGLVertexArrayObject m_gridVAO;
    QOpenGLVertexArrayObject m_axesVAO;
    QOpenGLVertexArrayObject m_axisVAO;
    QOpenGLBuffer m_letterVBO;
    QOpenGLBuffer m_gridVBO;
    QOpenGLBuffer m_axesVBO;
    QOpenGLBuffer m_axisVBO;

    QVector<QVector3D> m_baseVertices;  // 16 вершин для буквы "П"
    QVector<Vertex> m_letterVertices;   // 20 ребер = 40 вершин для отрисовки
    QVector<Vertex> m_gridVertices;
    QVector<Vertex> m_axesVertices;
    QVector<Vertex> m_axisVertices;

    QMatrix4x4 m_projectionMatrix;
    QMatrix4x4 m_viewMatrix;
    QMatrix4x4 m_transformMatrix;

    float m_rotationAngle;
    float m_scaleX;
    float m_scaleY;
    float m_scaleZ;
    float m_translationX;
    float m_translationY;
    float m_translationZ;

    QVector3D m_rotationAxisPoint;
    QVector3D m_rotationAxisDirection;

    float m_pitch;
    float m_yaw;
    float m_zoom;
    QPoint m_lastMousePos;

    bool m_showGrid;
    bool m_showAxes;
    bool m_showRotationAxis;

    enum ProjectionType {
        Perspective,
        OrthographicXY,
        OrthographicXZ,
        OrthographicYZ
    };
    ProjectionType m_projectionType;

    int m_width;
    int m_height;
};

#endif // OPENGLWIDGET_H

#include "openglwidget.h"
#include "transformations.h"
#include <QDebug>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

OpenGLWidget::OpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    , m_rotationAngle(0)
    , m_scaleX(1.0), m_scaleY(1.0), m_scaleZ(1.0)
    , m_translationX(0), m_translationY(0), m_translationZ(0)
    , m_rotationAxisPoint(0, 0, 0)
    , m_rotationAxisDirection(0, 0, 1)
    , m_pitch(-30.0f)
    , m_yaw(-45.0f)
    , m_zoom(5.0f)
    , m_showGrid(true)
    , m_showAxes(true)
    , m_showRotationAxis(true)
    , m_projectionType(Perspective)
    , m_width(800)
    , m_height(600) {

    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    setFormat(format);

    m_transformMatrix.setToIdentity();
    initLetterVertices();  // Заменяем initBaseVertices на initLetterVertices
}

OpenGLWidget::~OpenGLWidget() {
    makeCurrent();
    m_letterVAO.destroy();
    m_gridVAO.destroy();
    m_axesVAO.destroy();
    m_axisVAO.destroy();
    m_letterVBO.destroy();
    m_gridVBO.destroy();
    m_axesVBO.destroy();
    m_axisVBO.destroy();
    delete m_program;
    doneCurrent();
}

void OpenGLWidget::initializeGL() {
    initializeOpenGLFunctions();
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    initShaders();
    initGeometry();
    updateAxisGeometry();
    updateViewMatrix();
}

void OpenGLWidget::initShaders() {
    m_program = new QOpenGLShaderProgram(this);

    const char* vsrc =
        "#version 330 core\n"
        "layout(location = 0) in vec3 position;\n"
        "layout(location = 1) in vec3 color;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "out vec3 fragColor;\n"
        "void main() {\n"
        "    gl_Position = projection * view * vec4(position, 1.0);\n"
        "    fragColor = color;\n"
        "}\n";

    const char* fsrc =
        "#version 330 core\n"
        "in vec3 fragColor;\n"
        "out vec4 outColor;\n"
        "void main() {\n"
        "    outColor = vec4(fragColor, 1.0);\n"
        "}\n";

    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vsrc);
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fsrc);
    m_program->link();

    if (!m_program->isLinked()) {
        qDebug() << "Shader program link failed:" << m_program->log();
    }
}

void OpenGLWidget::initLetterVertices() {
    // Буква "П" - 3D каркас с вырезом спереди
    // 16 вершин для передней грани

    // Размеры буквы "П"
    float width = 1.5f;      // Ширина
    float height = 2.0f;     // Высота
    float depth = 0.3f;      // Глубина (толщина)
    float innerWidth = 0.9f; // Ширина внутреннего выреза
    float innerHeight = 1.7f; // Высота внутреннего выреза

    // Очищаем базовые вершины
    m_baseVertices.clear();

    // ПЕРЕДНИЙ СЛОЙ (z = depth/2)
    // Внешний прямоугольник (индексы 0-3)
    m_baseVertices.append(QVector3D(-width/2, height/2, depth/2));         // 0: Верхний левый внешний (9)
    m_baseVertices.append(QVector3D(width/2, height/2, depth/2));          // 1: Верхний правый внешний (12)
    m_baseVertices.append(QVector3D(width/2, -height/2, depth/2));         // 2: Нижний правый внешний (13)
    m_baseVertices.append(QVector3D(innerWidth/2, -height/2, depth/2));        // 3: Нижний левый внешний (7)

    // Внутренний прямоугольник выреза (индексы 4-7)
    m_baseVertices.append(QVector3D(innerWidth/2, -height/2 + innerHeight, depth/2));  // 4: Верхний левый внутренний (6?)
    m_baseVertices.append(QVector3D(-innerWidth/2, -height/2 + innerHeight, depth/2));   // 5: Верхний правый внутренний (16?)
    m_baseVertices.append(QVector3D(-innerWidth/2, -height/2, depth/2));    // 6: Нижний правый внутренний (1)
    m_baseVertices.append(QVector3D(-width/2, -height/2, depth/2));   // 7: Нижний левый внутренний (2)

    // ЗАДНИЙ СЛОЙ (z = -depth/2)
    // Внешний прямоугольник (индексы 8-11)
    m_baseVertices.append(QVector3D(-width/2, height/2, -depth/2));         // 8
    m_baseVertices.append(QVector3D(width/2, height/2, -depth/2));          // 9
    m_baseVertices.append(QVector3D(width/2, -height/2, -depth/2));         // 10
    m_baseVertices.append(QVector3D(innerWidth/2, -height/2, -depth/2));        // 11

    // Внутренний прямоугольник выреза (индексы 12-15)
    m_baseVertices.append(QVector3D(innerWidth/2, -height/2 + innerHeight, -depth/2));  // 12
    m_baseVertices.append(QVector3D(-innerWidth/2, -height/2 + innerHeight, -depth/2));   // 13
    m_baseVertices.append(QVector3D(-innerWidth/2, -height/2, -depth/2));    // 14
    m_baseVertices.append(QVector3D(-width/2, -height/2, -depth/2)); //15
}

void OpenGLWidget::setTransformParameters(float angle,
                                          float scaleX, float scaleY, float scaleZ,
                                          float transX, float transY, float transZ) {
    bool angleChanged = m_rotationAngle != angle;

    m_rotationAngle = angle;
    m_scaleX = scaleX;
    m_scaleY = scaleY;
    m_scaleZ = scaleZ;
    m_translationX = transX;
    m_translationY = transY;
    m_translationZ = transZ;

    if (angleChanged) {
        emit rotationAngleChanged(angle);
    }

    updateTransformedVertices();
    emit transformChanged(m_rotationAngle, m_scaleX, m_scaleY, m_scaleZ,
                          m_translationX, m_translationY, m_translationZ);
}

void OpenGLWidget::setRotationAxis(const QVector3D &point, const QVector3D &direction, bool preservePosition) {
    if (direction.length() < 0.0001) {
        qDebug() << "Направляющий вектор не может быть нулевым!";
        return;
    }

    QVector3D newDir = direction.normalized();

    // Сохраняем старый угол перед сбросом
    float oldAngle = m_rotationAngle;

    // ОБНУЛЯЕМ УГОЛ ВРАЩЕНИЯ при изменении оси
    m_rotationAngle = 0;

    if (preservePosition) {
        QMatrix4x4 oldTransform = calculateTransformMatrix();

        m_rotationAxisPoint = point;
        m_rotationAxisDirection = newDir;

        // Пересчитываем с новым углом (0)
        QMatrix4x4 newTransform = calculateTransformMatrix();

        QVector3D centroid(0, 0, 0);
        for (const auto &v : m_baseVertices) {
            centroid += v;
        }
        centroid /= float(m_baseVertices.size());

        QVector4D oldPos = oldTransform * QVector4D(centroid, 1.0f);
        QVector4D newPos = newTransform * QVector4D(centroid, 1.0f);

        QVector3D delta = QVector3D(oldPos.x(), oldPos.y(), oldPos.z()) -
                          QVector3D(newPos.x(), newPos.y(), newPos.z());

        m_translationX += delta.x();
        m_translationY += delta.y();
        m_translationZ += delta.z();

        updateAxisGeometry();
        updateTransformedVertices();

        emit transformChanged(m_rotationAngle, m_scaleX, m_scaleY, m_scaleZ,
                              m_translationX, m_translationY, m_translationZ);
        emit rotationAxisChanged(point, m_rotationAxisDirection);

        // Отправляем сигнал об изменении угла
        emit rotationAngleChanged(0);
        return;
    }

    m_rotationAxisPoint = point;
    m_rotationAxisDirection = newDir;

    updateAxisGeometry();
    updateTransformedVertices();

    emit rotationAxisChanged(point, m_rotationAxisDirection);
    emit transformChanged(m_rotationAngle, m_scaleX, m_scaleY, m_scaleZ,
                          m_translationX, m_translationY, m_translationZ);

    // Отправляем сигнал об изменении угла
    emit rotationAngleChanged(0);
}

float OpenGLWidget::calculateNewRotationAngle(const QVector3D &newAxisPoint,
                                              const QVector3D &newAxisDirection) {
    QVector3D oldDir = m_rotationAxisDirection.normalized();
    QVector3D newDir = newAxisDirection.normalized();

    float dotProduct = QVector3D::dotProduct(oldDir, newDir);
    if (fabs(dotProduct) > 0.9999f) {
        return m_rotationAngle;
    }

    return 0;
}

void OpenGLWidget::resetRotation() {
    m_rotationAngle = 0;
    updateTransformedVertices();
    emit rotationAngleChanged(0);
}

void OpenGLWidget::updateTransformedVertices() {
    m_transformMatrix = calculateTransformMatrix();

    QVector<QVector3D> transformedVertices;
    for (const auto& vertex : m_baseVertices) {
        QVector4D transformed = m_transformMatrix * QVector4D(vertex, 1.0f);
        transformedVertices.append(QVector3D(transformed.x(), transformed.y(), transformed.z()));
    }

    // Проверяем, что у нас достаточно вершин
    if (transformedVertices.size() < 16) {
        qDebug() << "Ошибка: недостаточно вершин. Только" << transformedVertices.size();
        return;
    }

    QVector3D letterColor(0.2f, 0.6f, 0.9f);
    m_letterVertices.clear();

    // Рёбра для буквы "П" с вырезом

    // 1. Внешний прямоугольник спереди (4 ребра)
    m_letterVertices.append(Vertex(transformedVertices[0], letterColor)); // 0-1 (верх)
    m_letterVertices.append(Vertex(transformedVertices[1], letterColor));

    m_letterVertices.append(Vertex(transformedVertices[1], letterColor)); // 1-2 (право)
    m_letterVertices.append(Vertex(transformedVertices[2], letterColor));

    m_letterVertices.append(Vertex(transformedVertices[2], letterColor)); // 2-3 (низ)
    m_letterVertices.append(Vertex(transformedVertices[3], letterColor));

    m_letterVertices.append(Vertex(transformedVertices[3], letterColor)); // 3-0 (лево)
    m_letterVertices.append(Vertex(transformedVertices[4], letterColor));

    // 2. Внутренний прямоугольник спереди (4 ребра)
    m_letterVertices.append(Vertex(transformedVertices[4], letterColor)); // 4-5 (верх внутренний)
    m_letterVertices.append(Vertex(transformedVertices[5], letterColor));

    m_letterVertices.append(Vertex(transformedVertices[5], letterColor)); // 5-6 (право внутренний)
    m_letterVertices.append(Vertex(transformedVertices[6], letterColor));

    m_letterVertices.append(Vertex(transformedVertices[6], letterColor)); // 6-7 (низ внутренний)
    m_letterVertices.append(Vertex(transformedVertices[7], letterColor));

    m_letterVertices.append(Vertex(transformedVertices[7], letterColor)); // 7-4 (лево внутренний)
    m_letterVertices.append(Vertex(transformedVertices[0], letterColor));

    // 3. Внешний прямоугольник сзади (4 ребра)
    m_letterVertices.append(Vertex(transformedVertices[8], letterColor)); // 8-9
    m_letterVertices.append(Vertex(transformedVertices[9], letterColor));

    m_letterVertices.append(Vertex(transformedVertices[9], letterColor)); // 9-10
    m_letterVertices.append(Vertex(transformedVertices[10], letterColor));

    m_letterVertices.append(Vertex(transformedVertices[10], letterColor)); // 10-11
    m_letterVertices.append(Vertex(transformedVertices[11], letterColor));

    m_letterVertices.append(Vertex(transformedVertices[11], letterColor)); // 11-8
    m_letterVertices.append(Vertex(transformedVertices[12], letterColor));

    // 4. Внутренний прямоугольник сзади (4 ребра)
    m_letterVertices.append(Vertex(transformedVertices[12], letterColor)); // 12-13
    m_letterVertices.append(Vertex(transformedVertices[13], letterColor));

    m_letterVertices.append(Vertex(transformedVertices[13], letterColor)); // 13-14
    m_letterVertices.append(Vertex(transformedVertices[14], letterColor));

    m_letterVertices.append(Vertex(transformedVertices[14], letterColor)); // 14-15
    m_letterVertices.append(Vertex(transformedVertices[15], letterColor));

    m_letterVertices.append(Vertex(transformedVertices[15], letterColor)); // 15-12
    m_letterVertices.append(Vertex(transformedVertices[8], letterColor));

    // 5. Соединения между передним и задним слоями (8 ребер)
    // Внешние углы
    m_letterVertices.append(Vertex(transformedVertices[0], letterColor)); // 0-8
    m_letterVertices.append(Vertex(transformedVertices[8], letterColor));

    m_letterVertices.append(Vertex(transformedVertices[1], letterColor)); // 1-9
    m_letterVertices.append(Vertex(transformedVertices[9], letterColor));

    m_letterVertices.append(Vertex(transformedVertices[2], letterColor)); // 2-10
    m_letterVertices.append(Vertex(transformedVertices[10], letterColor));

    m_letterVertices.append(Vertex(transformedVertices[3], letterColor)); // 3-11
    m_letterVertices.append(Vertex(transformedVertices[11], letterColor));

    // Внутренние углы
    m_letterVertices.append(Vertex(transformedVertices[4], letterColor)); // 4-12
    m_letterVertices.append(Vertex(transformedVertices[12], letterColor));

    m_letterVertices.append(Vertex(transformedVertices[5], letterColor)); // 5-13
    m_letterVertices.append(Vertex(transformedVertices[13], letterColor));

    m_letterVertices.append(Vertex(transformedVertices[6], letterColor)); // 6-14
    m_letterVertices.append(Vertex(transformedVertices[14], letterColor));

    m_letterVertices.append(Vertex(transformedVertices[7], letterColor)); // 7-15
    m_letterVertices.append(Vertex(transformedVertices[15], letterColor));

    if (m_letterVBO.isCreated()) {
        m_letterVAO.bind();
        m_letterVBO.bind();
        m_letterVBO.allocate(m_letterVertices.constData(), m_letterVertices.size() * sizeof(Vertex));
        m_letterVAO.release();
        m_letterVBO.release();
    }
    update();
}

QMatrix4x4 OpenGLWidget::calculateTransformMatrix() {
    QMatrix4x4 transform;
    transform.setToIdentity();

    // Порядок преобразований:
    // 1. Масштабирование
    // 2. Вращение вокруг оси
    // 3. Трансляция

    transform.scale(m_scaleX, m_scaleY, m_scaleZ);

    if (m_rotationAngle != 0) {
        QMatrix4x4 rotation = Transformations::createRotationAroundAxis(
            m_rotationAxisPoint, m_rotationAxisDirection, m_rotationAngle);
        transform = transform * rotation;
    }

    transform.translate(m_translationX, m_translationY, m_translationZ);

    return transform;
}

void OpenGLWidget::updateAxisGeometry() {
    m_axisVertices.clear();
    QVector3D axisColor(1.0f, 0.5f, 0.0f);

    float axisLength = 3.0f;
    QVector3D startPoint = m_rotationAxisPoint - m_rotationAxisDirection * axisLength;
    QVector3D endPoint = m_rotationAxisPoint + m_rotationAxisDirection * axisLength;

    m_axisVertices.append(Vertex(startPoint, axisColor));
    m_axisVertices.append(Vertex(endPoint, axisColor));

    QVector3D pointColor(1.0f, 0.0f, 1.0f);
    m_axisVertices.append(Vertex(m_rotationAxisPoint, pointColor));

    if (m_axisVBO.isCreated()) {
        m_axisVAO.bind();
        m_axisVBO.bind();
        m_axisVBO.allocate(m_axisVertices.constData(), m_axisVertices.size() * sizeof(Vertex));
        m_axisVAO.release();
        m_axisVBO.release();
    }
}

void OpenGLWidget::initGeometry() {
    updateTransformedVertices();

    float gridSize = 5.0f;
    float gridStep = 0.5f;
    QVector3D gridColor(0.7f, 0.7f, 0.7f);

    for (float y = -gridSize; y <= gridSize; y += gridStep) {
        m_gridVertices.append(Vertex(QVector3D(-gridSize, y, 0), gridColor));
        m_gridVertices.append(Vertex(QVector3D(gridSize, y, 0), gridColor));
    }

    for (float x = -gridSize; x <= gridSize; x += gridStep) {
        m_gridVertices.append(Vertex(QVector3D(x, -gridSize, 0), gridColor));
        m_gridVertices.append(Vertex(QVector3D(x, gridSize, 0), gridColor));
    }

    float axisLength = 3.0f;
    m_axesVertices = {
        Vertex(QVector3D(0, 0, 0), QVector3D(1.0f, 0.0f, 0.0f)),
        Vertex(QVector3D(axisLength, 0, 0), QVector3D(1.0f, 0.0f, 0.0f)),
        Vertex(QVector3D(0, 0, 0), QVector3D(0.0f, 1.0f, 0.0f)),
        Vertex(QVector3D(0, axisLength, 0), QVector3D(0.0f, 1.0f, 0.0f)),
        Vertex(QVector3D(0, 0, 0), QVector3D(0.0f, 0.0f, 1.0f)),
        Vertex(QVector3D(0, 0, axisLength), QVector3D(0.0f, 0.0f, 1.0f))
    };

    m_letterVAO.create();
    m_letterVAO.bind();
    m_letterVBO.create();
    m_letterVBO.bind();
    m_letterVBO.allocate(m_letterVertices.constData(), m_letterVertices.size() * sizeof(Vertex));
    m_program->enableAttributeArray(0);
    m_program->setAttributeBuffer(0, GL_FLOAT, offsetof(Vertex, position), 3, sizeof(Vertex));
    m_program->enableAttributeArray(1);
    m_program->setAttributeBuffer(1, GL_FLOAT, offsetof(Vertex, color), 3, sizeof(Vertex));
    m_letterVAO.release();
    m_letterVBO.release();

    m_gridVAO.create();
    m_gridVAO.bind();
    m_gridVBO.create();
    m_gridVBO.bind();
    m_gridVBO.allocate(m_gridVertices.constData(), m_gridVertices.size() * sizeof(Vertex));
    m_program->enableAttributeArray(0);
    m_program->setAttributeBuffer(0, GL_FLOAT, offsetof(Vertex, position), 3, sizeof(Vertex));
    m_program->enableAttributeArray(1);
    m_program->setAttributeBuffer(1, GL_FLOAT, offsetof(Vertex, color), 3, sizeof(Vertex));
    m_gridVAO.release();
    m_gridVBO.release();

    m_axesVAO.create();
    m_axesVAO.bind();
    m_axesVBO.create();
    m_axesVBO.bind();
    m_axesVBO.allocate(m_axesVertices.constData(), m_axesVertices.size() * sizeof(Vertex));
    m_program->enableAttributeArray(0);
    m_program->setAttributeBuffer(0, GL_FLOAT, offsetof(Vertex, position), 3, sizeof(Vertex));
    m_program->enableAttributeArray(1);
    m_program->setAttributeBuffer(1, GL_FLOAT, offsetof(Vertex, color), 3, sizeof(Vertex));
    m_axesVAO.release();
    m_axesVBO.release();

    m_axisVAO.create();
    m_axisVAO.bind();
    m_axisVBO.create();
    m_axisVBO.bind();
    m_axisVBO.allocate(m_axisVertices.constData(), m_axisVertices.size() * sizeof(Vertex));
    m_program->enableAttributeArray(0);
    m_program->setAttributeBuffer(0, GL_FLOAT, offsetof(Vertex, position), 3, sizeof(Vertex));
    m_program->enableAttributeArray(1);
    m_program->setAttributeBuffer(1, GL_FLOAT, offsetof(Vertex, color), 3, sizeof(Vertex));
    m_axisVAO.release();
    m_axisVBO.release();
}

void OpenGLWidget::resizeGL(int w, int h) {
    m_width = w;
    m_height = h;
    glViewport(0, 0, w, h);
    updateProjectionMatrix();
}

void OpenGLWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_program->bind();
    m_program->setUniformValue("view", m_viewMatrix);
    m_program->setUniformValue("projection", m_projectionMatrix);

    if (m_showGrid) {
        m_gridVAO.bind();
        glLineWidth(1.0f);
        glDrawArrays(GL_LINES, 0, m_gridVertices.size());
        m_gridVAO.release();
    }

    if (m_showAxes) {
        m_axesVAO.bind();
        glLineWidth(3.0f);
        glDrawArrays(GL_LINES, 0, m_axesVertices.size());
        m_axesVAO.release();
    }

    if (m_showRotationAxis && m_axisVertices.size() > 0) {
        m_axisVAO.bind();
        glLineWidth(2.0f);
        glDrawArrays(GL_LINES, 0, 2);
        glPointSize(8.0f);
        glDrawArrays(GL_POINTS, 2, 1);
        m_axisVAO.release();
    }

    m_letterVAO.bind();
    glLineWidth(3.0f);
    glDrawArrays(GL_LINES, 0, m_letterVertices.size());
    m_letterVAO.release();
    m_program->release();
}

void OpenGLWidget::updateViewMatrix() {
    if (m_projectionType == Perspective) {
        m_viewMatrix.setToIdentity();
        m_viewMatrix.translate(0.0f, 0.0f, -m_zoom);
        m_viewMatrix.rotate(m_pitch, 1.0f, 0.0f, 0.0f);
        m_viewMatrix.rotate(m_yaw, 0.0f, 1.0f, 0.0f);
    } else {
        switch (m_projectionType) {
        case OrthographicXY:
            m_viewMatrix = Transformations::createViewMatrix(
                QVector3D(0, 0, 5), QVector3D(0, 0, 0), QVector3D(0, 1, 0));
            break;
        case OrthographicXZ:
            m_viewMatrix = Transformations::createViewMatrix(
                QVector3D(0, 5, 0), QVector3D(0, 0, 0), QVector3D(0, 0, -1));
            break;
        case OrthographicYZ:
            m_viewMatrix = Transformations::createViewMatrix(
                QVector3D(5, 0, 0), QVector3D(0, 0, 0), QVector3D(0, 1, 0));
            break;
        default:
            break;
        }
    }
    update();
}

void OpenGLWidget::updateProjectionMatrix() {
    float aspect = float(m_width) / float(m_height);

    if (m_projectionType == Perspective) {
        m_projectionMatrix = Transformations::createPerspective(45.0f, aspect, 0.1f, 100.0f);
    } else {
        float orthoSize = 3.0f;
        switch (m_projectionType) {
        case OrthographicXY:
        case OrthographicXZ:
        case OrthographicYZ:
            m_projectionMatrix = Transformations::createOrthographic(
                -orthoSize * aspect, orthoSize * aspect,
                -orthoSize, orthoSize,
                -10.0f, 10.0f);
            break;
        default:
            break;
        }
    }
    update();
}

void OpenGLWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && m_projectionType == Perspective) {
        m_lastMousePos = event->pos();
    }
    QOpenGLWidget::mousePressEvent(event);
}

void OpenGLWidget::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton && m_projectionType == Perspective) {
        int dx = event->x() - m_lastMousePos.x();
        int dy = event->y() - m_lastMousePos.y();

        m_yaw += dx * 0.5f;
        m_pitch += dy * 0.5f;

        if (m_pitch > 89.0f) m_pitch = 89.0f;
        if (m_pitch < -89.0f) m_pitch = -89.0f;

        m_lastMousePos = event->pos();
        updateViewMatrix();
        emit cameraChanged(m_pitch, m_yaw, m_zoom);
    }
    QOpenGLWidget::mouseMoveEvent(event);
}

void OpenGLWidget::wheelEvent(QWheelEvent *event) {
    if (m_projectionType == Perspective) {
        m_zoom -= event->angleDelta().y() * 0.001f;
        m_zoom = qBound(0.1f, m_zoom, 20.0f);
        updateViewMatrix();
        emit cameraChanged(m_pitch, m_yaw, m_zoom);
    }
    QOpenGLWidget::wheelEvent(event);
}

void OpenGLWidget::setPerspectiveProjection() {
    m_projectionType = Perspective;
    updateProjectionMatrix();
    updateViewMatrix();
}

void OpenGLWidget::setOrthographicProjection(int type) {
    switch (type) {
    case 0: m_projectionType = OrthographicXY; break;
    case 1: m_projectionType = OrthographicXZ; break;
    case 2: m_projectionType = OrthographicYZ; break;
    default: m_projectionType = OrthographicXY; break;
    }
    updateProjectionMatrix();
    updateViewMatrix();
}

void OpenGLWidget::resetCamera() {
    m_pitch = -30.0f;
    m_yaw = -45.0f;
    m_zoom = 5.0f;
    updateViewMatrix();
    emit cameraChanged(m_pitch, m_yaw, m_zoom);
}

void OpenGLWidget::setCameraPosition(float pitch, float yaw, float zoom) {
    m_pitch = pitch;
    m_yaw = yaw;
    m_zoom = zoom;
    updateViewMatrix();
}

void OpenGLWidget::setShowGrid(bool show) {
    m_showGrid = show;
    update();
}

void OpenGLWidget::setShowAxes(bool show) {
    m_showAxes = show;
    update();
}

void OpenGLWidget::setShowRotationAxis(bool show) {
    m_showRotationAxis = show;
    update();
}

QVector3D OpenGLWidget::getTranslation() const {
    return QVector3D(m_translationX, m_translationY, m_translationZ);
}

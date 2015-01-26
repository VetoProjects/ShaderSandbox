#ifndef Model3Dobj_HPP
#define Model3Dobj_HPP

#include <string>

#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QVector2D>
#include <QVector3D>

#include <QDebug>

class Model3D : protected QOpenGLFunctions{
public:
    Model3D();
    ~Model3D();

    bool init() noexcept;
    bool loadModel(const std::string &path, bool smooth = true) noexcept;
    void draw() noexcept;

private:
    void pushData(const std::vector<float> &vertices, const std::vector<float> &uvs,
                  const std::vector<float> &normals, const std::vector<unsigned> &vertexIndices) noexcept;
    QOpenGLVertexArrayObject *vao;
    GLuint
        indexSize,
        vertexCount,
        vertexBuffer,
        uvBuffer,
        normalBuffer,
        indexBuffer;
};

#endif

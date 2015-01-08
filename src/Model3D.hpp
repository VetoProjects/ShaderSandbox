#ifndef Model3Dobj_HPP
#define Model3Dobj_HPP

#include <string>

#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QVector2D>
#include <QVector3D>

class Model3D : protected QOpenGLFunctions{
public:
    Model3D();
    Model3D(const std::string &path, bool smooth = true);

    ~Model3D();

    bool loadModel(const std::string &path, bool smooth = true);
    void draw();

private:
    void pushData(const std::vector<float> &vertices, const std::vector<float> &uvs, const std::vector<float> &normals, const std::vector<unsigned> &vertexIndices);
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

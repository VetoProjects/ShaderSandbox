#include <vector>
#include <cstdio>
#include <string>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>

#include "Model3D.hpp"

using namespace std;

Model3D::Model3D(){
    vao = new QOpenGLVertexArrayObject();
    vao->create();
    vao->bind();
    glGenBuffers(1, &vertexBuffer);
    glGenBuffers(1, &uvBuffer);
    glGenBuffers(1, &normalBuffer);
    glGenBuffers(1, &indexBuffer);
}

Model3D::Model3D(const std::string &path, bool smooth) : Model3D(){
//    if(!loadFile(path))
//        throw
    loadModel(path, smooth);
}

Model3D::~Model3D(){
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteBuffers(1, &uvBuffer);
    glDeleteBuffers(1, &normalBuffer);
    glDeleteBuffers(1, &indexBuffer);
    delete vao;
}

bool Model3D::loadModel(const std::string &path, bool smooth){
//    cout << "Loading model: " << path << endl;
    ifstream file(path);
    if(!file){
//        cout << "Faild to open file " << path << "." << endl;
        return false;
    }

    vector<int>			tempVertexIndices,	tempUVIndices,	tempNormalIndices;
    vector<QVector3D>	tempVertices,						tempNormals;
    vector<QVector2D>						tempUVs;

    string line;
    while(getline(file, line)){
        stringstream lineStream(line);
        string type;
        lineStream >> type;
        if(type == "v"){
            QVector3D vertex;
            lineStream >> vertex[0] >> vertex[1] >> vertex[2];
            if(lineStream.fail()){
                cerr << "Faild to parse file (vertex): " << path << "." << endl;
                file.close();
                return false;
            }
            tempVertices.push_back(vertex);
        }else if(type == "vt"){
            QVector2D uv;
            lineStream >> uv[0] >> uv[1];
            if(lineStream.fail()){
                cerr << "Faild to parse file (uv): " << path << "." << endl;
                file.close();
                return false;
            }
            // uv[1] = -uv[1]; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
            tempUVs.push_back(uv);
        }else if(type == "vn"){
            QVector3D normal;
            lineStream >> normal[0] >> normal[1] >> normal[2];
            if(lineStream.fail()){
                cerr << "Faild to parse file (normal): " << path << "." << endl;
                file.close();
                return false;
            }
            tempNormals.push_back(normal);
        }else if(type == "f"){
            unsigned count = 0;
            vector<int> vertexIndex, uvIndex, normalIndex;
            string part;
            while(getline(lineStream, part, ' ')){
                if(part == "" || part == "\r" || part == "\n") continue;
                ++count;
                char delim;
                int vertex(0), uv(0), normal(0);
                stringstream partStream(part);
                partStream >> vertex >> delim >> uv >> delim >> normal;
                vertexIndex.push_back(vertex - 1);
                uvIndex    .push_back(uv     - 1);
                normalIndex.push_back(normal - 1);
            }
            for(unsigned i = 0; i < count - 2; ++i){
                unsigned iHalf = i / 2;
                unsigned
                    even[] = {iHalf + 1        , count - iHalf - 1, iHalf},
                    odd [] = {count - iHalf - 1, iHalf + 1        , count - iHalf - 2};
                for(unsigned j : (i%2 ? odd : even)){
                    tempVertexIndices.push_back(vertexIndex[j]);
                    tempUVIndices    .push_back(uvIndex	   [j]);
                    tempNormalIndices.push_back(normalIndex[j]);
                }
            }
        }
    }
    file.close();


    vector<float> vertices, uvs, normals;
    vector<unsigned> vertex_indices;
    // QVector2D uvEdge[] = { QVector2D(0,0), QVector2D(0,1), QVector2D(1,1) };
    // QVector2D uvEdge[3];
    // unsigned reused = 0;
    for(unsigned i = 0; i < tempVertexIndices.size(); ++i){
        short curEdge = i % 3;
        if(tempUVIndices[i] < 0){
            tempUVIndices[i] = (tempUVs.size());
            // tempUVs.push_back(uvEdge[curEdge]);
            tempUVs.push_back(QVector2D());
        }
        bool noNormal = tempNormalIndices[i] < 0;
        if(noNormal){
            tempNormalIndices[i] = tempNormals.size();
            QVector3D
                &vert1 = tempVertices[tempVertexIndices[i - curEdge + 0]],
                &vert2 = tempVertices[tempVertexIndices[i - curEdge + 1]],
                &vert3 = tempVertices[tempVertexIndices[i - curEdge + 2]],
                normal =  QVector3D::crossProduct(vert2 - vert1, vert3 - vert1);
            normal.normalize();
            tempNormals.push_back(normal);
        }
        int found = -1;
        for(int j = (vertices.size() / 3) - 1; found == -1 && j >= 0; --j){
            bool match = true;
            for(short edge  = 0; match && edge  < 3; ++edge )
                match = (tempVertices[tempVertexIndices[i]][edge] == vertices[3 * j + edge]);
            // if(match)
            //     match = glm::distance(tempVertices[tempVertexIndices[i]], QVector3D(vertices[3 * j],vertices[3 * j + 1], vertices[3 * j + 2])) < 0.001f;
            for(short coord = 0; match && coord < 2; ++coord)
                match = (tempUVs     [tempUVIndices    [i]][coord] == uvs     [2 * j + coord]);
            if(!smooth)
                for(short edge  = 0; match && edge  < 3; ++edge)
                    match = (tempNormals [tempNormalIndices[i]][edge] == normals [3 * j + edge]);
            if(match)
                found = j;
        }
        if(found >= 0){
            if(smooth)
                for(short edge : {0, 1, 2})
                    normals[found * 3 + edge] += tempNormals[i][edge];
            // ++reused;
            vertex_indices.push_back(found);
        }else{
            vertex_indices.push_back(vertices.size() / 3);
            for(short edge  : {0, 1, 2}) vertices.push_back(tempVertices[tempVertexIndices[i]][edge ]);
            for(short coord : {0, 1   }) uvs     .push_back(tempUVs     [tempUVIndices    [i]][coord]);
            for(short edge  : {0, 1, 2}) normals .push_back(tempNormals [tempNormalIndices[i]][edge ]);
        }
    }
    if(smooth)
        for(unsigned i = 0; i < normals.size(); i += 3){
            QVector3D normal = QVector3D(normals[i], normals[i+1], normals[i+2]);
            normal.normalize();
            for(short edge : {0, 1, 2})
                normals[i + edge] = normal[edge];
        }
    // cout << "reused: " << reused << endl;
    pushData(vertices, uvs, normals, vertex_indices);

    return true;
}

void Model3D::pushData(const std::vector<GLfloat> &vertices, const std::vector<GLfloat> &uvs, const std::vector<GLfloat> &normals, const std::vector<GLuint> &vertexIndices){
    vertexCount = vertices.size();

    vao->bind();

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertexCount, &vertices[0], GL_STATIC_DRAW);


    if(!uvs.empty()){
        glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * uvs.size(), &uvs[0], GL_STATIC_DRAW);
    }

    if(!normals.empty()){
        glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * normals.size(), &normals[0], GL_STATIC_DRAW);
    }

    indexSize = vertexIndices.size();
    if(!vertexIndices.empty()){
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indexSize, &vertexIndices[0], GL_STATIC_DRAW);
    }else
        glDeleteBuffers(1, &indexBuffer);

    vao->release();
}

void Model3D::draw(){
    vao->bind();


// Vertex
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glVertexAttribPointer(
        0,			// Index
        3,			// Size
        GL_FLOAT,	// Type
        GL_FALSE,	// Normalized
        0,			// Stride
        (void*)0	// Offset
    );

// UV
    if(uvBuffer > 0){
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }

// Normal
    if(normalBuffer > 0){
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }

// Draw
    if(indexBuffer > 0){
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glDrawElements(
            GL_TRIANGLES,		// Mode
            indexSize,			// Count
            GL_UNSIGNED_INT,	// Type
            (void*)0			// Element array buffer offset
        );
    }else{
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }

    glDisableVertexAttribArray(0);
    if(uvBuffer     > 0) glDisableVertexAttribArray(1);
    if(normalBuffer > 0) glDisableVertexAttribArray(2);
    // if(colorBuffer  > 0) glDisableVertexAttribArray(3);

    vao->release();
}

#include "mesh.h"
#include "meshlite.h"
#include "theme.h"
#include <assert.h>

#define MAX_VERTICES_PER_FACE   100

Mesh::Mesh(void *meshlite, int meshId, bool broken) :
    m_triangleVertices(NULL),
    m_triangleVertexCount(0),
    m_edgeVertices(NULL),
    m_edgeVertexCount(0),
    m_broken(broken)
{
    int edgeVertexPositionCount = meshlite_get_vertex_count(meshlite, meshId);
    GLfloat *edgeVertexPositions = new GLfloat[edgeVertexPositionCount * 3];
    int loadedEdgeVertexPositionItemCount = meshlite_get_vertex_position_array(meshlite, meshId, edgeVertexPositions, edgeVertexPositionCount * 3);
    
    int offset = 0;
    while (offset < loadedEdgeVertexPositionItemCount) {
        QVector3D position = QVector3D(edgeVertexPositions[offset], edgeVertexPositions[offset + 1], edgeVertexPositions[offset + 2]);
        m_vertices.push_back(position);
        offset += 3;
    }
    int faceCount = meshlite_get_face_count(meshlite, meshId);
    int *faceVertexNumAndIndices = new int[faceCount * (1 + MAX_VERTICES_PER_FACE)];
    int loadedFaceVertexNumAndIndicesItemCount = meshlite_get_face_index_array(meshlite, meshId, faceVertexNumAndIndices, faceCount * (1 + MAX_VERTICES_PER_FACE));
    offset = 0;
    while (offset < loadedFaceVertexNumAndIndicesItemCount) {
        int indicesNum = faceVertexNumAndIndices[offset++];
        assert(indicesNum >= 0 && indicesNum <= MAX_VERTICES_PER_FACE);
        std::vector<int> face;
        for (int i = 0; i < indicesNum && offset < loadedFaceVertexNumAndIndicesItemCount; i++) {
            int index = faceVertexNumAndIndices[offset++];
            assert(index >= 0 && index < loadedEdgeVertexPositionItemCount);
            face.push_back(index);
        }
        m_faces.push_back(face);
    }
    delete[] faceVertexNumAndIndices;
    faceVertexNumAndIndices = NULL;
    
    int edgeCount = meshlite_get_halfedge_count(meshlite, meshId);
    int *edgeIndices = new int[edgeCount * 2];
    int loadedEdgeVertexIndexItemCount = meshlite_get_halfedge_index_array(meshlite, meshId, edgeIndices, edgeCount * 2);
    GLfloat *edgeNormals = new GLfloat[edgeCount * 3];
    int loadedEdgeNormalItemCount = meshlite_get_halfedge_normal_array(meshlite, meshId, edgeNormals, edgeCount * 3);
    
    m_edgeVertexCount = edgeCount * 2;
    m_edgeVertices = new Vertex[m_edgeVertexCount * 3];
    for (int i = 0; i < edgeCount; i++) {
        int firstIndex = i * 2;
        for (int j = 0; j < 2; j++) {
            assert(firstIndex + j < loadedEdgeVertexIndexItemCount);
            int posIndex = edgeIndices[firstIndex + j] * 3;
            assert(posIndex < loadedEdgeVertexPositionItemCount);
            Vertex *v = &m_edgeVertices[firstIndex + j];
            v->posX = edgeVertexPositions[posIndex + 0];
            v->posY = edgeVertexPositions[posIndex + 1];
            v->posZ = edgeVertexPositions[posIndex + 2];
            assert(firstIndex + 2 < loadedEdgeNormalItemCount);
            v->normX = edgeNormals[firstIndex + 0];
            v->normY = edgeNormals[firstIndex + 1];
            v->normZ = edgeNormals[firstIndex + 2];
            v->colorR = 0.0;
            v->colorG = 0.0;
            v->colorB = 0.0;
        }
    }
    
    int triangleMesh = meshlite_triangulate(meshlite, meshId);
    
    int triangleVertexPositionCount = meshlite_get_vertex_count(meshlite, triangleMesh);
    GLfloat *triangleVertexPositions = new GLfloat[triangleVertexPositionCount * 3];
    int loadedTriangleVertexPositionItemCount = meshlite_get_vertex_position_array(meshlite, triangleMesh, triangleVertexPositions, triangleVertexPositionCount * 3);
    
    int triangleCount = meshlite_get_face_count(meshlite, triangleMesh);
    int *triangleIndices = new int[triangleCount * 3];
    int loadedTriangleVertexIndexItemCount = meshlite_get_triangle_index_array(meshlite, triangleMesh, triangleIndices, triangleCount * 3);
    GLfloat *triangleNormals = new GLfloat[triangleCount * 3];
    int loadedTriangleNormalItemCount = meshlite_get_triangle_normal_array(meshlite, triangleMesh, triangleNormals, triangleCount * 3);
    
    float modelR = Theme::white.redF();
    float modelG = Theme::white.greenF();
    float modelB = Theme::white.blueF();
    if (m_broken) {
        modelR = 1.0;
        modelG = 1.0;
        modelB = 1.0;
    }
    m_triangleVertexCount = triangleCount * 3;
    m_triangleVertices = new Vertex[m_triangleVertexCount * 3];
    for (int i = 0; i < triangleCount; i++) {
        int firstIndex = i * 3;
        for (int j = 0; j < 3; j++) {
            assert(firstIndex + j < loadedTriangleVertexIndexItemCount);
            int posIndex = triangleIndices[firstIndex + j] * 3;
            assert(posIndex < loadedTriangleVertexPositionItemCount);
            Vertex *v = &m_triangleVertices[firstIndex + j];
            v->posX = triangleVertexPositions[posIndex + 0];
            v->posY = triangleVertexPositions[posIndex + 1];
            v->posZ = triangleVertexPositions[posIndex + 2];
            assert(firstIndex + 2 < loadedTriangleNormalItemCount);
            v->normX = triangleNormals[firstIndex + 0];
            v->normY = triangleNormals[firstIndex + 1];
            v->normZ = triangleNormals[firstIndex + 2];
            v->colorR = modelR;
            v->colorG = modelG;
            v->colorB = modelB;
        }
    }
    
    delete[] triangleVertexPositions;
    delete[] triangleIndices;
    delete[] triangleNormals;
    
    delete[] edgeVertexPositions;
    delete[] edgeIndices;
    delete[] edgeNormals;
}

Mesh::~Mesh()
{
    delete[] m_triangleVertices;
    m_triangleVertexCount = 0;
}

void Mesh::setBroken(bool broken)
{
    m_broken = broken;
}

const std::vector<QVector3D> &Mesh::vertices()
{
    return m_vertices;
}

const std::vector<std::vector<int>> &Mesh::faces()
{
    return m_faces;
}

Vertex *Mesh::triangleVertices()
{
    return m_triangleVertices;
}

int Mesh::triangleVertexCount()
{
    return m_triangleVertexCount;
}

Vertex *Mesh::edgeVertices()
{
    return m_edgeVertices;
}

int Mesh::edgeVertexCount()
{
    return m_edgeVertexCount;
}



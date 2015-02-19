#ifndef __MESH_H__
#define __MESH_H__

// core of this learned from:
//   http://ogldev.atspace.co.uk/www/tutorial22/tutorial22.html
//   http://ogldev.atspace.co.uk/www/tutorial38/tutorial38.html

#define MAX_BONES_PER_VERTEX    4

struct VertexFormat
{
    Vector3 pos;
    Vector2 uv[AI_MAX_NUMBER_OF_TEXTURECOORDS];
    Vector3 norm;
    Vector3 tangent;
    unsigned char color[4];
    unsigned int boneindices[MAX_BONES_PER_VERTEX];
    float weights[MAX_BONES_PER_VERTEX];
};

struct MeshChunk
{
    std::vector<VertexFormat> m_Vertices;
    std::vector<unsigned int> m_Indices;
};

struct Bone
{
    std::string m_Name;
    MyMatrix m_OffsetMatrix;
};

class Mesh
{
public:
    std::vector<MeshChunk> m_MeshChunks;
    std::vector<Bone> m_Bones;

    unsigned int m_NumUVChannels;
    bool m_HasNormals;
    bool m_HasTangents;
    bool m_HasBitangents;
    bool m_HasColor;
    unsigned int m_MostBonesInfluences;

public:
    Mesh()
    {
        m_NumUVChannels = 0;
        m_HasNormals = false;
        m_HasTangents = false;
        m_HasBitangents = false;
        m_HasColor = false;
        m_MostBonesInfluences = 0;
    }
    ~Mesh() {}

    void LoadFromFile(const char* filename);
    void ExportToFile(const char* filename);
}; 

#endif //__MESH_H__

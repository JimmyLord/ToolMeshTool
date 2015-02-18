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
    Vector4 color;
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

public:
    //void InitMesh(unsigned int Index, const aiMesh* paiMesh);
    //bool InitMaterials(const aiScene* pScene, const std::string& Filename);
    //void Clear();

public:
    Mesh() {}
    ~Mesh() {}

    void LoadFromFile(const char* filename);
    void ExportToFile(const char* filename);
}; 

#endif //__MESH_H__

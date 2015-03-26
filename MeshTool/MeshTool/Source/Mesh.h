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
    unsigned char boneindices[MAX_BONES_PER_VERTEX]; // max 256 bones. engine code has max limit of 100 ATM anyway.
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

//struct Animation
//{
//    std::string m_Name;
//
//    double m_Duration;
//    double m_TicksPerSecond;
//};

class Mesh
{
public:
    Assimp::Importer m_Importer;
    const aiScene* m_pScene;

    std::vector<MeshChunk> m_MeshChunks;
    std::vector<Bone> m_Bones;
    std::vector<char*> m_NodeNames;
    std::vector<MyMatrix> m_NodeTransforms;
    //std::vector<Animation> m_Animations;

    unsigned int m_NumUVChannels;
    bool m_HasNormals;
    bool m_HasTangents;
    bool m_HasBitangents;
    bool m_HasColor;
    unsigned int m_MostBonesInfluences;

public:
    Mesh();
    ~Mesh();

    void LoadFromFile(const char* filename);
    void PullMeshDataFromScene();
    void PullBoneDataFromScene();

    bool IsNodeABone(aiNode* pNode);

    void ExportToFile(const char* filename);
    int ExportNodeHeirarchyDataFromScene(cJSON* pParentNode, aiNode* pNode, int depth = 0);
    void ExportAnimationDataFromScene(cJSON* pAnimationArray);
    void DumpRawNodeTransformsFromScene(FILE* file);
    void DumpRawAnimationDataFromScene(FILE* file);
}; 

#endif //__MESH_H__

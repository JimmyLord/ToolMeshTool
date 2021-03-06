#ifndef __MESH_H__
#define __MESH_H__

// Core of this learned from:
//   http://ogldev.atspace.co.uk/www/tutorial22/tutorial22.html
//   http://ogldev.atspace.co.uk/www/tutorial38/tutorial38.html

#define MAX_BONES_PER_VERTEX    4 // Must be multiple of 3 or padding added to VertexFormat struct below.

struct VertexFormat
{
    Vector3 pos;
    Vector2 uv[AI_MAX_NUMBER_OF_TEXTURECOORDS];
    Vector3 norm;
    Vector3 tangent;
    unsigned char color[4];
    unsigned char boneIndices[MAX_BONES_PER_VERTEX]; // Max 256 bones. Engine code has max limit of 100 ATM anyway.
    float weights[MAX_BONES_PER_VERTEX];
};

struct Material
{
    std::string name;
    Vector4 colorAmbient;
    Vector4 colorDiffuse;
    Vector4 colorSpecular;
};

struct MeshChunk
{
    std::vector<VertexFormat> m_Vertices;
    std::vector<unsigned int> m_Indices;
    unsigned int m_MaterialIndex;
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
    std::vector<Material> m_Materials;
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
    void PullMaterialDataFromScene();
    void PullMeshDataFromScene();
    void PullBoneDataFromScene();

    bool IsNodeABone(aiNode* pNode);

    void ExportToFile(const char* filename, const char* materialDir);
    void ExportMaterials(const char* materialDir);
    int ExportNodeHeirarchyDataFromScene(cJSON* pParentNode, aiNode* pNode, int depth = 0);
    void ExportAnimationDataFromScene(cJSON* pAnimationArray);
    void DumpRawNodeTransformsFromScene(FILE* file);
    void DumpRawAnimationDataFromScene(FILE* file);
}; 

#endif //__MESH_H__

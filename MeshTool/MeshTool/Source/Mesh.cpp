#include "MeshToolPCH.h"

#include "Mesh.h"

Mesh::Mesh()
{
    m_pScene = nullptr;

    m_NumUVChannels = 0;
    m_HasNormals = false;
    m_HasTangents = false;
    m_HasBitangents = false;
    m_HasColor = false;
    m_MostBonesInfluences = 0;
}

Mesh::~Mesh()
{
}

void Mesh::LoadFromFile(const char* filename)
{
    // Load a model into an assimp scene.
    m_pScene = m_Importer.ReadFile( filename, 
                                    aiProcess_CalcTangentSpace |
                                    aiProcess_JoinIdenticalVertices |
                                    aiProcess_MakeLeftHanded |
                                    aiProcess_Triangulate |
                                    //aiProcess_RemoveComponent |
                                    aiProcess_GenNormals |
                                    //aiProcess_GenSmoothNormals |
                                    //aiProcess_SplitLargeMeshes |
                                    //aiProcess_PreTransformVertices |
                                    aiProcess_LimitBoneWeights |           // Limit of 4 bones influencing vertex.
                                    //aiProcess_ValidateDataStructure |
                                    //aiProcess_ImproveCacheLocality |
                                    //aiProcess_RemoveRedundantMaterials |
                                    //aiProcess_FixInfacingNormals |
                                    aiProcess_SortByPType |
                                    //aiProcess_FindDegenerates |
                                    //aiProcess_FindInvalidData |
                                    aiProcess_GenUVCoords |
                                    //aiProcess_TransformUVCoords |
                                    //aiProcess_FindInstances |
                                    //aiProcess_OptimizeMeshes |
                                    //aiProcess_OptimizeGraph |
                                    aiProcess_FlipUVs |
                                    aiProcess_FlipWindingOrder |
                                    //aiProcess_SplitByBoneCount |
                                    //aiProcess_Debone |
                                    0 );

    if( m_pScene == nullptr )
    {
        printf( "Importer.ReadFile(%s): %s\n", filename, m_Importer.GetErrorString() );
        return;
    }

    PullMaterialDataFromScene();
    PullMeshDataFromScene();
    PullBoneDataFromScene();
}

void Mesh::PullMaterialDataFromScene()
{
    unsigned int numMaterials = m_pScene->mNumMaterials;
    m_Materials.resize( numMaterials );

    for( unsigned int mati=0; mati<numMaterials; mati++ )
    {
        Material* pMaterial = &m_Materials[mati];
        aiMaterial* pSceneMaterial = m_pScene->mMaterials[mati];

        aiString matName = aiString( "unnamed" );
        pSceneMaterial->Get( AI_MATKEY_NAME, matName );
        pMaterial->name = matName.C_Str();

        aiColor3D colorAmbient( 1, 1, 1 );
        pSceneMaterial->Get( AI_MATKEY_COLOR_AMBIENT, colorAmbient );
        pMaterial->colorAmbient.Set( colorAmbient.r, colorAmbient.g, colorAmbient.b, 1 );

        aiColor3D colorDiffuse( 1, 1, 1 );
        pSceneMaterial->Get( AI_MATKEY_COLOR_DIFFUSE, colorDiffuse );
        pMaterial->colorDiffuse.Set( colorDiffuse.r, colorDiffuse.g, colorDiffuse.b, 1 );

        aiColor3D colorSpecular( 1, 1, 1 );
        pSceneMaterial->Get( AI_MATKEY_COLOR_SPECULAR, colorSpecular );
        pMaterial->colorSpecular.Set( colorSpecular.r, colorSpecular.g, colorSpecular.b, 1 );
    }
}

void Mesh::PullMeshDataFromScene()
{
    unsigned int numMeshes = m_pScene->mNumMeshes;
    m_MeshChunks.resize( numMeshes );

    printf( "===\n" );
    printf( "numMeshes: %d\n", numMeshes );

    // Initialize the meshes in the scene one by one.
    for( unsigned int mi=0; mi<numMeshes; mi++ )
    {
        aiMesh* pMesh = m_pScene->mMeshes[mi];

        printf( "mesh %d:\n", mi );
        //printf( "  numVerts: %d\n", pMesh->mNumVertices );
        //printf( "  numFaces: %d\n", pMesh->mNumFaces );
        //printf( "  numBones: %d\n", pMesh->mNumBones );

        MeshChunk* pMeshChunk = &m_MeshChunks[mi];

        pMeshChunk->m_MaterialIndex = pMesh->mMaterialIndex;
        
        // Deal with verts.
        {
            unsigned int numVerts = pMesh->mNumVertices;

            // Reserve enough space in our vector for the vertices.
            pMeshChunk->m_Vertices.resize( numVerts );

            for( unsigned int vi=0; vi<numVerts; vi++ )
            {
                // Typecasting should be safe as long as aiVector3D is simply 3 floats in order.
                memset( &pMeshChunk->m_Vertices[vi], 0, sizeof(VertexFormat) );

                if( pMesh->HasPositions() )
                {
                    pMeshChunk->m_Vertices[vi].pos = *(Vector3*)&pMesh->mVertices[vi];
                }
                if( pMesh->HasNormals() )
                {
                    pMeshChunk->m_Vertices[vi].norm = *(Vector3*)&pMesh->mNormals[vi];
                    m_HasNormals = true;
                }
                //if( pMesh->HasTangents() )
                //{
                //    pMeshChunk->m_Vertices[vi].tangent = *(Vector3*)&pMesh->mTangents[vi];
                //    m_HasTangents = true;
                //}
                if( pMesh->HasVertexColors( 0 ) )
                {
                    pMeshChunk->m_Vertices[vi].color[0] = (unsigned char)(pMesh->mColors[0][vi].r * 255);
                    pMeshChunk->m_Vertices[vi].color[1] = (unsigned char)(pMesh->mColors[0][vi].g * 255);
                    pMeshChunk->m_Vertices[vi].color[2] = (unsigned char)(pMesh->mColors[0][vi].b * 255);
                    pMeshChunk->m_Vertices[vi].color[3] = (unsigned char)(pMesh->mColors[0][vi].a * 255);
                    m_HasColor = true;
                }

                for( unsigned int i=0; i<AI_MAX_NUMBER_OF_TEXTURECOORDS; i++ )
                {
                    if( pMesh->HasTextureCoords( i ) )
                    {
                        pMeshChunk->m_Vertices[vi].uv[i]  = *(Vector2*)&pMesh->mTextureCoords[i][vi];

                        if( i+1 > m_NumUVChannels )
                            m_NumUVChannels = i+1;
                    }
                }
            }
        }

        // Deal with indices.
        {
            unsigned int numFaces = pMesh->mNumFaces;
            unsigned int numIndices = numFaces * 3;

            pMeshChunk->m_Indices.resize( numIndices );

            for( unsigned int fi=0; fi<numFaces; fi++ )
            {
                aiFace& Face = pMesh->mFaces[fi];

                assert( Face.mNumIndices == 3 );

                pMeshChunk->m_Indices[fi*3 + 0] = Face.mIndices[0];
                pMeshChunk->m_Indices[fi*3 + 1] = Face.mIndices[1];
                pMeshChunk->m_Indices[fi*3 + 2] = Face.mIndices[2];
            }
        }
    }
}

void Mesh::PullBoneDataFromScene()
{
    // Deal with bones.
    unsigned int numMeshes = m_pScene->mNumMeshes;

    for( unsigned int mi=0; mi<numMeshes; mi++ )
    {
        aiMesh* pMesh = m_pScene->mMeshes[mi];

        unsigned int numBones = pMesh->mNumBones;

        for( unsigned int bi=0; bi<numBones; bi++ )
        {
            Bone bone;
            bone.m_Name = pMesh->mBones[bi]->mName.data;
            bone.m_OffsetMatrix = *(MyMatrix*)&pMesh->mBones[bi]->mOffsetMatrix; // Typecasting should be safe as long as aiMatrix4x4 is simply 16 floats in order.
            bone.m_OffsetMatrix.Transpose();

            // Check that all bones influence at least one vertex.
            assert( pMesh->mBones[bi]->mNumWeights > 0 );

            // Check if this bone is already in our list.
            assert( numBones <= 256 );
            unsigned char ourBoneIndex = static_cast<unsigned char>( m_Bones.size() );
            for( unsigned char i=0; i<m_Bones.size(); i++ )
            {
                if( m_Bones[i].m_Name == bone.m_Name )
                {
                    ourBoneIndex = i;
                    assert( m_Bones[i].m_OffsetMatrix == bone.m_OffsetMatrix );
                }
            }
            if( ourBoneIndex == m_Bones.size() )
                m_Bones.push_back( bone );

            for( unsigned int wi=0; wi<pMesh->mBones[bi]->mNumWeights; wi++ )
            {
                unsigned int vi = pMesh->mBones[bi]->mWeights[wi].mVertexId;
                float weight = pMesh->mBones[bi]->mWeights[wi].mWeight;

                // Add this bone weighting to the vertex if necessary.
                for( unsigned int bwi=0; bwi<MAX_BONES_PER_VERTEX; bwi++ )
                {
                    if( m_MeshChunks[mi].m_Vertices[vi].weights[bwi] == 0 )
                    {
                        m_MeshChunks[mi].m_Vertices[vi].boneIndices[bwi] = ourBoneIndex;
                        m_MeshChunks[mi].m_Vertices[vi].weights[bwi] = weight;

                        if( bwi+1 > m_MostBonesInfluences )
                            m_MostBonesInfluences = bwi+1;

                        break;
                    }

                    // If this assert trips, then too many bones influence this vertex.
                    assert( bwi < MAX_BONES_PER_VERTEX );
                }
            }
        }
    }
}

bool Mesh::IsNodeABone(aiNode* pNode)
{
    for( unsigned int bi=0; bi<m_Bones.size(); bi++ )
    {
        if( m_Bones[bi].m_Name == pNode->mName.data )
        {
            return true;
        }
    }

    return false;
}

int Mesh::ExportNodeHeirarchyDataFromScene(cJSON* pParentNode, aiNode* pNode, int depth)
{
    int count = 1;

    m_NodeNames.push_back( pNode->mName.data );
    MyMatrix transform = *(MyMatrix*)&pNode->mTransformation;
    transform.Transpose();
    m_NodeTransforms.push_back( transform );

    cJSON* thisNode = cJSON_CreateObject();
    cJSON_AddItemToObject( pParentNode, pNode->mName.data, thisNode );

    //for( int i=0; i<depth; i++ )
    //    printf( " " );
    //printf( "%s\n", pNode->mName.data );

    for( unsigned int ni=0; ni<pNode->mNumChildren; ni++ )
    {
        count += ExportNodeHeirarchyDataFromScene( thisNode, pNode->mChildren[ni], depth+1 );
    }

    return count;
}

void Mesh::ExportAnimationDataFromScene(cJSON* pAnimationArray)
{
    unsigned int numAnims = m_pScene->mNumAnimations;

    for( unsigned int ai=0; ai<numAnims; ai++ )
    {
        cJSON* animation = cJSON_CreateObject();
        cJSON_AddItemToArray( pAnimationArray, animation );

        cJSON_AddStringToObject( animation, "Name", m_pScene->mAnimations[ai]->mName.data );
        cJSON_AddNumberToObject( animation, "Duration", m_pScene->mAnimations[ai]->mDuration );
        cJSON_AddNumberToObject( animation, "TicksPerSecond", m_pScene->mAnimations[ai]->mTicksPerSecond );
        cJSON_AddNumberToObject( animation, "NumChannels", m_pScene->mAnimations[ai]->mNumChannels );
    }
}

void Mesh::DumpRawNodeTransformsFromScene(FILE* file)
{
    fwrite( &m_NodeTransforms[0], sizeof(MyMatrix), m_NodeTransforms.size(), file );
}

void Mesh::DumpRawAnimationDataFromScene(FILE* file)
{
    unsigned int numAnims = m_pScene->mNumAnimations;

    // Anim data is typecast to int* in mesh loader, so must land on 4-byte boundary.
    int byteswritten = ftell( file );
    assert( byteswritten % 4 == 0 );

    for( unsigned int ai=0; ai<numAnims; ai++ )
    {
        unsigned int numChannels = m_pScene->mAnimations[ai]->mNumChannels;

        for( unsigned int ci=0; ci<numChannels; ci++ )
        {
            aiNodeAnim* pNodeAnim = m_pScene->mAnimations[ai]->mChannels[ci];

            // Write out the full node name.
            //fwrite( &pNodeAnim->mNodeName.length, sizeof(unsigned int), 1, file );
            //fwrite( &pNodeAnim->mNodeName.data, sizeof(char), pNodeAnim->mNodeName.length, file );
            // Write out the node index instead of the full node name.
            unsigned int ni;
            for( ni=0; ni<m_NodeNames.size(); ni++ )
            {
                if( strcmp( m_NodeNames[ni], pNodeAnim->mNodeName.data ) == 0 )
                    break;
            }
            assert( ni != m_NodeNames.size() );
            fwrite( &ni, sizeof(unsigned int), 1, file );

            // TODO: Eliminate duplicate keys.

            // Write out all positions.  time as a float, value as vector3.
            fwrite( &pNodeAnim->mNumPositionKeys, sizeof(unsigned int), 1, file );
            for( unsigned int ki=0; ki<pNodeAnim->mNumPositionKeys; ki++ )
            {
                float time = (float)pNodeAnim->mPositionKeys[ki].mTime;
                fwrite( &time, sizeof(float), 1, file );
            }
            for( unsigned int ki=0; ki<pNodeAnim->mNumPositionKeys; ki++ )
            {
                fwrite( &pNodeAnim->mPositionKeys[ki].mValue, sizeof(float)*3, 1, file );
            }

            // Write out all rotations.  time as a float, value as quaternion.
            fwrite( &pNodeAnim->mNumRotationKeys, sizeof(unsigned int), 1, file );
            for( unsigned int ki=0; ki<pNodeAnim->mNumRotationKeys; ki++ )
            {
                float time = (float)pNodeAnim->mRotationKeys[ki].mTime;
                fwrite( &time, sizeof(float), 1, file );
            }
            for( unsigned int ki=0; ki<pNodeAnim->mNumRotationKeys; ki++ )
            {
                fwrite( &pNodeAnim->mRotationKeys[ki].mValue.x, sizeof(float), 1, file );
                fwrite( &pNodeAnim->mRotationKeys[ki].mValue.y, sizeof(float), 1, file );
                fwrite( &pNodeAnim->mRotationKeys[ki].mValue.z, sizeof(float), 1, file );
                fwrite( &pNodeAnim->mRotationKeys[ki].mValue.w, sizeof(float), 1, file );
            }

            // Write out all scales.  time as a float, value as vector3.
            fwrite( &pNodeAnim->mNumScalingKeys, sizeof(unsigned int), 1, file );
            for( unsigned int ki=0; ki<pNodeAnim->mNumScalingKeys; ki++ )
            {
                float time = (float)pNodeAnim->mScalingKeys[ki].mTime;
                fwrite( &time, sizeof(float), 1, file );
            }
            for( unsigned int ki=0; ki<pNodeAnim->mNumScalingKeys; ki++ )
            {
                fwrite( &pNodeAnim->mScalingKeys[ki].mValue, sizeof(float)*3, 1, file );
            }

        }
    }
}

void AddPaddingToReachNext4ByteBoundary(FILE* file)
{
    int bytesWritten = ftell( file );
    while( bytesWritten % 4 != 0 )
    {
        fwrite( " ", 1, 1, file );
        bytesWritten++;
    }
}

void Mesh::ExportToFile(const char* filename, const char* materialDir)
{
    if( m_pScene == nullptr )
        return;

    unsigned int numMeshes = m_MeshChunks.size();
    unsigned int numMaterials = m_pScene->mNumMaterials;

    bool* MaterialsInUse = new bool[numMaterials];
    memset( MaterialsInUse, 0, sizeof(bool)*numMaterials );
    for( unsigned int mati=0; mati<numMaterials; mati++ )
    {
        for( unsigned int mi=0; mi<numMeshes; mi++ )
        {
            if( m_MeshChunks[mi].m_MaterialIndex == mati )
            {
                MaterialsInUse[mati] = true;
            }
        }
    }

    ExportMaterials( materialDir );

    // Create a json object.
    cJSON* root = cJSON_CreateObject();

    cJSON* meshArray = cJSON_CreateArray();
    cJSON_AddItemToObject( root, "Meshes", meshArray );

    // Export bones/nodes/anims commonly, not once per material.
    {
        unsigned int totalBones = m_Bones.size();

        cJSON_AddNumberToObject( root, "TotalBones", totalBones );

        // Add the bone names and matrices.
        cJSON* bones = cJSON_CreateArray();
        cJSON_AddItemToObject( root, "Bones", bones );
        for( unsigned int bi=0; bi<totalBones; bi++ )
        {
            //cJSON* bone = cJSON_CreateObject();
            //cJSON_AddStringToObject( bone, "Name", m_Bones[bi].m_Name.c_str() );
            //cJSONExt_AddFloatArrayToObject( bone, "Matrix", &m_Bones[bi].m_OffsetMatrix.m11, 16 );
            //cJSON_AddItemToArray( bones, bone );

            cJSON* boneName = cJSON_CreateString( m_Bones[bi].m_Name.c_str() );
            cJSON_AddItemToArray( bones, boneName );
        }

        int totalNodes = 0;
        if( m_pScene->mRootNode )
        {
            cJSON* nodes = cJSON_CreateObject();
            totalNodes = ExportNodeHeirarchyDataFromScene( nodes, m_pScene->mRootNode );
            cJSON_AddNumberToObject( root, "TotalNodes", totalNodes );
            cJSON_AddItemToObject( root, "Nodes", nodes );
        }

        if( m_pScene->mAnimations)
        {
            cJSON* animationArray = cJSON_CreateArray();
            cJSON_AddItemToObject( root, "AnimArray", animationArray );
            ExportAnimationDataFromScene( animationArray );
        }
    }

    for( unsigned int mati=0; mati<numMaterials; mati++ )
    {
        if( MaterialsInUse[mati] == false )
            continue;

        cJSON* mesh = cJSON_CreateObject();
        cJSON_AddItemToArray( meshArray, mesh );

        unsigned int totalVerts = 0;
        unsigned int totalIndices = 0;

        for( unsigned int mi=0; mi<numMeshes; mi++ )
        {
            if( m_MeshChunks[mi].m_MaterialIndex == mati )
            {
                totalVerts += m_MeshChunks[mi].m_Vertices.size();
                totalIndices += m_MeshChunks[mi].m_Indices.size();
            }
        }

        // Add the material index.
        char materialRelativePath[260];
        if( materialDir[0] != '\0' )
            sprintf_s( materialRelativePath, 260, "%s/%s.mymaterial", materialDir, m_Materials[mati].name.c_str() );
        else
            sprintf_s( materialRelativePath, 260, "%s.mymaterial", m_Materials[mati].name.c_str() );
        cJSON_AddStringToObject( mesh, "Material", materialRelativePath );

        // Add the vert/index/bone count.
        cJSON_AddNumberToObject( mesh, "TotalVerts", totalVerts );
        cJSON_AddNumberToObject( mesh, "TotalIndices", totalIndices );

        cJSONExt_AddNumberToObjectIfDiffers( mesh, "VF-uv", m_NumUVChannels, (unsigned int)0 );
        cJSONExt_AddNumberToObjectIfDiffers( mesh, "VF-normal", m_HasNormals, false );
        //cJSONExt_AddNumberToObjectIfDiffers( mesh, "VF-tangent", m_HasTangents, false );
        //cJSONExt_AddNumberToObjectIfDiffers( mesh, "VF-bitangent", m_HasBitangents, false );
        cJSONExt_AddNumberToObjectIfDiffers( mesh, "VF-color", m_HasColor, false );
        cJSONExt_AddNumberToObjectIfDiffers( mesh, "VF-mostweights", m_MostBonesInfluences, (unsigned int)0 );

        //// Quick debug, write out first meshchunks verts/indices as readable text.
        //{
        //    unsigned int numVertsInThisChunk = m_MeshChunks[0].m_Vertices.size();
        //    unsigned int numIndicesInThisChunk = m_MeshChunks[0].m_Indices.size();

        //    cJSON* verts = cJSON_CreateArray();
        //    cJSON_AddItemToObject( mesh, "verts", verts );

        //    for( unsigned int i=0; i<numVertsInThisChunk; i++ )
        //    {
        //        cJSON* pos = cJSON_CreateObject();
        //        cJSONExt_AddFloatArrayToObject( pos, "pos", &m_MeshChunks[0].m_Vertices[i].pos.x, 3 );
        //        cJSON_AddItemToArray( verts, pos );
        //    }

        //    cJSONExt_AddIntArrayToObject( mesh, "indices", (int*)&m_MeshChunks[0].m_Indices.front(), numIndicesInThisChunk );
        //}
    }

    // Save the json object to disk.
    //char* jsonString = cJSON_PrintUnformatted( root );
    char* jsonString = cJSON_Print( root );

    char outputFilename[260];
    size_t filenameLen = strlen(filename);
    if( filenameLen > 7 && strcmp( &filename[filenameLen-7], ".mymesh" ) == 0 )
        sprintf_s( outputFilename, 260, "%s", filename );
    else
        sprintf_s( outputFilename, 260, "%s.mymesh", filename );
    
    FILE* file;
#if MYFW_WINDOWS
    fopen_s( &file, outputFilename, "wb" );
#else
    file = fopen( outputfilename, "wb" );
#endif
    fprintf( file, "//\n" );
    fprintf( file, "//\n" );
    fprintf( file, "// Generated by MeshTool.exe, don't modify by hand... raw block must land of 4-byte boundary\n" );
    fprintf( file, "//\n" );
    fprintf( file, "//\n" );
    fprintf( file, "\n" );
    fprintf( file, "%s", jsonString );
    free( jsonString );

    // Raw data needs to land on 4-byte boundary.
    int bytesWritten = ftell( file ) + 1; // Add 1 for the \n that will be written before #RAW.
    while( bytesWritten % 4 != 0 )
    {
        fwrite( " ", 1, 1, file );
        bytesWritten++;
    }

    // Write out a marker for start of raw data.
    // Raw data format:
    //    - for each material/mesh chunk
    //      - vert dump(format listed in json struct)       - padded to start on 4-byte boundary
    //      - index dump(format based on number of verts)   - padded to start on 4-byte boundary
    //    - bone offset matrices                            - padded to start on 4-byte boundary
    //    - node transforms
    //    - animation data
    const char rawDelimiter[] = "\n#RAW";
    fwrite( rawDelimiter, 5, 1, file );

    bytesWritten = ftell( file );
    assert( bytesWritten % 4 == 0 );

    for( unsigned int mati=0; mati<numMaterials; mati++ )
    {
        unsigned int totalVerts = 0;
        unsigned int totalIndices = 0;

        for( unsigned int mi=0; mi<numMeshes; mi++ )
        {
            if( m_MeshChunks[mi].m_MaterialIndex == mati )
            {
                totalVerts += m_MeshChunks[mi].m_Vertices.size();
                totalIndices += m_MeshChunks[mi].m_Indices.size();
            }
        }

        // Raw dump of vertex info.
        for( unsigned int mi=0; mi<m_MeshChunks.size(); mi++ )
        {
            if( m_MeshChunks[mi].m_MaterialIndex != mati )
                continue;

            // Add padding to file.
            AddPaddingToReachNext4ByteBoundary( file );

            unsigned int numVertsInThisChunk = m_MeshChunks[mi].m_Vertices.size();

            for( unsigned int vi=0; vi<numVertsInThisChunk; vi++ )
            {
                fwrite( &m_MeshChunks[mi].m_Vertices[vi].pos, sizeof(Vector3), 1, file );

                for( unsigned int i=0; i<m_NumUVChannels; i++ )
                    fwrite( &m_MeshChunks[mi].m_Vertices[vi].uv[i], sizeof(Vector2), 1, file );

                if( m_HasNormals )
                    fwrite( &m_MeshChunks[mi].m_Vertices[vi].norm, sizeof(Vector3), 1, file );

                if( m_HasTangents )
                    fwrite( &m_MeshChunks[mi].m_Vertices[vi].tangent, sizeof(Vector3), 1, file );

                //if( m_HasBitangents )
                //    fwrite( &m_MeshChunks[mi].m_Vertices[vi].bitangent, sizeof(Vector3), 1, file );

                if( m_HasColor )
                    fwrite( &m_MeshChunks[mi].m_Vertices[vi].color, sizeof(unsigned char) * 4, 1, file );

                for( unsigned int i=0; i<m_MostBonesInfluences; i++ )
                    fwrite( &m_MeshChunks[mi].m_Vertices[vi].boneIndices[i], sizeof(unsigned char), 1, file );

                for( unsigned int i=0; i<m_MostBonesInfluences; i++ )
                    fwrite( &m_MeshChunks[mi].m_Vertices[vi].weights[i], sizeof(float), 1, file );
            }
        }

        // Raw dump of indices.
        int vertCount = 0;
        for( unsigned int mi=0; mi<m_MeshChunks.size(); mi++ )
        {
            if( m_MeshChunks[mi].m_MaterialIndex != mati )
                continue;

            // Add padding to file.
            AddPaddingToReachNext4ByteBoundary( file );

            unsigned int numVertsInThisChunk = m_MeshChunks[mi].m_Vertices.size();
        
            for( unsigned int i=0; i<m_MeshChunks[mi].m_Indices.size(); i++ )
            {
                if( totalVerts <= 256 ) // Write indices as unsigned chars.
                {
                    unsigned char index = vertCount + m_MeshChunks[mi].m_Indices[i];                
                    fwrite( &index, sizeof(unsigned char), 1, file );
                }
                else if( totalVerts <= 256*256 ) // Write indices as unsigned shorts.
                {
                    unsigned short index = vertCount + m_MeshChunks[mi].m_Indices[i];
                    fwrite( &index, sizeof(unsigned short), 1, file );
                }
                else // Write indices as unsigned ints.
                {
                    unsigned int index = vertCount + m_MeshChunks[mi].m_Indices[i];
                    fwrite( &index, sizeof(unsigned int), 1, file );
                }
            }

            vertCount += numVertsInThisChunk;
        }
    }

    // Add padding to file.
    AddPaddingToReachNext4ByteBoundary( file );

    // Dump more raw data to our file, these come after all verts/indices for each mesh material chunk.
    {
        // Raw dump of bone matrices.
        unsigned int totalBones = m_Bones.size();
        for( unsigned int bi=0; bi<totalBones; bi++ )
        {
            fwrite( &m_Bones[bi].m_OffsetMatrix.m11, sizeof(MyMatrix), 1, file );
        }
        DumpRawNodeTransformsFromScene( file );
        DumpRawAnimationDataFromScene( file );
    }

    fclose( file );

    cJSON_Delete( root );

    delete[] MaterialsInUse;
}

void Mesh::ExportMaterials(const char* materialDir)
{
    if( m_pScene == nullptr )
        return;

    unsigned int numMeshes = m_MeshChunks.size();
    unsigned int numMaterials = m_pScene->mNumMaterials;

    for( unsigned int mati=0; mati<numMaterials; mati++ )
    {
        bool materialsInUse = false;

        for( unsigned int mi=0; mi<numMeshes; mi++ )
        {
            if( m_MeshChunks[mi].m_MaterialIndex == mati )
            {
                materialsInUse = true;
                break;
            }
        }

        if( materialsInUse )
        {
            char filename[255];

            if( materialDir )
                sprintf_s( filename, 255, "%s/%s.mymaterial", materialDir, m_Materials[mati].name.c_str() );
            else
                sprintf_s( filename, 255, "%s.mymaterial", m_Materials[mati].name.c_str() );

            // Create a json object.
            cJSON* jRoot = cJSON_CreateObject();

            cJSON* jMaterial = cJSON_CreateObject();
            cJSON_AddItemToObject( jRoot, "Material", jMaterial );

            cJSON_AddStringToObject( jMaterial, "Name", m_Materials[mati].name.c_str() );
            //if( m_pShaderGroup )
            //    cJSON_AddStringToObject( jMaterial, "Shader", m_pShaderGroup->GetName() );
            //if( m_pTextureColor )
            //    cJSON_AddStringToObject( jMaterial, "TexColor", m_pTextureColor->m_Filename );

            cJSONExt_AddFloatArrayToObject( jMaterial, "ColorAmbient", &m_Materials[mati].colorAmbient.x, 4 );
            cJSONExt_AddFloatArrayToObject( jMaterial, "ColorDiffuse", &m_Materials[mati].colorDiffuse.x, 4 );
            cJSONExt_AddFloatArrayToObject( jMaterial, "ColorSpecular", &m_Materials[mati].colorSpecular.x, 4 );
            //cJSON_AddNumberToObject( jMaterial, "Shininess", m_Shininess );

            // Save the json object to disk.
            char* jsonString = cJSON_Print( jRoot );

            FILE* file;
#if MYFW_WINDOWS
            fopen_s( &file, filename, "wb" );
#else
            file = fopen( filename, "wb" );
#endif
            if( file != nullptr )
            {
                fprintf( file, "%s", jsonString );
                free( jsonString );

                fclose( file );
            }

            cJSON_Delete( jRoot );
        }
    }
}

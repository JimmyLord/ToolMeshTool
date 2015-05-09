#include "CommonHeader.h"

Mesh::Mesh()
{
    m_pScene = 0;

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
    // load a model into an assimp scene.
    m_pScene = m_Importer.ReadFile( filename, 
                                    aiProcess_CalcTangentSpace |
                                    aiProcess_JoinIdenticalVertices |
                                    //aiProcess_MakeLeftHanded |
                                    aiProcess_Triangulate |
                                    //aiProcess_RemoveComponent |
                                    aiProcess_GenNormals |
                                    //aiProcess_GenSmoothNormals |
                                    //aiProcess_SplitLargeMeshes |
                                    //aiProcess_PreTransformVertices |
                                    aiProcess_LimitBoneWeights |           // limit of 4 bones influencing vertex.
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
                                    //aiProcess_FlipWindingOrder |
                                    //aiProcess_SplitByBoneCount |
                                    //aiProcess_Debone |
                                    0 );

    if( m_pScene == 0 )
    {
        printf( "Importer.ReadFile(%s): %s\n", filename, m_Importer.GetErrorString() );
        return;
    }

    PullMeshDataFromScene();
    PullBoneDataFromScene();
}

void Mesh::PullMeshDataFromScene()
{
    unsigned int nummeshes = m_pScene->mNumMeshes;
    m_MeshChunks.resize( nummeshes );

    printf( "===\n" );
    printf( "nummeshes: %d\n", nummeshes );

    // Initialize the meshes in the scene one by one
    for( unsigned int mi=0; mi<nummeshes; mi++ )
    {
        aiMesh* pMesh = m_pScene->mMeshes[mi];

        printf( "mesh %d:\n", mi );
        //printf( "  numverts: %d\n", pMesh->mNumVertices );
        //printf( "  numfaces: %d\n", pMesh->mNumFaces );
        //printf( "  numbones: %d\n", pMesh->mNumBones );

        MeshChunk* pMeshChunk = &m_MeshChunks[mi];

        pMeshChunk->m_MaterialIndex = pMesh->mMaterialIndex;
        
        // deal with verts
        {
            unsigned int numverts = pMesh->mNumVertices;

            // Reserve enough space in our vector for the vertices.
            pMeshChunk->m_Vertices.resize( numverts );

            for( unsigned int vi=0; vi<numverts; vi++ )
            {
                // typecasting should be safe as long as aiVector3D is simply 3 floats in order.
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

        // deal with indices
        {
            unsigned int numfaces = pMesh->mNumFaces;
            unsigned int numindices = numfaces * 3;

            pMeshChunk->m_Indices.resize( numindices );

            for( unsigned int fi=0; fi<numfaces; fi++ )
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
    // deal with bones
    unsigned int nummeshes = m_pScene->mNumMeshes;

    for( unsigned int mi=0; mi<nummeshes; mi++ )
    {
        aiMesh* pMesh = m_pScene->mMeshes[mi];

        unsigned int numbones = pMesh->mNumBones;

        for( unsigned int bi=0; bi<numbones; bi++ )
        {
            Bone bone;
            bone.m_Name = pMesh->mBones[bi]->mName.data;
            bone.m_OffsetMatrix = *(MyMatrix*)&pMesh->mBones[bi]->mOffsetMatrix; // typecasting should be safe as long as aiMatrix4x4 is simply 16 floats in order.
            bone.m_OffsetMatrix.Transpose();

            // check that all bones influence at least one vertex.
            assert( pMesh->mBones[bi]->mNumWeights > 0 );

            // check if this bone is already in our list.
            assert( numbones <= 256 );
            unsigned char ourboneindex = m_Bones.size();
            for( unsigned char i=0; i<m_Bones.size(); i++ )
            {
                if( m_Bones[i].m_Name == bone.m_Name )
                {
                    ourboneindex = i;
                    assert( m_Bones[i].m_OffsetMatrix == bone.m_OffsetMatrix );
                }
            }
            if( ourboneindex == m_Bones.size() )
                m_Bones.push_back( bone );

            for( unsigned int wi=0; wi<pMesh->mBones[bi]->mNumWeights; wi++ )
            {
                unsigned int vi = pMesh->mBones[bi]->mWeights[wi].mVertexId;
                float weight = pMesh->mBones[bi]->mWeights[wi].mWeight;

                // add this bone weighting to the vertex if necessary.
                for( unsigned int bwi=0; bwi<MAX_BONES_PER_VERTEX; bwi++ )
                {
                    if( m_MeshChunks[mi].m_Vertices[vi].weights[bwi] == 0 )
                    {
                        m_MeshChunks[mi].m_Vertices[vi].boneindices[bwi] = ourboneindex;
                        m_MeshChunks[mi].m_Vertices[vi].weights[bwi] = weight;

                        if( bwi+1 > m_MostBonesInfluences )
                            m_MostBonesInfluences = bwi+1;

                        break;
                    }

                    // if this assert trips, then too many bones influence this vertex.
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

    cJSON* thisnode = cJSON_CreateObject();
    cJSON_AddItemToObject( pParentNode, pNode->mName.data, thisnode );

    //for( int i=0; i<depth; i++ )
    //    printf( " " );
    //printf( "%s\n", pNode->mName.data );

    for( unsigned int ni=0; ni<pNode->mNumChildren; ni++ )
    {
        count += ExportNodeHeirarchyDataFromScene( thisnode, pNode->mChildren[ni], depth+1 );
    }

    return count;
}

void Mesh::ExportAnimationDataFromScene(cJSON* pAnimationArray)
{
    unsigned int numanims = m_pScene->mNumAnimations;

    for( unsigned int ai=0; ai<numanims; ai++ )
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
    unsigned int numanims = m_pScene->mNumAnimations;

    for( unsigned int ai=0; ai<numanims; ai++ )
    {
        unsigned int numchannels = m_pScene->mAnimations[ai]->mNumChannels;

        for( unsigned int ci=0; ci<numchannels; ci++ )
        {
            aiNodeAnim* pNodeAnim = m_pScene->mAnimations[ai]->mChannels[ci];

            // write out the full node name.
            //fwrite( &pNodeAnim->mNodeName.length, sizeof(unsigned int), 1, file );
            //fwrite( &pNodeAnim->mNodeName.data, sizeof(char), pNodeAnim->mNodeName.length, file );
            // write out the node index instead of the full node name.
            unsigned int ni;
            for( ni=0; ni<m_NodeNames.size(); ni++ )
            {
                if( strcmp( m_NodeNames[ni], pNodeAnim->mNodeName.data ) == 0 )
                    break;
            }
            assert( ni != m_NodeNames.size() );
            fwrite( &ni, sizeof(unsigned int), 1, file );

            // TODO: eliminate duplicate keys

            // write out all positions.  time as a float, value as vector3.
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

            // write out all rotations.  time as a float, value as quaternion.
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

            // write out all scales.  time as a float, value as vector3.
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

void Mesh::ExportToFile(const char* filename)
{
    if( m_pScene == 0 )
        return;

    unsigned int nummeshes = m_MeshChunks.size();
    unsigned int nummaterials = m_pScene->mNumMaterials;

    bool* MaterialsInUse = new bool[nummaterials];
    memset( MaterialsInUse, 0, sizeof(bool)*nummaterials );
    for( unsigned int mati=0; mati<nummaterials; mati++ )
    {
        for( unsigned int mi=0; mi<nummeshes; mi++ )
        {
            if( m_MeshChunks[mi].m_MaterialIndex == mati )
            {
                MaterialsInUse[mati] = true;
            }
        }
    }

    // Create a json object.
    cJSON* root = cJSON_CreateObject();

    cJSON* mesharray = cJSON_CreateArray();
    cJSON_AddItemToObject( root, "Meshes", mesharray );

    // export bones/nodes/anims commonly, not once per material
    {
        unsigned int totalbones = m_Bones.size();

        cJSON_AddNumberToObject( root, "TotalBones", totalbones );

        // Add the bone names and matrices.
        cJSON* bones = cJSON_CreateArray();
        cJSON_AddItemToObject( root, "Bones", bones );
        for( unsigned int bi=0; bi<totalbones; bi++ )
        {
            //cJSON* bone = cJSON_CreateObject();
            //cJSON_AddStringToObject( bone, "Name", m_Bones[bi].m_Name.c_str() );
            //cJSONExt_AddFloatArrayToObject( bone, "Matrix", &m_Bones[bi].m_OffsetMatrix.m11, 16 );
            //cJSON_AddItemToArray( bones, bone );

            cJSON* bonename = cJSON_CreateString( m_Bones[bi].m_Name.c_str() );
            cJSON_AddItemToArray( bones, bonename );
        }

        int totalnodes = 0;
        if( m_pScene->mRootNode )
        {
            cJSON* nodes = cJSON_CreateObject();
            totalnodes = ExportNodeHeirarchyDataFromScene( nodes, m_pScene->mRootNode );
            cJSON_AddNumberToObject( root, "TotalNodes", totalnodes );
            cJSON_AddItemToObject( root, "Nodes", nodes );
        }

        if( m_pScene->mAnimations)
        {
            cJSON* animationarray = cJSON_CreateArray();
            cJSON_AddItemToObject( root, "AnimArray", animationarray );
            ExportAnimationDataFromScene( animationarray );
        }
    }

    for( unsigned int mati=0; mati<nummaterials; mati++ )
    {
        if( MaterialsInUse[mati] == false )
            continue;

        cJSON* mesh = cJSON_CreateObject();
        cJSON_AddItemToArray( mesharray, mesh );

        unsigned int totalverts = 0;
        unsigned int totalindices = 0;

        for( unsigned int mi=0; mi<nummeshes; mi++ )
        {
            if( m_MeshChunks[mi].m_MaterialIndex == mati )
            {
                totalverts += m_MeshChunks[mi].m_Vertices.size();
                totalindices += m_MeshChunks[mi].m_Indices.size();
            }
        }

        // Add the material index.
        cJSON_AddNumberToObject( mesh, "Material", mati );

        // Add the vert/index/bone count.
        cJSON_AddNumberToObject( mesh, "TotalVerts", totalverts );
        cJSON_AddNumberToObject( mesh, "TotalIndices", totalindices );

        cJSONExt_AddNumberToObjectIfDiffers( mesh, "VF-uv", m_NumUVChannels, (unsigned int)0 );
        cJSONExt_AddNumberToObjectIfDiffers( mesh, "VF-normal", m_HasNormals, false );
        //cJSONExt_AddNumberToObjectIfDiffers( mesh, "VF-tangent", m_HasTangents, false );
        //cJSONExt_AddNumberToObjectIfDiffers( mesh, "VF-bitangent", m_HasBitangents, false );
        cJSONExt_AddNumberToObjectIfDiffers( mesh, "VF-color", m_HasColor, false );
        cJSONExt_AddNumberToObjectIfDiffers( mesh, "VF-mostweights", m_MostBonesInfluences, (unsigned int)0 );

        //// quick debug, write out first meshchunks verts/indices as readable text.
        //{
        //    unsigned int numvertsinthischunk = m_MeshChunks[0].m_Vertices.size();
        //    unsigned int numindicesinthischunk = m_MeshChunks[0].m_Indices.size();

        //    cJSON* verts = cJSON_CreateArray();
        //    cJSON_AddItemToObject( mesh, "verts", verts );

        //    for( unsigned int i=0; i<numvertsinthischunk; i++ )
        //    {
        //        cJSON* pos = cJSON_CreateObject();
        //        cJSONExt_AddFloatArrayToObject( pos, "pos", &m_MeshChunks[0].m_Vertices[i].pos.x, 3 );
        //        cJSON_AddItemToArray( verts, pos );
        //    }

        //    cJSONExt_AddIntArrayToObject( mesh, "indices", (int*)&m_MeshChunks[0].m_Indices.front(), numindicesinthischunk );
        //}
    }

    // Save the json object to disk.
    //char* jsonstr = cJSON_PrintUnformatted( root );
    char* jsonstr = cJSON_Print( root );

    char outputfilename[260];
    size_t filenamelen = strlen(filename);
    if( filenamelen > 7 && strcmp( &filename[filenamelen-7], ".mymesh" ) == 0 )
        sprintf_s( outputfilename, 260, "%s", filename );
    else
        sprintf_s( outputfilename, 260, "%s.mymesh", filename );
    
    FILE* file;
    fopen_s( &file, outputfilename, "wb" );
    fprintf( file, jsonstr );
    free( jsonstr );

    // write out a marker for start of raw data
    // raw data format:
    //    - for each material/mesh chunk
    //      - vert dump(format listed in json struct)
    //      - index dump(format based on number of verts)
    //    - bone offset matrices
    //    - node transforms
    //    - animation data
    const char rawdelimiter[] = "\n#RAW";
    fwrite( rawdelimiter, sizeof(rawdelimiter), 1, file );

    for( unsigned int mati=0; mati<nummaterials; mati++ )
    {
        unsigned int totalverts = 0;
        unsigned int totalindices = 0;

        for( unsigned int mi=0; mi<nummeshes; mi++ )
        {
            if( m_MeshChunks[mi].m_MaterialIndex == mati )
            {
                totalverts += m_MeshChunks[mi].m_Vertices.size();
                totalindices += m_MeshChunks[mi].m_Indices.size();
            }
        }

        // raw dump of vertex info.
        for( unsigned int mi=0; mi<m_MeshChunks.size(); mi++ )
        {
            if( m_MeshChunks[mi].m_MaterialIndex != mati )
                continue;

            unsigned int numvertsinthischunk = m_MeshChunks[mi].m_Vertices.size();

            for( unsigned int vi=0; vi<numvertsinthischunk; vi++ )
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
                    fwrite( &m_MeshChunks[mi].m_Vertices[vi].boneindices[i], sizeof(unsigned char), 1, file );

                for( unsigned int i=0; i<m_MostBonesInfluences; i++ )
                    fwrite( &m_MeshChunks[mi].m_Vertices[vi].weights[i], sizeof(float), 1, file );
            }
        }

        // raw dump of indices.
        int vertcount = 0;
        for( unsigned int mi=0; mi<m_MeshChunks.size(); mi++ )
        {
            if( m_MeshChunks[mi].m_MaterialIndex != mati )
                continue;

            unsigned int numvertsinthischunk = m_MeshChunks[mi].m_Vertices.size();
        
            for( unsigned int i=0; i<m_MeshChunks[mi].m_Indices.size(); i++ )
            {
                if( totalverts <= 256 ) // write indices as unsigned chars
                {
                    unsigned char index = vertcount + m_MeshChunks[mi].m_Indices[i];                
                    fwrite( &index, sizeof(unsigned char), 1, file );
                }
                else if( totalverts <= 256*256 ) // write indices as unsigned shorts
                {
                    unsigned short index = vertcount + m_MeshChunks[mi].m_Indices[i];
                    fwrite( &index, sizeof(unsigned short), 1, file );
                }
                else // write indices as unsigned ints
                {
                    unsigned int index = vertcount + m_MeshChunks[mi].m_Indices[i];
                    fwrite( &index, sizeof(unsigned int), 1, file );
                }
            }

            vertcount += numvertsinthischunk;
        }
    }

    // dump more raw data to our file, these come after all verts/indices for each mesh material chunk.
    {
        // raw dump of bone matrices
        unsigned int totalbones = m_Bones.size();
        for( unsigned int bi=0; bi<totalbones; bi++ )
        {
            fwrite( &m_Bones[bi].m_OffsetMatrix.m11, sizeof(MyMatrix), 1, file );
        }
        DumpRawNodeTransformsFromScene( file );
        DumpRawAnimationDataFromScene( file );
    }

    fclose( file );

    cJSON_Delete( root );

    delete MaterialsInUse;
}

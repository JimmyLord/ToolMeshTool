#include "CommonHeader.h"

void Mesh::LoadFromFile(const char* filename)
{
    Assimp::Importer Importer;

    // load a model into an assimp scene.
    const aiScene* pScene = Importer.ReadFile( filename, 
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

    if( pScene == 0 )
    {
        printf( "Importer.ReadFile(%s): %s\n", filename, Importer.GetErrorString() );
        return;
    }

    unsigned int nummeshes = pScene->mNumMeshes;
    m_MeshChunks.resize( nummeshes );

    printf( "===\n" );
    printf( "nummeshes: %d\n", nummeshes );

    // Initialize the meshes in the scene one by one
    for( unsigned int mi=0; mi<nummeshes; mi++ )
    {
        aiMesh* pMesh = pScene->mMeshes[mi];

        printf( "mesh %d:\n", mi );
        //printf( "  numverts: %d\n", pMesh->mNumVertices );
        //printf( "  numfaces: %d\n", pMesh->mNumFaces );
        //printf( "  numbones: %d\n", pMesh->mNumBones );

        MeshChunk* pMeshChunk = &m_MeshChunks[mi];
        
        // deal with verts
        {
            unsigned int numverts = pMesh->mNumVertices;

            // Reserve enough space in our vector for the vertices.
            pMeshChunk->m_Vertices.resize( numverts );

            for( unsigned int vi=0; vi<numverts; vi++ )
            {
                // typecasting should be safe as long as aiVector3D is simply 3 floats in order.
                memset( &pMeshChunk->m_Vertices[vi], 0, sizeof(VertexFormat) );

                if( pMesh->mVertices )
                {
                    pMeshChunk->m_Vertices[vi].pos = *(Vector3*)&pMesh->mVertices[vi];
                }
                if( pMesh->mNormals )
                {
                    pMeshChunk->m_Vertices[vi].norm = *(Vector3*)&pMesh->mNormals[vi];
                    m_HasNormals = true;
                }
                //if( pMesh->mTangents )
                //{
                //    pMeshChunk->m_Vertices[vi].tangent = *(Vector3*)&pMesh->mTangents[vi];
                //    m_HasTangents = true;
                //}
                if( pMesh->mColors )
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

        // deal with bones
        {
            unsigned int numbones = pMesh->mNumBones;

            for( unsigned int bi=0; bi<numbones; bi++ )
            {
                Bone bone;
                bone.m_Name = pMesh->mBones[bi]->mName.data;
                bone.m_OffsetMatrix = *(MyMatrix*)&pMesh->mBones[bi]->mOffsetMatrix; // typecasting should be safe as long as aiMatrix4x4 is simply 16 floats in order.

                // check if this bone is already in our list.
                int ourboneindex = m_Bones.size();
                for( unsigned int i=0; i<m_Bones.size(); i++ )
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
}

void Mesh::ExportToFile(const char* filename)
{
    unsigned int nummeshes = m_MeshChunks.size();

    unsigned int totalverts = 0;
    unsigned int totalindices = 0;
    unsigned int totalbones = m_Bones.size();

    for( unsigned int mi=0; mi<nummeshes; mi++ )
    {
        totalverts += m_MeshChunks[mi].m_Vertices.size();
        totalindices += m_MeshChunks[mi].m_Indices.size();
    }

    // Create a json object.
    cJSON* root = cJSON_CreateObject();
    
    // Add the vert/index/bone count.
    cJSON_AddNumberToObject( root, "TotalVerts", totalverts );
    cJSON_AddNumberToObject( root, "TotalIndices", totalindices );
    cJSON_AddNumberToObject( root, "TotalBones", totalbones );

    cJSONExt_AddNumberToObjectIfDiffers( root, "VF-uv", m_NumUVChannels, (unsigned int)0 );
    cJSONExt_AddNumberToObjectIfDiffers( root, "VF-normal", m_HasNormals, false );
    //cJSONExt_AddNumberToObjectIfDiffers( root, "VF-tangent", m_HasTangents, false );
    //cJSONExt_AddNumberToObjectIfDiffers( root, "VF-bitangent", m_HasBitangents, false );
    cJSONExt_AddNumberToObjectIfDiffers( root, "VF-color", m_HasColor, false );
    cJSONExt_AddNumberToObjectIfDiffers( root, "VF-mostweights", m_MostBonesInfluences, (unsigned int)0 );

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

    //// quick debug, write out first meshchunks verts/indices as readable text.
    //{
    //    unsigned int numvertsinthischunk = m_MeshChunks[0].m_Vertices.size();
    //    unsigned int numindicesinthischunk = m_MeshChunks[0].m_Indices.size();

    //    cJSON* verts = cJSON_CreateArray();
    //    cJSON_AddItemToObject( root, "verts", verts );

    //    for( unsigned int i=0; i<numvertsinthischunk; i++ )
    //    {
    //        cJSON* pos = cJSON_CreateObject();
    //        cJSONExt_AddFloatArrayToObject( pos, "pos", &m_MeshChunks[0].m_Vertices[i].pos.x, 3 );
    //        cJSON_AddItemToArray( verts, pos );
    //    }

    //    cJSONExt_AddIntArrayToObject( root, "indices", (int*)&m_MeshChunks[0].m_Indices.front(), numindicesinthischunk );
    //}

    // Save the json object to disk.
    char* jsonstr = cJSON_Print( root );

    char outputfilename[260];
    sprintf_s( outputfilename, 260, "%s.mymesh", filename );
    
    FILE* file;
    fopen_s( &file, outputfilename, "wb" );

    // strip whitespace from json str.. keep newline's
    char* jsonstr_stripped;
    int len = strlen( jsonstr );
    jsonstr_stripped = new char[len+1];
    {
        int newlen = 0;
        for( int i=0; i<len; i++ )
        {
            if( jsonstr[i] != ' ' && jsonstr[i] != '\t' &&
                jsonstr[i] != '\r' ) //&& jsonstr[i] != '\n' )
            {
                jsonstr_stripped[newlen] = jsonstr[i];
                newlen++;
            }
        }
        jsonstr_stripped[newlen] = 0;
    }
    free( jsonstr );

    fprintf( file, jsonstr_stripped );
    delete jsonstr_stripped;

    // write out a marker for start of raw data
    const char rawdelimiter[] = "\n#RAW";
    fwrite( rawdelimiter, sizeof(rawdelimiter), 1, file ); 

    // raw dump of bone matrices
    for( unsigned int bi=0; bi<totalbones; bi++ )
    {
        fwrite( &m_Bones[bi].m_OffsetMatrix.m11, sizeof(MyMatrix), 1, file );
    }

    // raw dump of vertex info.
    for( unsigned int mi=0; mi<m_MeshChunks.size(); mi++ )
    {
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
                fwrite( &m_MeshChunks[mi].m_Vertices[vi].boneindices[i], sizeof(unsigned int), 1, file );

            for( unsigned int i=0; i<m_MostBonesInfluences; i++ )
                fwrite( &m_MeshChunks[mi].m_Vertices[vi].weights[i], sizeof(float), 1, file );
        }
    }

    // raw dump of indices.
    int vertcount = 0;
    for( unsigned int mi=0; mi<m_MeshChunks.size(); mi++ )
    {
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

    fclose( file );

    cJSON_Delete( root );
}

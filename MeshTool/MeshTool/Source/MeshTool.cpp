#include "MeshToolPCH.h"

#include "MeshTool.h"
#include "Mesh.h"

int main(int argc, char** argv)
{
    SettingsStruct settings;

    bool invalidArgs = MeshTool_ParseArgs( argc, argv, &settings );
    if( invalidArgs == true )
        return 1;

    Mesh* pMesh = new Mesh;

    pMesh->LoadFromFile( settings.sourceFilename );
    pMesh->ExportToFile( settings.outputFilename, settings.materialDir );

    delete pMesh;

    printf( "done\n" );

#if _DEBUG
    _getch();
#endif

    return 0;
}

bool MeshTool_ParseArgs(int argc, char** argv, SettingsStruct* pSettings)
{
    bool invalidArgs = false;

    if( argc < 2 )
        invalidArgs = true;

    for( int i=1; i<argc; i++ )
    {
        // treat the first arg as -s if - isn't the first character.
        if( i == 1 && strncmp( argv[i], "-", 1 ) != 0 )
        {
            pSettings->sourceFilename = argv[i];
        }
        if( ( strcmp( argv[i], "-s" ) == 0 || strcmp( argv[i], "-source" ) == 0 ) )
        {
            if( i+1 >= argc )
                invalidArgs = true;
            else
                pSettings->sourceFilename = argv[i+1];
        }
        if( ( strcmp( argv[i], "-o" ) == 0 || strcmp( argv[i], "-output" ) == 0 ) )
        {
            if( i+1 >= argc )
                invalidArgs = true;
            else
                pSettings->outputFilename = argv[i+1];
        }
        if( ( strcmp( argv[i], "-m" ) == 0 || strcmp( argv[i], "-materialdir" ) == 0 ) )
        {
            if( i+1 >= argc )
                invalidArgs = true;
            else
                pSettings->materialDir = argv[i+1];
        }
    }

    if( invalidArgs )
    {
        printf( "Invalid arguments\n" );
        printf( "\n" );
        printf( "[-s source filename] or -source = source filename\n" );
        printf( "[-o output filename] or -output = output filename\n" );
        printf( "[-m materials output dir] or -materialdir = materials output dir\n" );
        return true;
    }
    
    if( pSettings->sourceFilename == nullptr )
    {
        printf( "Source filename required - use -s\n" );
        return true;
    }
    
    if( pSettings->outputFilename == nullptr )
    {
        pSettings->outputFilename = pSettings->sourceFilename;
        //printf( "Output filename required - use -o\n" );
        //return true;
    }

    if( pSettings->materialDir == nullptr )
    {
        int bufferSize = strlen(pSettings->sourceFilename) + 1;
        pSettings->materialDir = new char[bufferSize];
        strcpy_s( pSettings->materialDir, bufferSize, pSettings->sourceFilename );

        int len = (int)strlen( pSettings->materialDir );
        int i;
        for( i=len; i>=0; i-- )
        {
            if( pSettings->materialDir[i] == '/' || pSettings->materialDir[i] == '\\' )
            {
                break;
            }
        }
        pSettings->materialDir[i] = '\0';
    }

    {
        printf( "Starting\n" );
        printf( "Source filename -> %s\n", pSettings->sourceFilename );
        printf( "Output filename -> %s\n", pSettings->outputFilename );
        printf( "Materials Output Dir -> %s\n", pSettings->materialDir );
    }

	return false;
}

#include "CommonHeader.h"

int main(int argc, char** argv)
{
    SettingsStruct settings;

    bool invalidargs = MeshTool_ParseArgs( argc, argv, &settings );
    if( invalidargs == true )
        return 1;

    Mesh* pMesh = new Mesh;

    pMesh->LoadFromFile( settings.sourcefilename );
    pMesh->ExportToFile( settings.outputfilename );

    delete pMesh;

    printf( "done\n" );

#if _DEBUG
    _getch();
#endif

    return 0;
}

bool MeshTool_ParseArgs(int argc, char** argv, SettingsStruct* pSettings)
{
    bool invalidargs = false;

    if( argc < 2 )
        invalidargs = true;

    for( int i=1; i<argc; i++ )
    {
        // treat the first arg as -s if - isn't the first character.
        if( i == 1 && strncmp( argv[i], "-", 1 ) != 0 )
        {
            pSettings->sourcefilename = argv[i];
        }
        if( ( strcmp( argv[i], "-s" ) == 0 || strcmp( argv[i], "-source" ) == 0 ) )
        {
            if( i+1 >= argc )
                invalidargs = true;
            else
                pSettings->sourcefilename = argv[i+1];
        }
        if( ( strcmp( argv[i], "-o" ) == 0 || strcmp( argv[i], "-output" ) == 0 ) )
        {
            if( i+1 >= argc )
                invalidargs = true;
            else
                pSettings->outputfilename = argv[i+1];
        }
    }
    
    if( invalidargs )
    {
        printf( "Invalid arguments\n" );
        printf( "\n" );
        printf( "[-s source filename] or -source = source filename\n" );
        printf( "[-o output filename] or -output = output filename\n" );
    }
    else if( pSettings->sourcefilename == 0 )
    {
        printf( "Source filename required - use -s\n" );
        invalidargs = true;
    }
    else if( pSettings->outputfilename == 0 )
    {
        pSettings->outputfilename = pSettings->sourcefilename;
        //printf( "Output filename required - use -o\n" );
    }
    else
    {
        printf( "Starting\n" );
        printf( "Source filename -> %s\n", pSettings->sourcefilename );
        printf( "Output filename -> %s\n", pSettings->outputfilename );
    }

	return invalidargs;
}

#ifndef __MESHTOOL_H__
#define __MESHTOOL_H__

struct SettingsStruct
{
    char* sourceFilename;
    char* outputFilename;
    char* materialDir;

    SettingsStruct()
    {
        sourceFilename = 0;
        outputFilename = 0;
        materialDir = 0;
    }

    ~SettingsStruct()
    {
    }
};

int main(int argc, char** argv);
bool MeshTool_ParseArgs(int argc, char** argv, SettingsStruct* pSettings);

#endif //__MESHTOOL_H__

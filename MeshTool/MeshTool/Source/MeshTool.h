#ifndef __MESHTOOL_H__
#define __MESHTOOL_H__

struct SettingsStruct
{
    char* sourcefilename;
    char* outputfilename;
    char* materialdir;

    SettingsStruct()
    {
        sourcefilename = 0;
        outputfilename = 0;
        materialdir = 0;
    }

    ~SettingsStruct()
    {
    }
};

int main(int argc, char** argv);
bool MeshTool_ParseArgs(int argc, char** argv, SettingsStruct* pSettings);

#endif //__MESHTOOL_H__

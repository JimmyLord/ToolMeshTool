#ifndef __MESHTOOL_H__
#define __MESHTOOL_H__

struct SettingsStruct
{
    const char* sourcefilename;
    const char* outputfilename;

    SettingsStruct::SettingsStruct()
    {
        sourcefilename = 0;
        outputfilename = 0;
    }
};

int main(int argc, char** argv);
bool MeshTool_ParseArgs(int argc, char** argv, SettingsStruct* pSettings);

#endif //__MESHTOOL_H__

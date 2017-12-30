#ifndef __COMMONHEADER_H__
#define __COMMONHEADER_H__

#if MESHTOOLGUI

#include <windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include "GL/GLExtensions.h"

#else

#if MYFW_WINDOWS
#include <SDKDDKVer.h>
#else
#define strcpy_s(a,b,c)             strcpy(a,c)
#define sprintf_s                   snprintf
#endif

#endif

#include <stdio.h>
#if MYFW_WINDOWS
#include <tchar.h>
#endif
#include <string.h>
#if MYFW_WINDOWS
#include <conio.h>
#endif
#include <stdlib.h>
#include <iostream>
#include <list>
#include <assert.h>
#include <vector>

#if MYFW_WINDOWS
#include <boost/filesystem.hpp>
#endif

#define PI 3.1415926535897932384626433832795f

#include "cJSON/cJSON.h"
#include "lodepng/lodepng.h"
#include "../../../assimp/include/assimp/mesh.h"
#include "../../../assimp/include/assimp/Importer.hpp"
#include "../../../assimp/include/assimp/scene.h"
#include "../../../assimp/include/assimp/postprocess.h"
//#include "../../../assimp/code/AssimpPCH.h"

#include "Framework/Utility.h"
#include "Framework/Vector.h"
#include "Framework/MyMatrix.h"
#include "Framework/cJSONHelpers.h"

#include "Mesh.h"
#include "MeshTool.h"

#if MESHTOOLGUI
#include "ShaderProgram.h"
#include "GameCore.h"
#endif

#endif //__COMMONHEADER_H__

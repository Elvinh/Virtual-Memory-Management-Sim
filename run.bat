rem
rem -- Make sure you remove stdafx.h if your mmc.cpp contains "#include "stdafx.h"
rem -- To compile use "cl mmc.cpp" or "cl mmc.cpp /clr"
rem 
cl -c vmms.cpp
link vmms.obj /DLL /SECTION:.SHARED,RWS
cl mmc.cpp vmms.lib 
cl test1.cpp vmms.lib
cl test2.cpp vmms.lib
rem
rem -- Use dumpbin command/tool to examine exe or dll headers.
rem dumpbin /headers vmms.dll

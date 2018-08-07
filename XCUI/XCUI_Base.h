#ifndef _XCUI_BASE_H_
#define _XCUI_BASE_H_

#include "XCUI.config"
#define _plat               //as platform code
#define _event virtual      //as object callback event

int _plat GetWindowSize(int &wide,int &height);
int _plat SetResizeHook();


#endif

//GenerateConsoleCtrlEvent
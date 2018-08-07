#include"XCUI_Base.h"
#ifdef XCUI_OS_LINUX

#include<sys/types.h>  
#include<sys/ioctl.h>  
#include<unistd.h>  
#include<termios.h> 
#include<stdlib.h>
#include<signal.h>
#include"XCUI.h"

 int _plat GetWindowSize(int&wide,int&height){
    struct winsize size;
    ioctl(STDIN_FILENO,TIOCGWINSZ,&size);
    wide=size.ws_col;
    height=size.ws_row;
    return 0;
 }
void _plat WindowSizeChangeHandler();
int _plat SetResizeHook(){
    signal(SIGWINCH,WindowSizeChangeHandler);
}

void _plat WindowSizeChangeHandler(){
    if(xcui_commondata.flg_onshow){
        int a,b;
        GetWindowSize(a,b);
        xcui_commondata.window.x=a;
        xcui_commondata.window.y=b;
        Refresh();
    }
}



#endif

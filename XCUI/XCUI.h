#ifndef _XCUI_H_
#define _XCUI_H_
#include<string>
#include<vector>
using namespace std;

#pragma region CommonData
struct rect{
    int x;
    int y;
};
typedef rect point;
struct xcui_commondata_t{
    bool flg_onshow;
    rect window;
}xcui_commondata;
#pragma endregion

#pragma region XcuiBase
class WidgetBase{
public:
	string uniqueID;            //Must be unique in one layer
    bool active;
	bool enable;
	bool visble;

	point pos;

	int _event OnLoad()={};
	int _event OnShow()={};
	int _event On

    virtual int  DrawBuf()={};
    virtual void* Widgetctl(WidgetBase& src,string& info)={};   //for widget interactive

};


class LayerBase{
public:
	string titleText;
	vector<WidgetBase> options;		//option text
	string uniqueID;			//Must be unique.
	int numOfStatus;

	int highLightedIndex;   //0 < this < numOfStatus

	virtual void DrawMenu();					//菜单绘制，允许重载
	virtual void StatusMove(int status) = 0;	//状态机跳转,必须提供重载
	virtual void beginOutPut(){};				//绘制菜单选项前输出信息，允许重载
	virtual void endOutPut(){};					//绘制菜单选项后输出信息，允许重载

	LayerBase(string _titletext, vector<string>& _options, vector<string>& _infos){
		titleText = titleText;
		options = _options;
		infos = _infos;
		numOfStatus = options.size();
		highLightedIndex = 0;
	}
	LayerBase(){;}

};

#pragma endregion


void Refresh();
void DrawLayer(LayerBase& layer);


#endif

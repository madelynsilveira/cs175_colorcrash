/*  =================== File Information =================
	File Name: main.cpp
	Description:
	Author: Michael Shah

	Purpose: Driver for 3D program to load .ply models
	Usage:
	===================================================== */

#include <string>
#include <iostream>
#include <fstream>
#include <math.h>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/names.h>
#include <FL/Fl_Box.H>
#include <Fl/Fl_PNG_Image.H>

#include "MyGLCanvas.h"
#include "GameManager.h"



using namespace std;

class MyAppWindow;
MyAppWindow* win;

class MyAppWindow : public Fl_Window {
public:
	MyGLCanvas* canvas;
	GameManager* gm;
public:
	// APP WINDOW CONSTRUCTOR
	MyAppWindow(int W, int H, const char* L = 0);

	static void idleCB(void* userdata) {
		win->gm->GetInstance()->GameUpdate();
		win->canvas->redraw();
	}
};


MyAppWindow::MyAppWindow(int W, int H, const char* L) : Fl_Window(W, H, L) {
	begin();
	// OpenGL window (5, 5, w() - 10, h() - 210);
	canvas = new MyGLCanvas(5, 5, w() - 10, w() - 10);
	gm->GetInstance()->glc = canvas;

	// set textbox location
	Fl_Pack* pack = new Fl_Pack(5, h() - (h() - w() - 10), w() - 10, 200, "");
	pack->box(FL_DOWN_FRAME);
	pack->type(Fl_Pack::VERTICAL);
	pack->spacing(30);
	pack->begin();

	Fl_Pack* envpack = new Fl_Pack(h() - 200, h() - w() + 20, w(), 200, "Game Rules");
	envpack->box(FL_DOWN_FRAME);
	envpack->labelfont(1);
	envpack->type(Fl_Pack::VERTICAL);
	envpack->spacing(0);
	envpack->begin();

	Fl_Box* texTextbox = new Fl_Box(0, 0, pack->w() - 20, 100, "Navigate Butter the bug to the correct colored wall.\nIf there is no wall his color, you'll have to swap colors!\nToggle colors with 'A' 'S' 'D', for RED, BLUE, and YELLOW.\nBy all means possible, avoid the black walls!");
	
	Fl_PNG_Image* red = new Fl_PNG_Image("red.png");
	Fl_Box* redBox = new Fl_Box(10,h() - 10, 20, 20);
	redBox->image(red);
	envpack->end();
	
}

/**************************************** main() ********************/
int main(int argc, char** argv) {
	win = new MyAppWindow(475, 600, "COLOR CRASH!");
	win->resizable(win);
	Fl::add_idle(MyAppWindow::idleCB);
	win->show();
	return(Fl::run());
}

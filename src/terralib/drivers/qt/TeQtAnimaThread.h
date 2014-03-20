#ifndef  __TERRALIB_INTERNAL_QTANIMATHREAD_H
#define  __TERRALIB_INTERNAL_QTANIMATHREAD_H
#include <qthread.h>
#include<qdir.h>
#include<qstringlist.h>

#include "../../kernel/TeDefines.h"

class TeQtCanvas;
class Animation;

class TL_DLL TeQtAnimaThread : public QThread
{
public:
	TeQtAnimaThread(){
		canvas_ = 0;
		pause_ = false;
		stop_ = true;
		rate_ = 1;
		frame_ = 0;
		frames_ = 0;
		pauseFrame_ = 0;
	}

	void run();
	void stop();
	void pause(bool b) {pause_ = b;}
	bool pause() {return pause_;}
	void frame(int n) {frame_ = n;}
	int frame() {return frame_;}
	void frames(int n) {frames_ = n;}
	int frames() {return frames_;}
	int pauseFrame(int pf) {return pauseFrame_ = pf;}
	void canvas(TeQtCanvas* c) {canvas_ = c;}
	void setAnimation(Animation* a, QDir d);
	void showFrame(int i);

private:
	bool		pause_;
	bool		stop_;
	int			rate_;
	int			frame_;
	int			frames_;
	int			pauseFrame_;
	TeQtCanvas*	canvas_;
	Animation*	animation_;
	QDir		dir_;
	QStringList	fileList_;
	void sleep(int t);
};
#endif

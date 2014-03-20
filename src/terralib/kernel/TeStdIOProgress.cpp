#include "TeStdIOProgress.h"

#include <iostream>

using std::string;
using std::cout;

TeStdIOProgress::TeStdIOProgress() :
	totalSteps_(0),
	curProg_(0)
{
}


TeStdIOProgress::~TeStdIOProgress()
{
}


void TeStdIOProgress::setTotalSteps(int n)
{ 
  totalSteps_ = n; 
  curProg_=0;
}


void TeStdIOProgress::reset()
{ 
	totalSteps_ = 0; 
	messageStr_ = "";
	captionStr_ = "";
	curProg_ = 0;
}


void TeStdIOProgress::cancel()
{ 
	totalSteps_ = 0; 
	messageStr_ = "";
	captionStr_ = "";
	curProg_ = 0;
}


void TeStdIOProgress::setMessage(const string& text)
{ 
  messageStr_ = text;
}


string TeStdIOProgress::getMessage() 
{ 
  return messageStr_; 
}


bool TeStdIOProgress::wasCancelled()
{ 
  return false; 
}
	

void TeStdIOProgress::setCaption(const string& cap)
{ 
  captionStr_ = cap;
}


void TeStdIOProgress::setProgress(int steps)
{
	int prog = (int)(100*steps/(totalSteps_+1));
	if (prog != curProg_)
	{
		cout << messageStr_ << " : " << prog << "%...\n";
		curProg_ = prog;
	}
}


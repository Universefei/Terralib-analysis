#include "TeDecoderMrSID.h"
#include "TeMrSID.h"
#include "../../kernel/TeDecoderMemory.h"
#include "../../kernel/TeVectorRemap.h"
#include "../../kernel/TeAsciiFile.h"
#include "../../kernel/TeGeometryAlgorithms.h"



TeDecoderMrSID::TeDecoderMrSID(const TeRasterParams& par) :
	TeDecoder(par),
	mrsid_(0),
	nSelectedBlocks_(0)
{
	params_ = par; 
	params_.decoderIdentifier_= "MrSID";
	params_.mode_ = 'r';
	mrsid_ = new TeMrSIDReader(par.fileName_.c_str());
	if (mrsid_)
	{
		params_.nBands(mrsid_->nBands());
		unsigned int w, h;
		mrsid_->getDimensions(w,h);
		params_.ncols_=w; params_.nlines_=h;
		vector<double> vv;
		mrsid_->getNavigationParameters(vv);
		params_.setNavigationParameters(vv);
		params_.projection(readProjection());
		params_.setDataType(TeUNSIGNEDCHAR);
		if(mrsid_->getColorModel() == TeMrSIDReader::ColorModelRGB)
			params_.setPhotometric(TeRasterParams::TeRGB);
		else
			params_.setPhotometric(TeRasterParams::TeMultiBand);
	    readMetadataInformation();
	}
}

TeDecoderMrSID::TeDecoderMrSID(const std::string& fname):
	mrsid_(0),
	nSelectedBlocks_(0)
{
	params_.fileName_ = fname;
	params_.mode_ = 'r';
	params_.decoderIdentifier_= "MrSID";
	mrsid_ = new TeMrSIDReader(fname.c_str());
	if (mrsid_)
	{
		params_.nBands(mrsid_->nBands());
		unsigned int w, h;
		mrsid_->getDimensions(w,h);
		params_.ncols_=w; params_.nlines_=h;
		vector<double> vv;
		mrsid_->getNavigationParameters(vv);
		params_.setNavigationParameters(vv);
		params_.setDataType(TeUNSIGNEDCHAR);
		if(mrsid_->getColorModel() == TeMrSIDReader::ColorModelRGB)
			params_.setPhotometric(TeRasterParams::TeRGB);
		else
			params_.setPhotometric(TeRasterParams::TeMultiBand);
	    readMetadataInformation();
	}
}

TeDecoderMrSID::~TeDecoderMrSID()
{
	clear();
	if (mrsid_)
	{
		delete mrsid_;
		mrsid_ = 0;
	}
}

void TeDecoderMrSID::init()
{
	clear();
	params_.status_= TeRasterParams::TeNotReady;

	if (params_.mode_ == 'r') // only use of this decoder....	
	{
		params_.status_ = TeRasterParams::TeReadyToRead;
		if (mrsid_)
		{
			if (mrsid_->fileName().empty() || mrsid_->fileName() != params_.fileName_)
			{
				delete mrsid_;
				mrsid_ = 0;
			}
		}
		if (!mrsid_)
		{
			try
			{
				params_.nBands(mrsid_->nBands());
				unsigned int w, h;
				mrsid_->getDimensions(w,h);
				params_.ncols_=w; params_.nlines_=h;
				vector<double> vv;
				mrsid_->getNavigationParameters(vv);
				params_.setNavigationParameters(vv);
				params_.setDataType(TeUNSIGNEDCHAR);
				if(mrsid_->getColorModel() == TeMrSIDReader::ColorModelRGB)
					params_.setPhotometric(TeRasterParams::TeRGB);
				else
					params_.setPhotometric(TeRasterParams::TeMultiBand);
				readMetadataInformation();
			}
			catch(...)
			{
				return;
			}
		}
		params_.status_= TeRasterParams::TeReadyToRead;
	}
	return;
}

void
TeDecoderMrSID::readMetadataInformation()
{
	if (!mrsid_)
		return;

	int i=0;
	vector<double> aux;
	if (mrsid_->getNoDataPixel(aux))
	{
		for (i=0; i<params_.nBands(); ++i)
			params_.setDummy(aux[i],i);
	}
	if (mrsid_->getMaxValues(aux))
	{
		for (i=0; i<params_.nBands(); ++i)
			params_.vmax_[i] = aux[i];
	}
	if (mrsid_->getMinValues(aux))
	{
		for (i=0; i<params_.nBands(); ++i)
			params_.vmin_[i] = aux[i];
	}
}

TeProjection* 
TeDecoderMrSID::readProjection()
{
	TeProjection* proj = 0;
	try
	{
		string projFile = TeGetName(params_.fileName_.c_str()) + ".prj";
		TeAsciiFile	pFile (projFile);
		string name;
		string pname, dname;
		int zone;
		TeProjectionParams projPar;
		while (pFile.isNotAtEOF())
		{
			name = pFile.readString();
			if (name == "Projection")
			{
				pname = pFile.readString();
			}	
			else if (name == "Datum")
			{
				name = pFile.readString();
				if (name == "SOUTH_AMERICAN_1969")
					dname = "SAD69";
				if (name == "WGS84")
					dname = "WGS84";

			}
			else if (name == "Zone")
			{
				zone = pFile.readInt();
			}
		}
		if (pname == "UTM" && (dname == "SAD69" || "WGS84"))
		{
			TeDatum d = TeDatumFactory::make(dname);
			proj = new TeUtm(d,zone*TeCDR);
		}
	}
	catch(...)
	{
	}
	if (!proj)
		proj = new TeNoProjection();
	return proj;
}

bool 
TeDecoderMrSID::clear()
{
	nSelectedBlocks_ = 0;
	params_.status_= TeRasterParams::TeNotReady;
	return true;
}

int 
TeDecoderMrSID::bestResolution(double res)
{
	return mrsid_->bestResolution(res);
}


int 
TeDecoderMrSID::bestResolution(TeBox& bb, int ncols, int nlines, TeProjection* proj)
{
	TeBox box = bb;
	if (proj)
		box = TeRemapBox(bb, proj, params_.projection());
	return (mrsid_->getWorstResolutionLevel(ncols, nlines, bb.x1(), bb.y2(), bb.x2(), bb.y1()));
}


bool TeDecoderMrSID::selectBlocks(TeBox& bbox, int resLevel, TeRasterParams& parBlock) 
{
	// check if desired resolution level is between the range of available levels
	if (resLevel < 0 || resLevel >= mrsid_->getNumLevels())
		return false;

	nSelectedBlocks_ = 0;
	// calculates width and height of the selected area

	double x1 = bbox.x1_;
	double y1 = bbox.y1_;
	double x2 = bbox.x2_;
	double y2 = bbox.y2_;

	if (!mrsid_->selectArea(resLevel,x1,y1,x2,y2))
		return false;

	params_.blockWidth_ = mrsid_->getSelectedAreaWidth();
	params_.blockHeight_ = mrsid_->getSelectedAreaHeight();

	// defines parameters of the selected block
	nSelectedBlocks_ = 1;			// always select only one block
	parBlock = params_;
	parBlock.boundingBoxLinesColumns(x1,y1,x2,y2,params_.blockHeight_,params_.blockWidth_);
	parBlock.interleaving_ = TeRasterParams::TePerBand;
	return true;
}

bool TeDecoderMrSID::getSelectedRasterBlock(TeDecoderMemory* memDec) 
{
  if (!mrsid_ || nSelectedBlocks_ <= 0)
	  return false;
  unsigned char* data = (unsigned char*) memDec->data(0);
  bool res = mrsid_->getSelectedAreaData(data);
  nSelectedBlocks_--;
  return res;
}

void 
TeDecoderMrSID::clearBlockSelection()
{	
	nSelectedBlocks_ = 0;	
}

TeDecoderMrSIDFactory::TeDecoderMrSIDFactory(const string& name): 
	TeDecoderFactory(name) 
{
	TeDecoderFactory::instanceName2Dec()["SID"]  = "MrSID";	
}
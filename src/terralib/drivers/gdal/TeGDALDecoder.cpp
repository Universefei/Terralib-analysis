#include "TeGDALDecoder.h"
#include "TeGDALUtils.h"

// GDAL
#include <cpl_string.h>
#include <gdal_priv.h>

// Static drivers with creation support map
std::vector<TeGDALDriverInfo> TeGDALDecoder::_gdalDrivers;

TeGDALDecoder::TeGDALDecoder() :
_gdalDS(0)
{}

TeGDALDecoder::TeGDALDecoder(const TeRasterParams& par) :
_gdalDS(0)
{
	params_ = par;
	params_.decoderIdentifier_ = "GDAL";
	params_.errorMessage_.clear();
}

TeGDALDecoder::~TeGDALDecoder() 
{
	if(_gdalDS != 0)
		clear();
}

void TeGDALDecoder::init()
{
	params_.status_= TeRasterParams::TeNotReady;
	
	char mode = params_.mode_;
	if(mode != 'c' && mode != 'r' && mode != 'w')
	{
		params_.errorMessage_ = "Invalid access mode! Try 'c' for creation, 'r' for read and 'w' for update.";
		return;
	}

	if(mode == 'r' || mode == 'w') // Read & Write
	{
		GDALAccess access = GA_ReadOnly;
		if(mode == 'w')
			access = GA_Update;
		
		// Opens the raster file
		_gdalDS = (GDALDataset*)GDALOpen(params_.fileName_.c_str(), access);
		if(_gdalDS == 0)
		{
			params_.errorMessage_ = "Fail to open the raster file:" + params_.fileName_ + "\n";
			params_.errorMessage_ += CPLGetLastErrorMsg();
			return;
		}

		Convert2TerraLib(_gdalDS, params_);
		
		if(mode == 'r')
			params_.status_ = TeRasterParams::TeReadyToRead;
		else
			params_.status_ = TeRasterParams::TeReadyToWrite;

		if(params_.blockWidth_ == 0)
			params_.blockWidth_ = params_.ncols_;

		if(params_.blockHeight_ == 0)
			params_.blockHeight_ = params_.nlines_;

		TeDecoderVirtualMemory::init();
	}
	else if(mode == 'c') // Creation
	{
		if(!params_.box().isValid())
			params_.lowerLeftResolutionSize(0.5, 0.5, 1, 1, params_.ncols_, params_.nlines_);

		// Creates the GDAL raster
		_gdalDS = CreateGDALRaster(params_);
		if(_gdalDS == 0)
		{
			params_.errorMessage_ = "Fail to create the raster file:" + params_.fileName_ + "\n";
			params_.errorMessage_ += CPLGetLastErrorMsg();
			return;
		}
		params_.status_ = TeRasterParams::TeReadyToWrite;
	}
}

bool TeGDALDecoder::clear()
{
	TeDecoderVirtualMemory::clear();
	
	GDALClose((GDALDataset*)_gdalDS);
	_gdalDS = 0;
	
	return true;
} 

bool TeGDALDecoder::setElement(int col, int lin, double val, int band)
{
	GDALRasterBand* rasterBand = _gdalDS->GetRasterBand(band+1);
	
	// Write the pixel value
	CPLErr error = rasterBand->RasterIO(GF_Write, col, lin, 1, 1, &val, 1, 1, GDT_Float64, 0, 0);
	if(error == CE_Failure)
		return false;

	return true;
}

//bool TeGDALDecoder::getElement(int col, int lin, double& val, int band)
//{
//	GDALRasterBand* rasterBand = _gdalDS->GetRasterBand(band+1);
//
//	// Reads the pixel value
//	CPLErr error = rasterBand->RasterIO(GF_Read, col, lin, 1, 1, &val, 1, 1, GDT_Float64, 0, 0);
//	if(error == CE_Failure)
//		return false;
//
//	return true;
//}

void TeGDALDecoder::initialize()
{
	GDALDriverManager* driverManager = GetGDALDriverManager();
	if(driverManager->GetDriverCount() != 0)
		return;

	// Registers all format drivers built into GDAL.
	GDALAllRegister();

	char** gdalDriverMetadata; // to drivers metadata
	int nDrivers = driverManager->GetDriverCount();
	for(int i = 0; i < nDrivers; ++i) // for each driver
	{
		// Gets the i-th driver
		GDALDriver* driver = driverManager->GetDriver(i);
		if(driver == 0)
			continue;

		// Gets the i-th driver metadata
		gdalDriverMetadata = driver->GetMetadata();
		if(gdalDriverMetadata == 0)
			continue;

		// GDAL driver identify
		std::string driverid = driver->GetDescription();

		// Builds the GDAL driver info
		TeGDALDriverInfo dinfo;
		dinfo.setId(driverid);

		// Try fecth the extension associated with this driver...
		const char* ext = CSLFetchNameValue(gdalDriverMetadata, GDAL_DMD_EXTENSION);
		if(ext != 0)
			dinfo.setExtension(ext);

		// Try fecth the driver full name...
		const char* fn = CSLFetchNameValue(gdalDriverMetadata, GDAL_DMD_LONGNAME);
		if(fn != 0)
			dinfo.setFullName(fn);

		// Has creation support?
		bool createSupport = CSLFetchBoolean(gdalDriverMetadata, GDAL_DCAP_CREATE, FALSE);
		dinfo.setCreateSupport(createSupport);

		_gdalDrivers.push_back(dinfo);
	}
}

std::string TeGDALDecoder::getGDALDriverName(const std::string& extension)
{
	if(extension.empty())
		return "";

	for(unsigned int i = 0; i < _gdalDrivers.size(); ++i)
	{
		std::string myExtension = _gdalDrivers[i].getExtension();
		if(myExtension == extension)
			return _gdalDrivers[i].getId();
	}

	return ""; // driver not found with the given extension
}

const std::vector<TeGDALDriverInfo>& TeGDALDecoder::getGDALDrivers()
{
	return _gdalDrivers;
}

std::string TeGDALDecoder::getSupportedFilesFilter()
{
	std::string filter = "All (*.*)";
	for(unsigned int i = 0; i < _gdalDrivers.size(); ++i)
	{
		std::string driverFilter = _gdalDrivers[i].getFilter();
		filter += ";;" + driverFilter;
	}
	return filter;
}

bool TeGDALDecoder::getRasterBlock(const TeBlockIndex& index, void* buf)
{
	GDALDataType gtype = Convert2GDAL(params_.dataType_[0]);

	int ulCol, ulLin, band;
	blockIndexPos(index, ulCol, ulLin, band);

	// Gets the GDAL raster band
	GDALRasterBand* rasterBand = _gdalDS->GetRasterBand(band+1);

	//rounds the block values in order to query the data
	int blockWidth = params_.blockWidth_;
	int blockHeight = params_.blockHeight_;
	if(ulCol + blockWidth > params_.ncols_)
	{
		blockWidth = params_.ncols_ - ulCol;
	}
	if(ulLin + blockHeight > params_.nlines_)
	{
		blockHeight = params_.nlines_ - ulLin;
	}

	// Reads the buffer of pixels
	CPLErr error = rasterBand->RasterIO(GF_Read, ulCol, ulLin,
										blockWidth , blockHeight, 
										buf, params_.blockWidth_, params_.blockHeight_, gtype, 0, 0);
	if(error == CE_Failure)
		return false;

	return true;
}

bool TeGDALDecoder::putRasterBlock(const TeBlockIndex& /*index*/, void* /*buf*/, long /*bsize*/)
{
	return false;
}

TeGDALDecoderFactory::TeGDALDecoderFactory(const string& name) : TeDecoderFactory(name) 
{
	std::map<std::string, std::string> mapTypes; //<id,extension>
	mapTypes["JPEG2000"] = "JP2";
	mapTypes["GIF"] = "GIF";
	mapTypes["PNG"] = "PNG";
	mapTypes["BMP"] = "BMP";

	// Starts GDAL Decoder support
	TeGDALDecoder::initialize();
	std::vector<TeGDALDriverInfo> gdalDrivers = TeGDALDecoder::getGDALDrivers();
	for(unsigned int i = 0; i < gdalDrivers.size(); i++)
	{
		std::map<std::string, std::string>::iterator it = mapTypes.find(gdalDrivers[i].getId());
		if(it != mapTypes.end())
		{
			TeDecoderFactory::instanceName2Dec()[it->second] = "GDAL";
		}
	}
}

TeGDALDriverInfo::TeGDALDriverInfo() :
_id(""),
_fullName(""),
_extension(""),
_hasCreateSupport(false)
{}

TeGDALDriverInfo::~TeGDALDriverInfo()
{}

const std::string& TeGDALDriverInfo::getId() const
{
	return _id;
}

const std::string& TeGDALDriverInfo::getFullName() const
{
	return _fullName;
}

const std::string& TeGDALDriverInfo::getExtension() const
{
	return _extension;
}

bool TeGDALDriverInfo::hasCreateSupport() const
{
	return _hasCreateSupport;
}

void TeGDALDriverInfo::setId(const std::string& id)
{
	_id = id;
}

void TeGDALDriverInfo::setFullName(const std::string& n)
{
	_fullName = n;
}

void TeGDALDriverInfo::setExtension(const std::string& e)
{
	_extension = TeConvertToLowerCase(e);
}

void TeGDALDriverInfo::setCreateSupport(bool b)
{
	_hasCreateSupport = b;
}

void TeGDALDriverInfo::print() const
{
	std::cout << "- GDAL Driver Id: " << _id << std::endl;
	std::cout << "- Full Name: " << _fullName << std::endl;
	
	std::string s;
	_extension.empty() ? s = "empty" : s = _extension;
	std::cout << "- Associated extension: " << s << std::endl;
	
	_hasCreateSupport ? s = "yes" : s = "no";
	std::cout << "- Create support: " << s << std::endl;

	std::cout << "- File filter: " << getFilter() << "\n" << std::endl;
}

std::string TeGDALDriverInfo::getFilter() const
{
	std::string filter = _fullName + " ";
	filter += "(";
	if(_extension.empty())
		filter += "*.*";
	else
		filter += "*." + _extension;
	filter += ")";

	return filter;
}

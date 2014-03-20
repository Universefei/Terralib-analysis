// GDAL Driver include files
#include "TeGDALDecoder.h"
#include "TeGDALUtils.h"

// TerraLib include files
#include "../../kernel/TeRaster.h"
#include "../../kernel/TeUtils.h"

void Convert2TerraLib(GDALDataset* ds, TeRasterParams& params)
{
	// Number of collumns
	params.ncols_  = ds->GetRasterXSize();
	// Number of lines
	params.nlines_ = ds->GetRasterYSize();
	// Number of bands
	params.nBands(ds->GetRasterCount());

	//Projection
	std::string wkt = ds->GetProjectionRef();
	TeProjection* proj = TeGetTeProjectionFromWKT(wkt);
	params.projection(proj);
	delete proj;

	// For each band, adjusts params
	for(int b = 0; b < params.nBands(); b++)
	{
		GDALRasterBand* band = ds->GetRasterBand(b+1);
		Convert2TerraLib(band, params, b);
	}

	double geoTransformParams[6];
	CPLErr error = ds->GetGeoTransform(geoTransformParams);
	if(error == CE_Failure) // format does not support transformation to projection coordinates
	{
		// Builds the Box
		double llx = 0;
		double lly = 0;
		double urx = (double)params.ncols_;
		double ury = (double)params.nlines_;

		params.boundingBoxLinesColumns(llx, lly, urx, ury, params.nlines_, params.ncols_);
		
		return;
	}

	// Calculates box and pixel resolution
	Convert2TerraLib(geoTransformParams, params);
}

void Convert2TerraLib(GDALRasterBand* rasterBand, TeRasterParams& params, const int& i)
{
	// Gets the dummy value
	int noDataValueIsUsed = 0;
	double dummy = rasterBand->GetNoDataValue(&noDataValueIsUsed);
	if(!noDataValueIsUsed)
		params.useDummy_ = false;
	else
		params.setDummy(dummy, i);

	// Gets the DataType
	GDALDataType type = rasterBand->GetRasterDataType();
	params.setDataType(Convert2TerraLib(type), i);

	// Gets the Photometric Interpretation
	TeRasterParams::TeRasterPhotometricInterpretation ci = Convert2TerraLib(rasterBand->GetColorInterpretation());
	params.setPhotometric(ci, i);
	
	// If pallete, read the associated LUT
	if(ci == TeRasterParams::TePallete)
		ReadLUT(rasterBand, params);

	// Gets min/max values
	int gotMin, gotMax;
	double MinMax[2];
	MinMax[0] = rasterBand->GetMinimum(&gotMin);
    MinMax[1] = rasterBand->GetMaximum(&gotMax);
    if(!(gotMin && gotMax))
		rasterBand->ComputeStatistics(FALSE, &MinMax[0], &MinMax[1], 0, 0, 0, 0);

	params.vmin_[i] = MinMax[0];
	params.vmax_[i] = MinMax[1];
}

TeRasterParams::TeRasterPhotometricInterpretation Convert2TerraLib(GDALColorInterp colorInterpretation)
{
	switch(colorInterpretation)
	{
		case GCI_PaletteIndex: 
			return TeRasterParams::TePallete;
		break;

		case GCI_RedBand:
		case GCI_GreenBand:
		case GCI_BlueBand:
			return TeRasterParams::TeRGB;
		break;

		default:
			return TeRasterParams::TeMultiBand;
	}
}

void ReadLUT(GDALRasterBand* rasterBand, TeRasterParams& params)
{
	GDALColorTable* colorTable = rasterBand->GetColorTable();
	if(colorTable == 0)
		return;

	int ncolors = colorTable->GetColorEntryCount();
	for(int i = 0; i < ncolors; ++i)
	{
		const GDALColorEntry* ce = colorTable->GetColorEntry(i);
		if(ce == 0)
			continue;

		params.lutr_.push_back(ce->c1); 
		params.lutg_.push_back(ce->c2);
		params.lutb_.push_back(ce->c3);
	}
}

void Convert2TerraLib(double* geoTransformParams, TeRasterParams& params)
{
	// Gets the resolution
	params.resx_ = geoTransformParams[1];
	params.resy_ = (-1.0) * geoTransformParams[5];

	// Builds the Box
	double llx = geoTransformParams[0];
	double lly = geoTransformParams[3] + ((double)params.nlines_ * geoTransformParams[5]);
	double urx = geoTransformParams[0] + ((double)params.ncols_  * geoTransformParams[1]);
	double ury = geoTransformParams[3];

	params.boundingBoxLinesColumns(llx, lly, urx, ury, params.nlines_, params.ncols_);
}

void Convert2GDAL(double* geoTransformParams, const TeRasterParams& params)
{
	geoTransformParams[1] = params.resx_;
	geoTransformParams[5] = -1 * params.resy_;

	TeBox b = params.boundingBox();
	geoTransformParams[0] = b.x1_;
	geoTransformParams[3] = b.y2_;

	geoTransformParams[2] = 0.0;
	geoTransformParams[4] = 0.0;
}

TeDataType Convert2TerraLib(GDALDataType type)
{
	// Convert GDAL DataType to TerraLib
	switch(type)
	{
		case GDT_Byte: 
			return TeUNSIGNEDCHAR;
		break;
		
		case GDT_UInt16: 
		case GDT_UInt32:
			return TeUNSIGNEDLONG;
		break;

		case GDT_Int16:
		case GDT_Int32:
			return TeINTEGER;
		break;

		case GDT_Float32: 
			return TeFLOAT;
		break;
		
		case GDT_Float64: 
			return TeDOUBLE;
		break;

		default:
			return TeUNSIGNEDCHAR;
	}
}

GDALDataType Convert2GDAL(TeDataType type)
{
	// Convert TerraLib DataType to GDAL
	switch(type)
	{
		case TeUNSIGNEDCHAR: 
			return GDT_Byte;
		break;
		
		case TeUNSIGNEDLONG: 
			return GDT_UInt32;
		break;

		case TeINTEGER:
			return GDT_Int32;
		break;

		case TeFLOAT: 
			return GDT_Float32;
		break;
		
		case TeDOUBLE: 
			return GDT_Float64;
		break;

		default:
			return GDT_Unknown;
	}
}

GDALDataset* CreateGDALRaster(TeRasterParams& params)
{
	// Gets the appropriate GDAL driver
	std::string path = params.fileName_;
	if(path.empty())
		return 0;
	
	std::string extension = TeGetExtension(path.c_str());
	std::string driverName = TeGDALDecoder::getGDALDriverName(extension);
	if(driverName.empty())
		return 0;

	GDALDriverManager* driverManager = GetGDALDriverManager();
	GDALDriver* driver = driverManager->GetDriverByName(driverName.c_str());

	if(driver == 0)
		return 0;

	// Converts the raster data type
	GDALDataType gDataType = Convert2GDAL(params.dataType_[0]);
	// Creates the raster GDAL
	GDALDataset* ds = driver->Create(path.c_str(), params.ncols_, params.nlines_, params.nBands(), gDataType, 0);

	if(ds == 0)
		return 0;

	// Sets the geometric transformations
	double gt[6];
	Convert2GDAL(gt, params);
	ds->SetGeoTransform(gt);

	// Sets the raster projection
	TeProjection* proj = params.projection();
	if(proj)
	{
		int epsg = proj->epsgCode();
		
		OGRSpatialReference oSRS;
		oSRS.importFromEPSG(epsg);
		
		char* projWKT = 0;
		if(oSRS.exportToWkt(&projWKT) == OGRERR_NONE)
		{
			ds->SetProjection(projWKT);
			OGRFree(projWKT);
		}
	}

	return ds;
}

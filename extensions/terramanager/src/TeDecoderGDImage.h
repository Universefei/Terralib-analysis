/** \file TeDecoderGDImage.h
    \brief This file contais the definitions of a decoder to a GD Image.
	\author Gilberto Ribeiro de Queiroz <gribeiro@dpi.inpe.br>
	\author Ricardo Cartaxo <cartaxo@dpi.inpe.br>
  */

#ifndef __TERRAMANAGER_INTERNAL_DECODERGDIMAGE_H
#define __TERRAMANAGER_INTERNAL_DECODERGDIMAGE_H

// TerraManager include files
#include "TerraManagerConfig.h"

// TerraLib include files
#include <TeDecoder.h>
#include <TeRasterParams.h>

// GD library include files
#include <gd.h>

namespace TeMANAGER
{

	/** \class TeDecoderGDImage
	  * \brief A class to decode GD image data.
	  *
	  *
	  * \sa TeDecoder, TeRasterParams, TeDecoderGDImageFactory
	  *
	  */
	class TERRAMANAGEREXPORT TeDecoderGDImage : public TeDecoder
	{
		public:

			/** \brief Constructor from  raster parameters.
			  */
			TeDecoderGDImage(const TeRasterParams& params)
				: image_(0)
			{
				params_ = params;
			}

			/** \brief Destructor
			  */
			~TeDecoderGDImage()
			{
			}

			/** \brief Sets the value of a specific raster pixel.
				\param col Pixel column identifier.
				\param lin Pixel line identifier.
				\param val Pixel value being inserted.
				\param band Pixel band identifier.
			  */
			bool setElement(int col, int lin, double val, int band = 0)
			{
				int r,g,b;

				int rgb = gdImageGetPixel(image_, col, lin);

				r = gdTrueColorGetRed(rgb);
				g = gdTrueColorGetGreen(rgb);
				b = gdTrueColorGetBlue(rgb);

				if(band == 0)
					r = (int)val;
				else if(band == 1)
					g = (int)val;
				else if(band == 2)
					b = (int)val;
				else
					return false;

				rgb = b | (g << 8) | (r << 16);

				gdImageSetPixel(image_, col, lin, rgb);

				return true;
			}

			/** \brief Gets an specific element (col, lin, band) of a raster data.
		   		\param col Pixel column identifier.
				\param lin Pixel line identifier.
				\param val Pixel value being retrieved.
				\param band Pixel band identifier.
			  */
			bool getElement(int col, int lin, double &val, int band = 0)
			{
				int rgb = gdImageGetPixel(image_, col, lin);

				if(band == 0)
					val = (double)gdTrueColorGetRed(rgb);
				else if(band == 1)
					val = (double)gdTrueColorGetGreen(rgb);
				else if(band == 2)
					val = (double)gdTrueColorGetBlue(rgb);
				else
					return false;

				return true;
			}

			/** \brief Initializes the internal structures of the decoder, from its raster parameters structure.
			  */
			void init()
			{
				params_.status_ = TeRasterParams::TeReadyToWrite;
			}

			/** \brief Sets the GD image used to draw.
				\note This class will NOT free this image, so you must free it by yourself.
			  */
			void setImage(gdImagePtr ptr)
			{
				image_ = ptr;
			}

			/** \brief Clears internal state.
			  */
			bool clear()
			{
				return true;
			}

		private:
			
			gdImagePtr	image_; //!< A pointer to a GD image structure
	};

	/** \class TeDecoderGDImageFactory
	  * \brief Implements a factory to build GD Decoders.
	  *
	  * \sa TeDecoderGDImage, TeDecoderFactory
	  *
	  */
	class TERRAMANAGEREXPORT TeDecoderGDImageFactory : public TeDecoderFactory
	{
		public:

			/** \brief Builds an appropriate decoder from a identifier.
			  */
			TeDecoderGDImageFactory(const string& name) : TeDecoderFactory(name)
			{
			}

			/** \brief Creates an instance of TeDecoderGDImage.
			  */
			TeDecoder* build(const TeRasterParams& arg)
			{
				return new TeDecoderGDImage(arg);
			}
	};

}	// end namespace TeMANAGER

#endif	// __TERRAMANAGER_INTERNAL_DECODERGDIMAGE_H


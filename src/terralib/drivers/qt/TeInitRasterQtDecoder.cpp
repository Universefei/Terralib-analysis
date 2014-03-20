#include "TeDecoderQtImage.h"

#include <map>

void TeInitRasterQtDecoder();

void 
TeInitRasterQtDecoder()
{
	static TeDecoderQtImageFactory theDecoderQtImageFactory("QT");
}
#ifndef __TIFF_H_
#define __TIFF_H_

#include <Windows.h>
#include "tiffio.h"
#include "PWImage.H"
#include <QString>

using namespace std;




class TiffConverter
{

public:
    typedef enum
    {
        C = 0,
        M = 1,
        Y = 2,
        K = 3
    } TIFF_COLOR_CHANNEL;
    typedef TIFF_COLOR_CHANNEL CMYK_COLOR_CHANNEL;

    /*static bool ConvertToTiffChannels(const char*, bool bLvt, bool bTiffs);
	static bool ConvertToTiffChannel(const char*, bool bLvt, bool bTiffs,TIFF_COLOR_CHANNEL channel, string sOutPutFileName = "");
	static bool ConvertCMYKTiffToRGB(const char*, bool bLvt, bool bTiffs,TIFF_COLOR_CHANNEL channel);
	static bool ConvertCMYKTiffToLVTImageRGBC(const char* tiffFile, LVTImageRGBC* lvtImageUC, TIFF_COLOR_CHANNEL channel);


    static bool ConvertCMYKTiffToLVTImageCMYK(const char* tiffFile, PWImageCMYK* lvtImageCMYK, unsigned uPage = 0);    
    static bool ConvertCMYKTiffToLVTImageCMYK(const char* tiffFile, const char* lvtFile);
*/
    static bool ConvertMultipageCMYKTiffToLVTImageCMYK(std::string tiffFile, std::string sOutputFolder, unsigned dpi);

private:
    static void SetTags(TIFF* image,unsigned w, unsigned h);
    static bool CheckTiffCompatibility(TIFF*);
    /*

	static bool SaveTiffChannel(const char*, unsigned char*,unsigned, unsigned, TIFF_COLOR_CHANNEL);
    static bool SaveLVTChannel(const char*, PWImageUC*, TIFF_COLOR_CHANNEL);

	//static bool SaveTiffChannelRGB(const char*, unsigned char*,unsigned, unsigned, TIFF_COLOR_CHANNEL);
    static bool SaveLVTChannel(const char*, PWImageRGBC*, TIFF_COLOR_CHANNEL);*/
};

#endif

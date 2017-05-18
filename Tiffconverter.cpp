#include "TiffConverter.h"

#include "stdio.h"
#include <vector>
#include <iostream>
using  namespace std;

void TiffConverter::SetTags(TIFF* image,unsigned w, unsigned h)
{
	/*
	photometric == 5 (CMYK)
	Compression LZQ == 5
	Orienatation == 1
	Bits per Pixel 8
	Planar == 1
	Samples per pixel == 4
	Strip size
	*/
	TIFFSetField(image, TIFFTAG_IMAGEWIDTH, w);
	TIFFSetField(image, TIFFTAG_IMAGELENGTH, h);
	TIFFSetField(image, TIFFTAG_BITSPERSAMPLE, 8);
	TIFFSetField(image, TIFFTAG_SAMPLESPERPIXEL, 1);
	TIFFSetField(image, TIFFTAG_COMPRESSION, 1);
	TIFFSetField(image, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISWHITE);
	TIFFSetField(image, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(image, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
}

/*
Jos kaytat tata:
Liner Input Additional dependencies: libtiff.lib
Linker Additional library dirs :C:\dev\libtiff\lib
C++ Additional Include directories C:\dev\libtiff\include
Ja juureen libimage.dll libpng13.dll libtiff3.dll glut32.dll jpeg62.dll
Loytyy serverilta Software C++ libraries tjsp / Libtiff
*/

bool TiffConverter::CheckTiffCompatibility(TIFF* tif)
{
    using namespace std;
	bool bOk = true;
    uint16 uPhotometric,uCompres, uOrientation,uBitsPerPixel, uPlanarConfig, uSamples;


	TIFFGetField(tif, TIFFTAG_PHOTOMETRIC , &uPhotometric);
	if (5 != uPhotometric)
		bOk = false;
	TIFFGetField(tif, TIFFTAG_COMPRESSION , &uCompres);
    //if (5 != uCompres) // taa voi olla myos varmana jotain muuta
	//	bOk = false;
	TIFFGetField(tif, TIFFTAG_ORIENTATION , &uOrientation);
	if (1 != uOrientation)
		bOk = false;
	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE , &uBitsPerPixel);
	if (8 != uBitsPerPixel)
		bOk = false;
	TIFFGetField(tif,TIFFTAG_PLANARCONFIG, &uPlanarConfig);
	if (1 != uPlanarConfig)
		bOk = false;
	TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &uSamples);
	if (4 != uSamples)
		bOk = false;



	return bOk;
}


/*
bool TiffConverter::SaveLVTChannel(const char* szFileName, PWImageUC* pLvt,TIFF_COLOR_CHANNEL tfcChannel)
{
	std::string sFile;
	unsigned uDotMark = strlen(szFileName)-1;

	if (0 == strcmp(&szFileName[strlen(szFileName)-strlen(".tiff")],".tiff" ))
		uDotMark = strlen(szFileName)-strlen(".tiff");
	if (0 == strcmp(&szFileName[strlen(szFileName)-strlen(".tif")],".tif" ))
		uDotMark = strlen(szFileName)-strlen(".tif");

	sFile = szFileName;
	sFile = StringBuilder() << sFile.substr(0, uDotMark);

	switch (tfcChannel)
	{
	case C:
		sFile = StringBuilder() << sFile <<  "_C.lvt";
		break;
	case M:
		sFile = StringBuilder() << sFile <<  "_M.lvt";
		break;
	case Y:
		sFile = StringBuilder() << sFile <<  "_Y.lvt";
		break;
	case K:
		sFile = StringBuilder() << sFile <<  "_K.lvt";
		break;

	}

    pLvt->save((char*)sFile.c_str());
	return true;
}

bool TiffConverter::SaveLVTChannel(const char* szFileName, PWImageRGBC* pLvt,TIFF_COLOR_CHANNEL tfcChannel)
{
	std::string sFile;
	unsigned uDotMark = strlen(szFileName)-1;

	if (0 == strcmp(&szFileName[strlen(szFileName)-strlen(".tiff")],".tiff" ))
		uDotMark = strlen(szFileName)-strlen(".tiff");
	if (0 == strcmp(&szFileName[strlen(szFileName)-strlen(".tif")],".tif" ))
		uDotMark = strlen(szFileName)-strlen(".tif");

	sFile = szFileName;
	sFile = StringBuilder() << sFile.substr(0, uDotMark);

	switch (tfcChannel)
	{
	case C:
		sFile = StringBuilder() << sFile <<  "_C.lvt";
		break;
	case M:
		sFile = StringBuilder() << sFile <<  "_M.lvt";
		break;
	case Y:
		sFile = StringBuilder() << sFile <<  "_Y.lvt";
		break;
	case K:
		sFile = StringBuilder() << sFile <<  "_K.lvt";
		break;

	}

    pLvt->save((char*)sFile.c_str());
	return true;
}



bool TiffConverter::SaveTiffChannel(const char* szFileName, unsigned char *pData, unsigned uWidth, unsigned uHeight, TIFF_COLOR_CHANNEL tfcChannel)
{
	TIFF* tifImage;
	std::string sFile;
	unsigned uDotMark = strlen(szFileName)-1;

	if (0 == strcmp(&szFileName[strlen(szFileName)-strlen(".tiff")],".tiff" ))
		uDotMark = strlen(szFileName)-strlen(".tiff");
	if (0 == strcmp(&szFileName[strlen(szFileName)-strlen(".tif")],".tif" ))
		uDotMark = strlen(szFileName)-strlen(".tif");
	
	sFile = szFileName;
	sFile = StringBuilder() << sFile.substr(0, uDotMark);

	switch (tfcChannel)
	{
	case C:
		sFile = StringBuilder() << sFile <<  "_C.tif";
		break;
	case M:
		sFile = StringBuilder() << sFile <<  "_M.tif";
		break;
	case Y:
		sFile = StringBuilder() << sFile <<  "_Y.tif";
		break;
	case K:
		sFile = StringBuilder() << sFile <<  "_K.tif";
		break;

	}

	tifImage = TIFFOpen(sFile.c_str(), "w");
	if (tifImage)
	{
		SetTags(tifImage,uWidth,uHeight);
		unsigned uResult = TIFFWriteRawStrip(tifImage, 0, reinterpret_cast<tdata_t*>(pData), uWidth  * uHeight);
		TIFFClose(tifImage);
		return true;
	}
	else
	{
		return false;
	}
}



*/


bool TiffConverter::ConvertMultipageCMYKTiffToLVTImageCMYK(std::string tiffFile, std::string sOutputFolder, unsigned dpi)
{
    TIFF* tif = TIFFOpen(tiffFile.c_str(), "r");
    if(tif)
    {
        cout << "Opened tiff" << endl;
        if (!CheckTiffCompatibility(tif))
        {
            cout << "Tiff not compatible" << endl;
            TIFFClose(tif);
            return false;
        }


        int dircount = 0;
        do
        {
            dircount++;
        } while (TIFFReadDirectory(tif));

        cout << "Tiff has " << dircount << " directories " << endl;

        for (int i = 0; i < dircount; i++)
        {
            TIFFSetDirectory(tif,i);

            cout << "Opened tiff page " << i << endl;

            std::vector <TIFF_COLOR_CHANNEL> vChannels;
            std::vector <TIFF_COLOR_CHANNEL>::iterator it;
            vChannels.push_back(C);
            vChannels.push_back(M);
            vChannels.push_back(Y);
            vChannels.push_back(K);

            uint32 uWidth, uHeight;
            TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &uWidth);
            TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &uHeight);
            cout << "TIFF Page " << i << " size is " << uWidth << "," << uHeight << endl;

            bool bScanlineTiff = false;
            unsigned uStripSize = TIFFStripSize (tif);
            unsigned uLineLength = uStripSize / sizeof(PWPixelCMYK);
            unsigned uStripMax = TIFFNumberOfStrips (tif);
            if (uStripMax != uHeight && uStripSize != sizeof(PWPixelCMYK) * uWidth)
            {
                bScanlineTiff = true;
            }

            unsigned ubufferSize = uStripMax * uStripSize;



            if (bScanlineTiff)
            {
                PWImageCMYK lvt(PW_CMYK_32BIT,uWidth,uHeight);

                cout << "Scan line tiff" << endl;
                TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &uStripMax);
                cout << "Strip max " << uStripMax <<endl;

                uStripSize = TIFFScanlineSize(tif);
                uLineLength = uStripSize / 4;
                ubufferSize = uStripSize * uStripMax;

                float fCMYK[4];

                vector<unsigned char> pTemp;
                pTemp.resize(uStripSize);

                for (int i = 0; i < uStripMax; i++)
                {
                    for (int uSamples = 0; uSamples < 4; uSamples++)
                    {
                        TIFFReadScanline(tif, pTemp.data(),uStripMax - i -1,uSamples);
                        int d = 0;
                        for (int z = 0; z < uStripSize; z+=4)
                        {
                            unsigned uSlot = 0;

                            for (int b = 0; b < vChannels.size(); b++)
                            {
                                TIFF_COLOR_CHANNEL uOFfset = vChannels[b];
                                //fCMYK[uOFfset] =  (pTemp[z + uOFfset]);
                                fCMYK[uOFfset] =  (pTemp.data()[z + uOFfset]);
                                uSlot++;
                            }

                            PWPixelCMYK cmyk;
                            cmyk.cC = fCMYK[C];
                            cmyk.cK = fCMYK[K];
                            cmyk.cM = fCMYK[M];
                            cmyk.cY = fCMYK[Y];
                            d++;
                            memcpy(
                                    &lvt.getImagePointer()[(i* uLineLength) + (uLineLength - d)],
                                    &cmyk,
                                    sizeof(PWPixelCMYK));
                        }
                    }
                }

                unsigned uDot = tiffFile.find_last_of('.');
                unsigned uSlash = tiffFile.find_last_of('/');
                if (uDot > 0)
                {
                    std::string sSaveName = sOutputFolder;
                    sSaveName.append(tiffFile.substr(uSlash,uDot-uSlash));
                    sSaveName.append("_Sheet_");
                    char szTmp[32];
                    sSaveName.append(itoa(i+1,szTmp,10));
                    sSaveName.append(StringBuilder() << "_" << dpi << "dpi");
                    sSaveName.append(".lvt");
                    cout << "Saving " << sSaveName << endl;
                    lvt.save(sSaveName.c_str());
                }

                cout << "done" << endl;

            } // end scanline
            else
            {
                cout << "Not a scanline TIFF, exit" << endl;
                TIFFClose(tif);
                return false;
            }

        } // Page for

        TIFFClose(tif);
        return true;
    }
    return false;

}

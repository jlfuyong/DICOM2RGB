#pragma once

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef _WINDOWS
typedef unsigned char       BYTE;
typedef unsigned long       DWORD;
typedef unsigned short      WORD;
typedef bool      BOOL;
#endif

enum RT_DICOMRGB_MAP {
	RT_SUCCEED,
	RT_FAILURE,
	RT_INVALID_PARAM
};

typedef enum {
	DCMRGB_PHOTMETR_INTEPR_NONE,
	DCMRGB_PHOTMETR_INTEPR_MONOCHROME1,
	DCMRGB_PHOTMETR_INTEPR_MONOCHROME2,
	DCMRGB_PHOTMETR_INTEPR_PALETTECOLOR,
	DCMRGB_PHOTMETR_INTEPR_RGB,
	DCMRGB_PHOTMETR_INTEPR_HSV,
	DCMRGB_PHOTMETR_INTEPR_ARGB,
	DCMRGB_PHOTMETR_INTEPR_CMYK,
	DCMRGB_PHOTMETR_INTEPR_YBR_FULL,
	DCMRGB_PHOTMETR_INTEPR_YBR_FULL_422,
	DCMRGB_PHOTMETR_INTEPR_YBR_PARTIAL_422

}   DCMRGB_PHOTMETR_INTEPR;

struct DCMRGBImgPixInfo {
	int				nNumberOfFrames;	//0028,0008) Number of Frames
	unsigned short	sSamplesPerPixel;	//Samples per Pixel (0028,0002)
	DCMRGB_PHOTMETR_INTEPR ePhotmetrIntepr;	//Photometric Interpretation (0028,0004)
	unsigned short  sRows;				//Rows (0028,0010)
	unsigned short  sColumns;			//Columns (0028,0011)
	unsigned short  sBitsAllocated;		//Bits Allocated (0028,0100)
	unsigned short  sBitsStored;		//Bits Stored (0028,0101)
	unsigned short  sHighBit;			//High Bit (0028,0102)
	bool			bPixRepr;			//Pixel Representation (0028,0103)
	bool			bPlanConf;			//Planar Configuration (0028,0006)
	int				nVPixAspRat;		//Pixel Aspect Ratio (0028,0034)
	int				nHPixAspRat;		//
	short			sSmallestImgPixValue;//Smallest Image Pixel Value (0028,0106)
	short			sLargestImgPixValue;//Largest Image Pixel Value (0028,0107)
	short			sRedPalColLUTDes[3];//Red Palette Color Lookup Table Descriptor	(0028,1101)
	short			sGreenPalColLUTDes[3];//Green Palette Color Lookup Table Descriptor(0028,1102)
	short			sBluePalColLUTDes[3];//Blue Palette Color Lookup Table	Descriptor(0028,1103)
	WORD *			pRedPalColLUTData;	//Red Palette Color Lookup Table Data (0028,1201)
	WORD *			pGreenPalColLUTData;//Green Palette Color Lookup Table Data	(0028,1202)
	WORD *			pBluePalColLUTData;	//Blue Palette Color Lookup Table Data (0028,1203)
	int				nXferSyn;
};

struct DCMRGBModLUTInfo {
	double dRescIntercept;				//Rescale Intercept (0028,1052)
	double dRescSlope;					//Rescale Slope (0028,1053)
	BOOL bLUT;							//When the transformation is linear, the Modality LUT is described by
										//Rescale Slope (0028,1053) and Rescale Intercept (0028,1052). When the transformation is non-linear,
										//the Modality LUT is described by Modality LUT Sequence (0028,3000).
	BOOL bRescale;						//
	unsigned short sLUTDescriptor[3];	//LUT Descriptor (0028,3002)
	unsigned short * pLUTData;			//LUT Data (0028,3006)
	char szRescOrLUTType[64 + 1];	//Modality LUT Type (0028,3004)
	char szLUTExplanation[64 + 1];	//LUT Explanation (0028,3003)
};

class CDICOMRGBMap
{
public:
	CDICOMRGBMap();
	virtual ~CDICOMRGBMap();

public:
	DCMRGBImgPixInfo* m_pDCMRGBImgPixInfo;
	DCMRGBModLUTInfo* m_pDCMRGBModLUTInfo;
    
    BYTE* m_pRAWDataIn;
    BYTE* m_pRAWDataOut;

private:
	int* m_pDCMIndexValue;
	BYTE* m_pRGBIndexValue;
private:
	int GetDicomValue(BYTE* pRAWData, DWORD dwOffest, int* pDICOMPixValue);
	int GetDicomValue(int nSrcPixel, int* pDICOMPixValue);

	int DCMRGBDicomValueMaxMinGet(BYTE* pRAWData);
	int DCMRGBDCMIndexLookUpCollect(BYTE* pRAWData);
	int DCMRGBRGBIndexLookUpCollect(int nWindowWidth, int nWindowLevel);

	int DCMRGBValueGet(int nX, int nY, int* pValueR, int* pValueG, int* pValueB, BYTE* pRAWData);
    int DCMRGBValueGetRAW(BYTE* pRAWDataIn, BYTE* pRAWDataOut);

    
public:
    int DCMRGBInit(BYTE* pRAWDataIn, int nWindow, int nLevel);
    int DCMRGBRescale(int nWindow, int nLevel);
};

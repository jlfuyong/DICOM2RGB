//#ifdef _WINDOWS
#include "stdafx.h"
//#endif 


#include "DICOMRGBMap.h"
#include "string.h"
#include "cmath"
#include "cstdlib"

CDICOMRGBMap::CDICOMRGBMap()
{
	m_pDCMIndexValue = NULL;
	m_pRGBIndexValue = NULL;
    
    m_pRAWDataIn = NULL;
    m_pRAWDataOut = NULL;

	m_pDCMRGBImgPixInfo = NULL;
	m_pDCMRGBModLUTInfo = NULL;
}

CDICOMRGBMap::~CDICOMRGBMap()
{
	if (m_pDCMIndexValue != NULL) {
		delete []m_pDCMIndexValue;
		m_pDCMIndexValue = NULL;
	}
	if (m_pRGBIndexValue != NULL) {
		delete m_pRGBIndexValue;
		m_pRGBIndexValue = NULL;
	}
    
    if (m_pRAWDataIn != NULL) {
        delete []m_pRAWDataIn;
        m_pRAWDataIn = NULL;
    }
    if (m_pRAWDataOut != NULL) {
        delete m_pRAWDataOut;
        m_pRAWDataOut = NULL;
    }
    
	if (m_pDCMRGBImgPixInfo != NULL) {
		delete m_pDCMRGBImgPixInfo;
		m_pDCMRGBImgPixInfo = NULL;
	}
	if (m_pDCMRGBModLUTInfo != NULL) {
		delete m_pDCMRGBModLUTInfo;
		m_pDCMRGBModLUTInfo = NULL;
	}
}

int CDICOMRGBMap::DCMRGBDCMIndexLookUpCollect(BYTE* pRAWData)
{
	if (m_pDCMRGBImgPixInfo == NULL || m_pDCMRGBModLUTInfo == NULL || pRAWData == NULL) return RT_INVALID_PARAM;

	if (m_pDCMIndexValue != NULL) {
		delete m_pDCMIndexValue;
		m_pDCMIndexValue = NULL;
	}

	DWORD dwMemCount = 0;
	if (m_pDCMRGBImgPixInfo->sBitsAllocated > 16)
		dwMemCount = 0XFFFFF;
	else
		dwMemCount = 0XFFFF;
	m_pDCMIndexValue = new int[dwMemCount + 1];
	for (int n = 0;n <= dwMemCount;n++)
	{
		GetDicomValue(n, &m_pDCMIndexValue[n]);
	}

	return RT_SUCCEED;
}

int CDICOMRGBMap::DCMRGBRGBIndexLookUpCollect(int nWindowWidth, int nWindowLevel)
{
	int nWindow = nWindowWidth;
	int nLevel = nWindowLevel;

	if (m_pDCMRGBModLUTInfo->bRescale && (((int)(m_pDCMRGBModLUTInfo->dRescSlope + 0.5)) != 0))
	{
		nLevel = (int)(nLevel - m_pDCMRGBModLUTInfo->dRescIntercept) / (int)(m_pDCMRGBModLUTInfo->dRescSlope + 0.5);
	}

	GetDicomValue(nLevel, &nLevel);
	double nSlope = m_pDCMRGBModLUTInfo->dRescSlope;
	if (nSlope < 1)
		nSlope = 1;

	nWindow /= nSlope;
	nLevel -= (nWindow >> 1);

	int nMaxValue = m_pDCMRGBImgPixInfo->sLargestImgPixValue;
	int nMinValue = m_pDCMRGBImgPixInfo->sSmallestImgPixValue;
	m_pRGBIndexValue = new BYTE[nMaxValue - nMinValue + 1];
    memset(m_pRGBIndexValue, 0, nMaxValue - nMinValue + 1);

	int nStart = 0;
	int ntemp = 0;
	if (nLevel - nMinValue > 0 && (nLevel - nMinValue < nMaxValue - nMinValue + 1))
	{
		memset(m_pRGBIndexValue + nStart, 0, nLevel - nMinValue);
		nStart = nLevel - nMinValue;
	}

	if (nStart < 0)
		nStart = 0;
	if (nStart > nMaxValue - nMinValue)
		nStart = nMaxValue - nMinValue;

	ntemp = nStart;
	BOOL bDone = FALSE;
	int i = 0;
	for (i = 0;i < 256;i++)
	{
		if (nMinValue < 0)
			nStart = (i*nWindow / 255) + nLevel - nMinValue;
		else
			nStart = (i*nWindow / 255) + nLevel;
		if (nStart < 0)
		{
			nStart = 0;
			continue;
		}
		if (nStart > nMaxValue - nMinValue)
		{
			nStart = nMaxValue - nMinValue;
			memset(m_pRGBIndexValue + ntemp, i, nStart - ntemp + 1);
			bDone = TRUE;
			break;
		}
		if (nStart >= 0)
		{
			memset(m_pRGBIndexValue + ntemp, i, nStart - ntemp + 1);
			ntemp = nStart;
		}
	}
	if (nStart >= 0)
	{
		if (i > 255)
			i = 255;
		if (nStart < abs(nMaxValue) + abs(nMinValue))
			memset(m_pRGBIndexValue + nStart, i, abs(nMaxValue) + abs(nMinValue) + 1 - nStart);
	}
	else
	{
		if (i > 255)
			i = 255;
		memset(m_pRGBIndexValue, i, abs(nMaxValue) + abs(nMinValue) + 1 - nStart);
	}

	return RT_SUCCEED;
}

int CDICOMRGBMap::DCMRGBDicomValueMaxMinGet(BYTE* pRAWData)
{
	if (pRAWData == NULL || m_pDCMRGBImgPixInfo == NULL) return FALSE;
	DWORD dwLength = m_pDCMRGBImgPixInfo->sRows * 
		m_pDCMRGBImgPixInfo->sColumns ;

	int nCTValue = 0;
	GetDicomValue(pRAWData, 0, &nCTValue);
	m_pDCMRGBImgPixInfo->sLargestImgPixValue = m_pDCMRGBImgPixInfo->sSmallestImgPixValue = nCTValue;
	for (DWORD i = 1; i < dwLength; i++)
	{
		GetDicomValue(pRAWData, i, &nCTValue);
		if (nCTValue > m_pDCMRGBImgPixInfo->sLargestImgPixValue)
			m_pDCMRGBImgPixInfo->sLargestImgPixValue = nCTValue;
		if (nCTValue < m_pDCMRGBImgPixInfo->sSmallestImgPixValue)
			m_pDCMRGBImgPixInfo->sSmallestImgPixValue = nCTValue;
	}

	return RT_SUCCEED;
}

int CDICOMRGBMap::DCMRGBValueGetRAW(BYTE* pRAWDataIn, BYTE* pRAWDataOut)
{
	if (pRAWDataOut == NULL || pRAWDataIn == NULL) return RT_FAILURE;

	int nValueA = 0;
	for (int nLoopY = 0; nLoopY < m_pDCMRGBImgPixInfo->sRows; ++nLoopY)
	{
		for (int nLoopX = 0; nLoopX < m_pDCMRGBImgPixInfo->sColumns; ++nLoopX)
		{
			int nValueR, nValueG, nValueB;
			if (DCMRGBValueGet(nLoopX, nLoopY, &nValueR, &nValueG, &nValueB, pRAWDataIn) != RT_SUCCEED) return RT_FAILURE;

			int nPos = nLoopY * m_pDCMRGBImgPixInfo->sColumns * 3 + nLoopX * 3;
			memcpy(pRAWDataOut + nPos, (void*)((BYTE*)(&nValueR)), sizeof(BYTE));
			memcpy(pRAWDataOut + nPos + 1, (void*)((BYTE*)(&nValueG)), sizeof(BYTE));
			memcpy(pRAWDataOut + nPos + 2, (void*)((BYTE*)(&nValueB)), sizeof(BYTE));
		}
	}
	return RT_SUCCEED;
}

int CDICOMRGBMap::DCMRGBValueGet(int nX, int nY, int* pValueR, int* pValueG, int* pValueB, BYTE* pRAWData)
{
	int nDicmpixel = 0, nDicmpixelTemp = 0, nCTValue = 0;
	DWORD dwtemp = 0;
	DWORD dwOffset = 0;

	int nWidth = m_pDCMRGBImgPixInfo->sColumns;
	int nHeight = m_pDCMRGBImgPixInfo->sRows;
	if (m_pDCMRGBImgPixInfo->ePhotmetrIntepr == DCMRGB_PHOTMETR_INTEPR_RGB)
	{
		if (m_pDCMRGBImgPixInfo->bPlanConf == 0) {
			dwOffset = (nY * m_pDCMRGBImgPixInfo->sColumns + nX) * 3;
			nDicmpixel = (*(pRAWData + dwOffset));
			nCTValue = *(m_pDCMIndexValue + nDicmpixel);
			if (m_pDCMRGBImgPixInfo->sSmallestImgPixValue < 0)
				*pValueR = *(m_pRGBIndexValue + nCTValue - m_pDCMRGBImgPixInfo->sSmallestImgPixValue);
			else
				*pValueR = *(m_pRGBIndexValue + nCTValue);

			dwOffset = (nY * m_pDCMRGBImgPixInfo->sColumns + nX) * 3 + 1;
			nDicmpixel = (*(pRAWData + dwOffset));
			nCTValue = *(m_pDCMIndexValue + nDicmpixel);
			if (m_pDCMRGBImgPixInfo->sSmallestImgPixValue < 0)
				*pValueG = *(m_pRGBIndexValue + nCTValue - m_pDCMRGBImgPixInfo->sSmallestImgPixValue);
			else
				*pValueG = *(m_pRGBIndexValue + nCTValue);

			dwOffset = (nY * m_pDCMRGBImgPixInfo->sColumns + nX) * 3 + 2;
			nDicmpixel = (*(pRAWData + dwOffset));
			nCTValue = *(m_pDCMIndexValue + nDicmpixel);
			if (m_pDCMRGBImgPixInfo->sSmallestImgPixValue < 0)
				*pValueB = *(m_pRGBIndexValue + nCTValue - m_pDCMRGBImgPixInfo->sSmallestImgPixValue);
			else
				*pValueB = *(m_pRGBIndexValue + nCTValue);

			//CString szTrace = "";
			//szTrace.Format("dwOffset:%d, R:%d, G:%d, B:%d\n",dwOffset, *pValueR, *pValueG, *pValueB);
			//TRACE(szTrace);
		}else if (m_pDCMRGBImgPixInfo->bPlanConf == 1) {//RRR....GGG....BBB....
			dwOffset = nY * m_pDCMRGBImgPixInfo->sColumns + nX;
			nDicmpixel = (*(pRAWData + dwOffset));
			nCTValue = *(m_pDCMIndexValue + nDicmpixel);
			if (m_pDCMRGBImgPixInfo->sSmallestImgPixValue < 0)
				*pValueR = *(m_pRGBIndexValue + nCTValue - m_pDCMRGBImgPixInfo->sSmallestImgPixValue);
			else
				*pValueR = *(m_pRGBIndexValue + nCTValue);

			dwOffset = nY * m_pDCMRGBImgPixInfo->sColumns + nX + m_pDCMRGBImgPixInfo->sColumns * m_pDCMRGBImgPixInfo->sRows;
			nDicmpixel = (*(pRAWData + dwOffset));
			nCTValue = *(m_pDCMIndexValue + nDicmpixel);
			if (m_pDCMRGBImgPixInfo->sSmallestImgPixValue < 0)
				*pValueG = *(m_pRGBIndexValue + nCTValue - m_pDCMRGBImgPixInfo->sSmallestImgPixValue);
			else
				*pValueG = *(m_pRGBIndexValue + nCTValue);

			dwOffset = nY * m_pDCMRGBImgPixInfo->sColumns * 3 + nX + +m_pDCMRGBImgPixInfo->sColumns * m_pDCMRGBImgPixInfo->sRows * 2;
			nDicmpixel = (*(pRAWData + dwOffset));
			nCTValue = *(m_pDCMIndexValue + nDicmpixel);
			if (m_pDCMRGBImgPixInfo->sSmallestImgPixValue < 0)
				*pValueB = *(m_pRGBIndexValue + nCTValue - m_pDCMRGBImgPixInfo->sSmallestImgPixValue);
			else
				*pValueB = *(m_pRGBIndexValue + nCTValue);
		}
	}
	else
	{
		dwOffset = nY * m_pDCMRGBImgPixInfo->sColumns + nX;
		if (m_pDCMRGBImgPixInfo->sBitsAllocated == 32){
			if (m_pDCMRGBImgPixInfo->nXferSyn == 1){//DCM_TS_EXPLICIT_VR_BE
				nDicmpixel = *(pRAWData + (dwOffset << 1) + 1) + ((*(pRAWData + (dwOffset << 1))) << 8); 
			}else{
				dwtemp = (*(pRAWData + (dwOffset << 2))) + ((*(pRAWData + (dwOffset << 2) + 1) << 8)); 
				nDicmpixel = (*(pRAWData + (dwOffset << 2) + 2)) + ((*(pRAWData + (dwOffset << 2) + 3) << 8)); 
				nDicmpixel = (nDicmpixel << 16) + dwtemp;
			}
		}

		if (m_pDCMRGBImgPixInfo->sBitsAllocated == 16){
			if (m_pDCMRGBImgPixInfo->nXferSyn == 1){//DCM_TS_EXPLICIT_VR_BE
				nDicmpixel = *(pRAWData + (dwOffset << 1) + 1) + ((*(pRAWData + (dwOffset << 1))) << 8); 
			}else{
				nDicmpixel = (*(pRAWData + (dwOffset << 1))) + ((*(pRAWData + (dwOffset << 1) + 1) << 8)); 
			}
		}

		if (m_pDCMRGBImgPixInfo->sBitsAllocated == 8){
			nDicmpixel = (*(pRAWData + dwOffset)); 
		}

		if (m_pDCMRGBImgPixInfo->sBitsAllocated > 8 && m_pDCMRGBImgPixInfo->sBitsAllocated < 16){
			if (m_pDCMRGBImgPixInfo->nXferSyn != 1){//DCM_TS_EXPLICIT_VR_BE
				DWORD nEndOff = 0;
				DWORD nByteOff = 0;
				DWORD nDicmTemp = 0;

				nByteOff = dwOffset * 10 >> 3;
				nEndOff = dwOffset * 10 % 8;
				nDicmpixel = (*(pRAWData + nByteOff)) + (*(pRAWData + nByteOff + 1) << 8); 
				nDicmTemp = nDicmpixel >> nEndOff; 
				nDicmTemp = nDicmTemp << (32 - m_pDCMRGBImgPixInfo->sBitsStored);
				nDicmpixel = nDicmTemp >> (32 - m_pDCMRGBImgPixInfo->sBitsStored);
			}
		}
		nCTValue = *(m_pDCMIndexValue + nDicmpixel);

		int nPixelValue = 0;
		if (m_pDCMRGBImgPixInfo->sSmallestImgPixValue < 0) {
			nPixelValue = *(m_pRGBIndexValue + nCTValue - m_pDCMRGBImgPixInfo->sSmallestImgPixValue);
		}else {
			nPixelValue = *(m_pRGBIndexValue + nCTValue);
		}

		*pValueR = *pValueG = *pValueB = nPixelValue;
	}

	return RT_SUCCEED;
}

int CDICOMRGBMap::GetDicomValue(BYTE* pRAWData, DWORD dwOffest, int* pDICOMPixValue)
{
	int nDicmpixelTemp = 0;
	DWORD nEndOff = 0, nByteOff = 0, nDicmTemp = 0;
	if (m_pDCMRGBImgPixInfo->sBitsAllocated == 16)
	{
		if (m_pDCMRGBImgPixInfo->nXferSyn == 1)//DCM_TS_EXPLICIT_VR_BE
			*pDICOMPixValue = *(pRAWData + (dwOffest << 1) + 1) + ((*(pRAWData + (dwOffest << 1))) << 8);
		else
			*pDICOMPixValue = (*(pRAWData + (dwOffest << 1))) + (*(pRAWData + (dwOffest << 1) + 1) << 8);
	}
	if (m_pDCMRGBImgPixInfo->sBitsAllocated == 8)
		*pDICOMPixValue = *(pRAWData + dwOffest);
	if (m_pDCMRGBImgPixInfo->sBitsAllocated > 8 && m_pDCMRGBImgPixInfo->sBitsAllocated < 16)
	{
		if (m_pDCMRGBImgPixInfo->nXferSyn != 1)//DCM_TS_EXPLICIT_VR_BE
		{
			nByteOff = dwOffest * 10 >> 3;
			nEndOff = dwOffest * 10 % 8;
			*pDICOMPixValue = (*(pRAWData + nByteOff)) + (*(pRAWData + nByteOff + 1) << 8);
			nDicmTemp = (*pDICOMPixValue) >> nEndOff;
			nDicmTemp = nDicmTemp << (32 - m_pDCMRGBImgPixInfo->sBitsStored);
			*pDICOMPixValue = nDicmTemp >> (32 - m_pDCMRGBImgPixInfo->sBitsStored);
		}
	}
	return GetDicomValue(*pDICOMPixValue, pDICOMPixValue);
}

int CDICOMRGBMap::GetDicomValue(int nSrcPixel, int* pDICOMPixValue)
{
	DWORD nDicmpixelTemp = 32 - m_pDCMRGBImgPixInfo->sBitsStored;
	*pDICOMPixValue = (DWORD)nSrcPixel << nDicmpixelTemp >> nDicmpixelTemp;

	if (m_pDCMRGBImgPixInfo->ePhotmetrIntepr == DCMRGB_PHOTMETR_INTEPR_MONOCHROME1) {
		nDicmpixelTemp = *pDICOMPixValue << (32 - m_pDCMRGBImgPixInfo->sBitsStored);
		*pDICOMPixValue = (~nDicmpixelTemp) >> (32 - m_pDCMRGBImgPixInfo->sBitsStored);
	}
	else if (m_pDCMRGBImgPixInfo->ePhotmetrIntepr == DCMRGB_PHOTMETR_INTEPR_RGB)
		return RT_SUCCEED;

	short sBitsStore = 1 << (m_pDCMRGBImgPixInfo->sBitsStored - 1);
	if (m_pDCMRGBImgPixInfo->bPixRepr)
	{
		if (*pDICOMPixValue&sBitsStore)
		{
			nDicmpixelTemp = 0xffffffff << m_pDCMRGBImgPixInfo->sBitsStored;
			*pDICOMPixValue = (*pDICOMPixValue) | nDicmpixelTemp;
		}
	}
	return RT_SUCCEED;
}

int CDICOMRGBMap::DCMRGBInit(BYTE* pRAWDataIn, int nWindow, int nLevel)
{
    if(m_pRAWDataIn != NULL){
        delete[] m_pRAWDataIn;
        m_pRAWDataIn = NULL;
    }
    if(m_pRAWDataOut != NULL){
        delete[] m_pRAWDataOut;
        m_pRAWDataOut = NULL;
    }
    int nRet = RT_FAILURE;
	int nImageSize = (m_pDCMRGBImgPixInfo->sRows *
		m_pDCMRGBImgPixInfo->sColumns *
		m_pDCMRGBImgPixInfo->sSamplesPerPixel *
		m_pDCMRGBImgPixInfo->sBitsAllocated) >> 3;
	
	m_pRAWDataIn = new BYTE[nImageSize];
    memset(m_pRAWDataIn, 0, nImageSize);
    memcpy(m_pRAWDataIn, pRAWDataIn, nImageSize);
    
    nRet = DCMRGBDicomValueMaxMinGet(m_pRAWDataIn);
    if(nRet != RT_SUCCEED) return nRet;
    
    nRet = DCMRGBDCMIndexLookUpCollect(m_pRAWDataIn);
    if(nRet != RT_SUCCEED) return nRet;
    
    if ((nRet = DCMRGBRGBIndexLookUpCollect(nWindow, nLevel)) == RT_SUCCEED) {
        m_pRAWDataOut = new BYTE[m_pDCMRGBImgPixInfo->sColumns * m_pDCMRGBImgPixInfo->sRows * 3];
        memset(m_pRAWDataOut, 0, m_pDCMRGBImgPixInfo->sColumns * m_pDCMRGBImgPixInfo->sRows * 3);
        return DCMRGBValueGetRAW(m_pRAWDataIn, m_pRAWDataOut);
    }
    
    return RT_SUCCEED;
}

int CDICOMRGBMap::DCMRGBRescale(int nWindow, int nLevel)
{
    int nRet = RT_FAILURE;
    if ((nRet = DCMRGBRGBIndexLookUpCollect(nWindow, nLevel)) == RT_SUCCEED) {
        int nImageOutSize = m_pDCMRGBImgPixInfo->sRows * m_pDCMRGBImgPixInfo->sColumns * 3;
        memset(m_pRAWDataOut, 0, nImageOutSize);
        return DCMRGBValueGetRAW(m_pRAWDataIn, m_pRAWDataOut);
    }
    
    return RT_SUCCEED;
}

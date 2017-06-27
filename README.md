# DICOM2RGB
A c++ class using for convert DICOM PixelData to RGB Data

1. Get dicom Pixel data(one frame)
2. Build DCMRGBImgPixInfo
3. Build DCMRGBModLUTInfo
4. Get default Window width and window center
5. DCMRGBInit(pRAWData, m_nWindow, m_nLevel);
6. The rgb output to CDICOMRGBMap->m_pRAWDataOut
7. Draw image

Demo VS2015 like :

void test()
{
	CString szRAWFileName = "";
	szRAWFileName.Format("%s.raw", szFileName);
	DCMObj elemList;
	char szErrorMsg[1024] = { 0 };
	DCMReadFile((char*)(LPCTSTR)szFileName, &elemList, szErrorMsg);
	DCMImgPixInfo dcmImgPixInfo;
	DCMElemListGetPixelInfo(&elemList, &dcmImgPixInfo, szErrorMsg);
	DCMModLUTInfo dcmModLUTInfo;
	DCMElemListGetModLUTInfo(&elemList, &dcmModLUTInfo, szErrorMsg);

	if (DCMElemListValueGetInt(&elemList, 0x00281050, &m_nLevel, 0, szErrorMsg)) {

	}
	else m_nLevel = 0;
	if (DCMElemListValueGetInt(&elemList, 0x00281051, &m_nWindow, 0, szErrorMsg)) {

	}
	else m_nWindow = 0;

	if (m_nLevel == 0 && m_nWindow == 0) {
		m_nWindow = 255;
		m_nLevel = 127;
	}
	DCMObj elemPixel;
	if (!DCMElemListElemGet(&elemList, 0x7fe00010, &elemPixel, szErrorMsg)) {
		DCMElemListDestroy(&elemList, szErrorMsg);
		return ;
	}
	ULONG ulBufLen = dcmImgPixInfo.sColumns*dcmImgPixInfo.sRows*dcmImgPixInfo.sBitsAllocated / 8;
	if (!DCMElemLenGet(&elemPixel, &ulBufLen, dcmImgPixInfo.nXferSyn, szErrorMsg))
	{
		DCMElemListDestroy(&elemList, szErrorMsg);
		return ;
	}
	ULONG ulFileBufLen = ulBufLen / dcmImgPixInfo.nNumberOfFrames;


	//char * szDestFilenameTemp = "";
	for (int nLoop = 0;nLoop < dcmImgPixInfo.nNumberOfFrames;++nLoop)
	{
		//char * szDestFilenameTemp = szDestFilename;
		BYTE* lpDicmPixel;
		lpDicmPixel = new BYTE[ulFileBufLen];
		if (!DCMElemListValueGetPixel(&elemList, lpDicmPixel, ulFileBufLen, nLoop, szErrorMsg))
		{
			DCMElemListDestroy(&elemList, szErrorMsg);
			delete[] lpDicmPixel;
			return ;
		}



		FILE *file = fopen(szRAWFileName, "wb");
		if (file != NULL)
		{
			fwrite(lpDicmPixel, sizeof(BYTE), ulFileBufLen, file);
		}


		fclose(file);
		delete[] lpDicmPixel;

		break;
	}
	DCMElemListDestroy(&elemList);

	int nPixelLength = (dcmImgPixInfo.sRows *
		dcmImgPixInfo.sColumns *
		dcmImgPixInfo.sSamplesPerPixel *
		dcmImgPixInfo.sBitsAllocated) >> 3;

	BYTE* pRAWData = new BYTE[nPixelLength];
	FILE *fp = fopen((char*)(LPCTSTR)szRAWFileName, "r");
	if (fp == NULL) {
		AfxMessageBox("fopen failure.");
	}
	fread(pRAWData, sizeof(BYTE), nPixelLength, fp);
	fclose(fp);

	if (m_pDICOMRGBMap != NULL) {
		delete m_pDICOMRGBMap;
		m_pDICOMRGBMap = NULL;
	}
	m_pDICOMRGBMap = new CDICOMRGBMap;
	m_pDICOMRGBMap->m_pDCMRGBImgPixInfo = new DCMRGBImgPixInfo;
	m_pDICOMRGBMap->m_pDCMRGBImgPixInfo->bPixRepr = dcmImgPixInfo.bPixRepr;
	m_pDICOMRGBMap->m_pDCMRGBImgPixInfo->bPlanConf = dcmImgPixInfo.bPlanConf;
	m_pDICOMRGBMap->m_pDCMRGBImgPixInfo->ePhotmetrIntepr = (DCMRGB_PHOTMETR_INTEPR)dcmImgPixInfo.ePhotmetrIntepr;
	m_pDICOMRGBMap->m_pDCMRGBImgPixInfo->sRows = dcmImgPixInfo.sRows;
	m_pDICOMRGBMap->m_pDCMRGBImgPixInfo->sColumns = dcmImgPixInfo.sColumns;
	m_pDICOMRGBMap->m_pDCMRGBImgPixInfo->sBitsAllocated = dcmImgPixInfo.sBitsAllocated;
	m_pDICOMRGBMap->m_pDCMRGBImgPixInfo->sBitsStored = dcmImgPixInfo.sBitsStored;
	m_pDICOMRGBMap->m_pDCMRGBImgPixInfo->sHighBit = dcmImgPixInfo.sHighBit;
	m_pDICOMRGBMap->m_pDCMRGBImgPixInfo->nNumberOfFrames = dcmImgPixInfo.nNumberOfFrames;
	m_pDICOMRGBMap->m_pDCMRGBImgPixInfo->sSamplesPerPixel = dcmImgPixInfo.sSamplesPerPixel;
	m_pDICOMRGBMap->m_pDCMRGBImgPixInfo->sSmallestImgPixValue = dcmImgPixInfo.sSmallestImgPixValue;
	m_pDICOMRGBMap->m_pDCMRGBImgPixInfo->sLargestImgPixValue = dcmImgPixInfo.sLargestImgPixValue;

	m_pDICOMRGBMap->m_pDCMRGBModLUTInfo = new DCMRGBModLUTInfo;
	m_pDICOMRGBMap->m_pDCMRGBModLUTInfo->bLUT = dcmModLUTInfo.bLUT;
	m_pDICOMRGBMap->m_pDCMRGBModLUTInfo->dRescIntercept = dcmModLUTInfo.dRescIntercept;
	m_pDICOMRGBMap->m_pDCMRGBModLUTInfo->dRescSlope = dcmModLUTInfo.dRescSlope;
	m_pDICOMRGBMap->m_pDCMRGBModLUTInfo->bRescale = dcmModLUTInfo.bRescale;

	m_pDICOMRGBMap->DCMRGBInit(pRAWData, m_nWindow, m_nLevel);
	delete[]pRAWData;

}



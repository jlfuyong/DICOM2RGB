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



BOOL CFTTestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();


	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}


	SetIcon(m_hIcon, TRUE);			
	SetIcon(m_hIcon, FALSE);		


	m_pDICOMRGBMap = NULL;

	return TRUE;
}

void CFTTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

void CFTTestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);


		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;


		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}


HCURSOR CFTTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void flipIt(void* buffer, long nSize)
{
	void* b = buffer;
	__asm
	{
		mov ecx, nSize
		mov ebx, b
		label :
		mov al, [ebx + 0]
			mov ah, [ebx + 2]
			mov[ebx + 2], al
			mov[ebx + 0], ah

			add ebx, 3
			dec ecx
			jnz label
	}
}


void CFTTestDlg::SaveToBmpAndBlend(BYTE *buffer)
{
	HDC hdc;
      
	CDC* pDC  = this->GetDC();
	hdc = pDC->m_hDC;

	HDC hdcMem = CreateCompatibleDC(hdc);

	HBITMAP hBmp;

	int nImgWidth = m_pDICOMRGBMap->m_pDCMRGBImgPixInfo->sColumns;
	int nImgHeight = m_pDICOMRGBMap->m_pDCMRGBImgPixInfo->sRows;

	BITMAPINFO bmpinfo;
	bmpinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpinfo.bmiHeader.biWidth = nImgWidth;
	bmpinfo.bmiHeader.biHeight = nImgHeight;
	bmpinfo.bmiHeader.biPlanes = 1;
	bmpinfo.bmiHeader.biBitCount = 24;
	bmpinfo.bmiHeader.biCompression = BI_RGB;
	bmpinfo.bmiHeader.biSizeImage = nImgWidth * nImgHeight * 3;
	bmpinfo.bmiHeader.biXPelsPerMeter = 0;
	bmpinfo.bmiHeader.biClrImportant = 0;
	bmpinfo.bmiHeader.biClrUsed = 0;
	BYTE* pDibs = NULL;
 
	hBmp = CreateDIBSection(hdcMem, &bmpinfo, DIB_RGB_COLORS, (void**)&pDibs, NULL, 0);
	//for (int nLoop = 0; nLoop < nImgWidth * nImgHeight * 3; nLoop++)
	//{
	//	pDibs[nLoop] = buffer[nLoop];
	//}
	for (int nLoopX = 0; nLoopX < nImgWidth; ++nLoopX)
	{
		for (int nLoopY = 0; nLoopY < nImgHeight; ++nLoopY)
		{
			int nPosDib = nLoopY * nImgWidth * 3 + nLoopX * 3;
			int nPosImg = (nImgHeight - nLoopY - 1) * nImgWidth * 3 + nLoopX * 3;
			pDibs[nPosDib] = buffer[nPosImg];
			pDibs[nPosDib + 1] = buffer[nPosImg + 1];
			pDibs[nPosDib + 2] = buffer[nPosImg + 2];
		}
	}

	if(m_pDICOMRGBMap->m_pDCMRGBImgPixInfo->ePhotmetrIntepr == DCMRGB_PHOTMETR_INTEPR_RGB) 
	flipIt(pDibs, nImgWidth * nImgHeight);

	HGDIOBJ hOldSel = SelectObject(hdcMem, hBmp);

	CRect rcImage;
	m_stcImage.GetWindowRect(&rcImage);
	ScreenToClient(&rcImage);
	//BitBlt(hdc, rcImage.left, rcImage.top, rcImage.Width(), rcImage.Height(), hdcMem, 0, 0, SRCCOPY);//
	SetStretchBltMode(hdc, STRETCH_HALFTONE);
	StretchBlt(hdc, rcImage.left, rcImage.top, rcImage.Width(), rcImage.Height(), hdcMem, 0, 0, nImgWidth, nImgHeight, SRCCOPY);
	//for (int nLoopX = 0; nLoopX < nImgWidth; ++nLoopX)
	//{
	//	for (int nLoopY = 0; nLoopY < nImgHeight; ++nLoopY)
	//	{
	//		int nPosDib = nLoopY * nImgWidth * 3 + nLoopX * 3;
	//		int nPosImg = (nImgHeight - nLoopY - 1) * nImgWidth * 3 + nLoopX * 3;
	//		pDibs[nPosDib] = buffer[nPosImg];
	//		pDibs[nPosDib + 1] = buffer[nPosImg + 1];
	//		pDibs[nPosDib + 2] = buffer[nPosImg + 2];
	//		pDC->SetPixel(nLoopX, nLoopY, RGB(buffer[nPosImg], buffer[nPosImg + 1], buffer[nPosImg + 2]));
	//	}
	//}
	SelectObject(hdcMem, hOldSel);
	DeleteDC(hdcMem);
	::ReleaseDC(m_hWnd, hdc);
}

void CFTTestDlg::OnBnClickedButton2()
{
	if (m_pDICOMRGBMap == NULL) return;
	DWORD dTickBegin = GetTickCount();

	m_pDICOMRGBMap->DCMRGBRescale(m_nWindow, m_nLevel);

	SaveToBmpAndBlend(m_pDICOMRGBMap->m_pRAWDataOut);

	DWORD dTickEnd = GetTickCount();
	CString szTick = "";
	szTick.Format("Total time: %d", dTickEnd - dTickBegin);
	m_edtTimer.SetWindowText(szTick);
}

void CFTTestDlg::OnBnClickedButton1()
{
	CString filter;
	filter = "DICOM(*.dcm)|*.dcm|文档(*.*)|*.*||";
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, filter);
	CString szFileName = "";
	if (dlg.DoModal() == IDOK)
	{
		szFileName = dlg.GetPathName();
	}
	else return;

	BeginWaitCursor();
	DWORD dTickBegin = GetTickCount();
	
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

	CRect rcImage;
	m_stcImage.GetClientRect(&rcImage);
	m_nWindowStep = (m_pDICOMRGBMap->m_pDCMRGBImgPixInfo->sLargestImgPixValue - m_pDICOMRGBMap->m_pDCMRGBImgPixInfo->sSmallestImgPixValue) / rcImage.Height();
	if (m_nWindowStep <= 0) m_nWindowStep = 1;

	SaveToBmpAndBlend(m_pDICOMRGBMap->m_pRAWDataOut);

	EndWaitCursor();
	DWORD dTickEnd = GetTickCount();
	CString szTick = "";
	szTick.Format("Total time: %d", dTickEnd - dTickBegin);
	m_edtTimer.SetWindowText(szTick);
}

void CFTTestDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	if (m_pDICOMRGBMap != NULL) {
		delete m_pDICOMRGBMap;
		m_pDICOMRGBMap = NULL;
	}
}


void CFTTestDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if (nFlags == MK_LBUTTON) {
		CRect rcImage;
		m_stcImage.GetWindowRect(&rcImage);
		ScreenToClient(&rcImage);

		if (!rcImage.PtInRect(point)) {
			CDialogEx::OnMouseMove(nFlags, point);

			return;
		}
		m_nLevel += point.x - ptPrev.x;
		m_nWindow += point.y - ptPrev.y;
		OnBnClickedButton2();
		
		ptPrev = point;
	}

	CDialogEx::OnMouseMove(nFlags, point);
}


void CFTTestDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	ptPrev = point;

	CDialogEx::OnLButtonDown(nFlags, point);
}


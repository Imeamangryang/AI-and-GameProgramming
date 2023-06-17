//
//	Dept. Software Convergence, Kyung Hee University
//	Prof. Daeho Lee, nize@khu.ac.kr
//
#include "KhuGleWin.h"
#include "KhuGleSignal.h"
#include <iostream>
#include <string>
#include <iomanip>

#pragma warning(disable:4996)

#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>

#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif
#endif  // _DEBUG

static void SetInt(int iInput, char* pOutput)
{
	pOutput[0] = (unsigned char)(iInput);
	pOutput[1] = (unsigned char)(iInput >> 8);
	pOutput[2] = (unsigned char)(iInput >> 16);
	pOutput[3] = (unsigned char)(iInput >> 24);
}

class CBmpFileHeader {
public:
	CBmpFileHeader()
	{
		memset(this, 0, sizeof(CBmpFileHeader));

		m_arrType[0] = 'B';
		m_arrType[1] = 'M';
		m_arrOffBit[0] = 54;
	}

	void SetSize(int iSize)
	{
		SetInt(iSize, m_arrSize);
	}

private:
	char m_arrType[2];
	char m_arrSize[4];
	char m_arrReserved[4];
	char m_arrOffBit[4];
};

class CBmpInfoHeader
{
public:
	CBmpInfoHeader()
	{
		memset(this, 0, sizeof(CBmpInfoHeader));
		m_arrSize[0] = 40;
		m_arrPlanes[0] = 1;
		m_arrBitCount[0] = 24;
	}

	void SetWidth(int iWidth)
	{
		SetInt(iWidth, m_arrWidth);
	}

	void SetHeight(int iHeight)
	{
		SetInt(iHeight, m_arrHeight);
	}

	void SetImageSize(int iImageSize)
	{
		SetInt(iImageSize, m_arrImageSize);
	}

private:
	char  m_arrSize[4];
	char  m_arrWidth[4];
	char  m_arrHeight[4];
	char  m_arrPlanes[2];
	char  m_arrBitCount[2];
	char  m_arrCompression[4];
	char  m_arrImageSize[4];
	char  m_arrXpxPerMeter[4];
	char  m_arrYpxPerMeter[4];
	char  m_arrColorUsed[4];
	char  m_arrColorImportant[4];
};

bool SaveBmp(const char* pszFileName, unsigned char* pRGB, int iWidth, int iHeight)
{
	FILE* fd = fopen(pszFileName, "wb");
	if (fd == NULL) return false;

	CBmpFileHeader clsFileHeader;
	CBmpInfoHeader clsInfoHeader;

	int iPadSize = (4 - iWidth % 4) % 4;
	int iImageSize = iWidth * iHeight * 3 + iHeight * iPadSize;
	int iFileSize = iImageSize + 52;

	clsFileHeader.SetSize(iFileSize);
	clsInfoHeader.SetWidth(iWidth);
	clsInfoHeader.SetHeight(iHeight);
	clsInfoHeader.SetImageSize(iImageSize);

	fwrite(&clsFileHeader, 14, 1, fd);
	fwrite(&clsInfoHeader, sizeof(clsInfoHeader), 1, fd);

	unsigned char arrPad[3] = { 0,0,0 };

	for (int y = 0; y < iHeight; ++y)
	{
		for (int x = 0; x < iWidth; ++x)
		{
			// 위 아래를 바꾸어야 한다.
			int iRow = (iHeight - y - 1) * iWidth * 3;
			// 좌 우를 바꾸어야 한다.
			int iCol = (iWidth - x - 1) * 3;

			// QQQ: 이미지의 색상이 이상하면 RGB -> BGR​ 로 변환하여서 저장해야 한다.
			fwrite(pRGB + iRow + iCol, 3, 1, fd);
		}

		if (iPadSize > 0)
		{
			fwrite(arrPad, iPadSize, 1, fd);
		}
	}
	fclose(fd);
	return true;
}

class CKhuGleImageLayer : public CKhuGleLayer {
public:
	CKhuGleSignal m_Image, m_ImageOut;

	CKhuGleImageLayer(int nW, int nH, KgColor24 bgColor, CKgPoint ptPos = CKgPoint(0, 0))
		: CKhuGleLayer(nW, nH, bgColor, ptPos) {}
	void DrawBackgroundImage(int OffsetX, int OffsetY);
	static void DrawLine(unsigned char** R, unsigned char** G, unsigned char** B, int nW, int nH, int x0, int y0, int x1, int y1, KgColor24 Color24);
};

void CKhuGleImageLayer::DrawBackgroundImage(int OffsetX, int OffsetY)
{
	if (m_Image.m_Red && m_Image.m_Green && m_Image.m_Blue)
	{
		for (int y = 0; y < m_Image.m_nH && y + OffsetY < m_nH; ++y)
			for (int x = 0; x < m_Image.m_nW && x + OffsetX < m_nW; ++x)
			{
				m_ImageBgR[y + OffsetY][x + OffsetX] = m_Image.m_Red[y][x];
				m_ImageBgG[y + OffsetY][x + OffsetX] = m_Image.m_Green[y][x];
				m_ImageBgB[y + OffsetY][x + OffsetX] = m_Image.m_Blue[y][x];
			}
	}
}

void CKhuGleImageLayer::DrawLine(unsigned char** R, unsigned char** G, unsigned char** B, int nW, int nH, int x0, int y0, int x1, int y1, KgColor24 Color24)
{
	::DrawLine(R, nW, nH, x0, y0, x1, y1, KgGetRed(Color24));
	::DrawLine(G, nW, nH, x0, y0, x1, y1, KgGetGreen(Color24));
	::DrawLine(B, nW, nH, x0, y0, x1, y1, KgGetBlue(Color24));
}

class KhuPaintBrush : public CKhuGleWin
{
public:
	// Constructor
	KhuPaintBrush(int nW, int nH);
	~KhuPaintBrush();

	// Update Function
	void Update();
	void ButtonInit();
	void GetMousePointXY();
	void DeleteSprite(CKhuGleImageLayer* targetlayer, CKhuGleSprite* target);
	void RecursionFill1(int x, int y, KgColor24 color);
	void RecursionFill2(int x, int y, KgColor24 color);
	void RecursionFill3(int x, int y, KgColor24 color);
	void RecursionFill4(int x, int y, KgColor24 color);
	void ResetFillCheck();

	// Member Variables
	CKhuGleImageLayer* ImageLayer;
	CKhuGleImageLayer* ToolbarLayer;
	CKgPoint LButtonPoint;
	CKgPoint LButtonTemp;

	KgColor24 BrushColor;

	Brush BrushStatus; // Brush 종류가 무엇인지 나타내는 변수
	Toolbar ToolbarStatus; // Toolbar에서 무슨 작업을 수행하는지 나타내는 변수

	CKhuGleSprite* Palette[30]; // Palette Sprite
	CKhuGleSprite* CurrentColor; // 현재 색상이 무엇인지 보여줄 Sprite
	KgColor24 PaletteColor[30]; // Color값 저장

	// Toolbar Button
	CKhuGleSprite* Pen;
	CKhuGleSprite* Erase;
	CKhuGleSprite* Spuit;
	CKhuGleSprite* Text;
	CKhuGleSprite* Shape_Rect;
	CKhuGleSprite* Shape_Ellipse;
	CKhuGleSprite* Fullfill;

	CKgPoint Shape_Point1;
	CKgPoint Shape_Point2;
	bool ShapeSet;

	CKgPoint Text_Point;
	std::vector<std::pair<std::string, CKgPoint>> contents;
	bool TextSet;

	bool CreateErase;
	CKhuGleSprite* EraseBall;
	KgColor24 EraseColor;

	CKhuGleSprite* ImageRead;
	CKhuGleSprite* ImageWrite;

	CKgPoint Image_Point;
	bool ImageSet;

	CKhuGleSprite* PaletteButton;
	CKhuGleSprite* PaletteExitButton;
	CKhuGleSprite* Palette_Extention[729];
	KgColor24 PaletteColor_Extention[729];

	CKhuGleSprite* CurrentColor2;
	CKhuGleSprite* CompareColor;
	int PaletteNum;

	CKhuGleImageLayer* PaletteLayer;

	bool ImageSave;

	CKgPoint FillPoint;
	KgColor24 FillColor;
	bool FillSet;

	bool** Fillcheck;

	CKhuGleSprite* SobelFilter;
	bool SobelSet;
	bool SobelCheck;

	double** TempR;
	double** TempG;
	double** TempB;

	CKhuGleSprite* ChangeYCbCr;
	CKhuGleSprite* ChangeHSV;
	CKhuGleSprite* ChangeCMYK;

	bool YCbCrSet;
	bool YCbCrCheck;

	bool HSVSet;
	bool HSVCheck;
	double** TempH;
	double** TempS;
	double** TempV;
	double** TempHSVR;
	double** TempHSVG;
	double** TempHSVB;


	bool CMYKSet;
	bool CMYKCheck;
	double** TempC;
	double** TempM;
	double** TempY;
	double** TempCMYR;
	double** TempCMYG;
	double** TempCMYB;

};

KhuPaintBrush::KhuPaintBrush(int nW, int nH) : CKhuGleWin(nW, nH)
{
	m_pScene = new CKhuGleScene(1066, 685, KG_COLOR_24_RGB(202, 212, 227));
	ImageLayer = new CKhuGleImageLayer(1046, 585, KG_COLOR_24_RGB(255, 255, 255), CKgPoint(10, 90));
	ToolbarLayer = new CKhuGleImageLayer(1066, 80, KG_COLOR_24_RGB(245, 246, 247), CKgPoint(0, 0));

	m_pScene->AddChild(ImageLayer);
	m_pScene->AddChild(ToolbarLayer);


	LButtonPoint = { 0, 0 };
	LButtonTemp = { 0, 0 };

	BrushColor = KG_COLOR_24_RGB(0, 0, 0);
	EraseColor = KG_COLOR_24_RGB(255, 255, 255);

	BrushStatus = PAINT;
	ToolbarStatus = IDLE;

	// Palette 초기화
	for (int i = 0; i < 30; i++)
	{
		if (i < 10)
		{
			PaletteColor[i] = KG_COLOR_24_RGB(25 * i, 0, 0);
			Palette[i] = new CKhuGleSprite(GP_STYPE_RECT, GP_CTYPE_STATIC, CKgLine(CKgPoint(810 + (20 * i), 10), CKgPoint(825 + (20 * i), 25)), PaletteColor[i], true, 0);
			Palette[i]->m_brush = COLOR;
		}
		else if (i >= 10 && i < 20)
		{
			PaletteColor[i] = KG_COLOR_24_RGB(0, 25 * (i - 10), 0);
			Palette[i] = new CKhuGleSprite(GP_STYPE_RECT, GP_CTYPE_STATIC, CKgLine(CKgPoint(810 + (20 * (i - 10)), 30), CKgPoint(825 + (20 * (i - 10)), 45)), PaletteColor[i], true, 0);
			Palette[i]->m_brush = COLOR;
		}
		else
		{
			PaletteColor[i] = KG_COLOR_24_RGB(0, 0, 25 * (i - 20));
			Palette[i] = new CKhuGleSprite(GP_STYPE_RECT, GP_CTYPE_STATIC, CKgLine(CKgPoint(810 + (20 * (i - 20)), 50), CKgPoint(825 + (20 * (i - 20)), 65)), PaletteColor[i], true, 0);
			Palette[i]->m_brush = COLOR;
		}
		ToolbarLayer->AddChild(Palette[i]);
	}

	CurrentColor = new CKhuGleSprite(GP_STYPE_RECT, GP_CTYPE_STATIC, CKgLine(CKgPoint(760, 10), CKgPoint(800, 50)), BrushColor, true, 0);
	ToolbarLayer->AddChild(CurrentColor);

	Pen = new CKhuGleSprite(GP_STYPE_RECT, GP_CTYPE_STATIC, CKgLine(CKgPoint(400, 10), CKgPoint(420, 30)), KG_COLOR_24_RGB(153, 217, 234), true, 0);
	Pen->m_brush = PAINT;
	Erase = new CKhuGleSprite(GP_STYPE_RECT, GP_CTYPE_STATIC, CKgLine(CKgPoint(400, 40), CKgPoint(420, 60)), KG_COLOR_24_RGB(0, 0, 0), false, 0);
	Erase->m_brush = ERASE;
	Text = new CKhuGleSprite(GP_STYPE_RECT, GP_CTYPE_STATIC, CKgLine(CKgPoint(430, 10), CKgPoint(450, 30)), KG_COLOR_24_RGB(0, 0, 0), false, 0);
	Text->m_brush = TEXT;
	Spuit = new CKhuGleSprite(GP_STYPE_RECT, GP_CTYPE_STATIC, CKgLine(CKgPoint(430, 40), CKgPoint(450, 60)), KG_COLOR_24_RGB(0, 0, 0), false, 0);
	Spuit->m_brush = SPUIT;
	Shape_Rect = new CKhuGleSprite(GP_STYPE_RECT, GP_CTYPE_STATIC, CKgLine(CKgPoint(460, 10), CKgPoint(480, 30)), KG_COLOR_24_RGB(0, 0, 0), false, 0);
	Shape_Rect->m_brush = SHAPE_RECT;
	Shape_Ellipse = new CKhuGleSprite(GP_STYPE_RECT, GP_CTYPE_STATIC, CKgLine(CKgPoint(460, 40), CKgPoint(480, 60)), KG_COLOR_24_RGB(0, 0, 0), false, 0);
	Shape_Ellipse->m_brush = SHAPE_ELLIPSE;
	Fullfill = new CKhuGleSprite(GP_STYPE_RECT, GP_CTYPE_STATIC, CKgLine(CKgPoint(490, 10), CKgPoint(510, 30)), KG_COLOR_24_RGB(0, 0, 0), false, 0);
	Fullfill->m_brush = FILL;

	ToolbarLayer->AddChild(Pen);
	ToolbarLayer->AddChild(Erase);
	ToolbarLayer->AddChild(Text);
	ToolbarLayer->AddChild(Spuit);
	ToolbarLayer->AddChild(Shape_Rect);
	ToolbarLayer->AddChild(Shape_Ellipse);
	ToolbarLayer->AddChild(Fullfill);

	CreateErase = false;

	ShapeSet = false;
	TextSet = false;

	contents.clear();

	ImageRead = new CKhuGleSprite(GP_STYPE_RECT, GP_CTYPE_STATIC, CKgLine(CKgPoint(100, 10), CKgPoint(150, 60)), KG_COLOR_24_RGB(0, 0, 0), false, 0);
	ImageRead->m_toolbar = READ;
	ImageWrite = new CKhuGleSprite(GP_STYPE_RECT, GP_CTYPE_STATIC, CKgLine(CKgPoint(160, 10), CKgPoint(210, 60)), KG_COLOR_24_RGB(0, 0, 0), false, 0);
	ImageWrite->m_toolbar = WRITE;

	ToolbarLayer->AddChild(ImageRead);
	ToolbarLayer->AddChild(ImageWrite);

	ImageSet = false;

	PaletteButton = new CKhuGleSprite(GP_STYPE_RECT, GP_CTYPE_STATIC, CKgLine(CKgPoint(700, 10), CKgPoint(750, 60)), KG_COLOR_24_RGB(0, 0, 0), false, 0);
	PaletteButton->m_toolbar = CHOOSE;
	ToolbarLayer->AddChild(PaletteButton);

	ImageSave = false;
	PaletteNum = 0;

	FillSet = false;

	Fillcheck = new bool* [ImageLayer->m_nH];
	for (int y = 0; y < ImageLayer->m_nH; y++)
	{
		Fillcheck[y] = new bool[ImageLayer->m_nW];
	}

	ResetFillCheck();

	SobelFilter = new CKhuGleSprite(GP_STYPE_RECT, GP_CTYPE_STATIC, CKgLine(CKgPoint(340, 10), CKgPoint(390, 60)), KG_COLOR_24_RGB(0, 0, 0), false, 0);
	SobelFilter->m_toolbar = SOBEL;

	ToolbarLayer->AddChild(SobelFilter);
	SobelSet = false;
	SobelCheck = false;

	TempR = dmatrix(ImageLayer->m_nH, ImageLayer->m_nW);
	TempG = dmatrix(ImageLayer->m_nH, ImageLayer->m_nW);
	TempB = dmatrix(ImageLayer->m_nH, ImageLayer->m_nW);

	ChangeYCbCr = new CKhuGleSprite(GP_STYPE_RECT, GP_CTYPE_STATIC, CKgLine(CKgPoint(520, 10), CKgPoint(570, 60)), KG_COLOR_24_RGB(0, 0, 0), false, 0);
	ChangeYCbCr->m_toolbar = YCBCR;
	ChangeHSV = new CKhuGleSprite(GP_STYPE_RECT, GP_CTYPE_STATIC, CKgLine(CKgPoint(580, 10), CKgPoint(630, 60)), KG_COLOR_24_RGB(0, 0, 0), false, 0);
	ChangeHSV->m_toolbar = HSV;
	ChangeCMYK = new CKhuGleSprite(GP_STYPE_RECT, GP_CTYPE_STATIC, CKgLine(CKgPoint(640, 10), CKgPoint(690, 60)), KG_COLOR_24_RGB(0, 0, 0), false, 0);
	ChangeCMYK->m_toolbar = CMYK;

	ToolbarLayer->AddChild(ChangeYCbCr);
	ToolbarLayer->AddChild(ChangeHSV);
	ToolbarLayer->AddChild(ChangeCMYK);

	YCbCrSet = false;
	YCbCrCheck = false;

	HSVSet = false;
	HSVCheck = false;
	TempH = dmatrix(ImageLayer->m_nH, ImageLayer->m_nW);
	TempS = dmatrix(ImageLayer->m_nH, ImageLayer->m_nW);
	TempV = dmatrix(ImageLayer->m_nH, ImageLayer->m_nW);
	TempHSVR = dmatrix(ImageLayer->m_nH, ImageLayer->m_nW);
	TempHSVG = dmatrix(ImageLayer->m_nH, ImageLayer->m_nW);
	TempHSVB = dmatrix(ImageLayer->m_nH, ImageLayer->m_nW);

	CMYKSet = false;
	CMYKCheck = false;

	TempC = dmatrix(ImageLayer->m_nH, ImageLayer->m_nW);
	TempM = dmatrix(ImageLayer->m_nH, ImageLayer->m_nW);
	TempY = dmatrix(ImageLayer->m_nH, ImageLayer->m_nW);
	TempCMYR = dmatrix(ImageLayer->m_nH, ImageLayer->m_nW);
	TempCMYG = dmatrix(ImageLayer->m_nH, ImageLayer->m_nW);
	TempCMYB = dmatrix(ImageLayer->m_nH, ImageLayer->m_nW);
};

KhuPaintBrush::~KhuPaintBrush()
{

}

void KhuPaintBrush::Update()
{
	// Erase 선택 시 경계를 보여주고 충돌을 처리하기 위한 공 생성
	if (BrushStatus == ERASE && CreateErase == false)
	{
		EraseBall = new CKhuGleSprite(GP_STYPE_ELLIPSE, GP_CTYPE_DYNAMIC, CKgLine(CKgPoint(m_MousePosX - 10, m_MousePosY - 10), CKgPoint(m_MousePosX + 10, m_MousePosY + 10)), KG_COLOR_24_RGB(0, 0, 0), false, 100);
		ImageLayer->AddChild(EraseBall);
		CreateErase = true;		
	}
	// Erase 비선택 시 경계 해제 및 공 삭제
	if (BrushStatus != ERASE && CreateErase == true)
	{
		DeleteSprite(ImageLayer ,EraseBall);
		CreateErase = false;
	}

	// Brush 구현
	if (m_bMousePressed[0])
	{
		// Mouse가 ToolbarLayer에 위치해있을 시 
		if (m_MousePosY < 80)
		{
			// 마우스 버튼 클릭 처리 
			for (auto& SpriteA : ToolbarLayer->m_Children)
			{
				CKhuGleSprite* Button = (CKhuGleSprite*)SpriteA;

				// 마우스 포인터의 중심이 Button의 안쪽에 있는지 판별
				if (m_MousePosX > Button->m_rtBoundBox.Left
					&& m_MousePosX < Button->m_rtBoundBox.Right
					&& m_MousePosY > Button->m_rtBoundBox.Top
					&& m_MousePosY < Button->m_rtBoundBox.Bottom)
				{
					// Palette 버튼일 때
					if (Button->m_brush == COLOR)
					{
						BrushColor = Button->m_fgColor;
						CurrentColor->m_fgColor = BrushColor;
					}
					// Pen 버튼일 때 
					else if (Button->m_brush == PAINT)
					{
						BrushStatus = PAINT;
						ButtonInit();
						Pen->m_fgColor = KG_COLOR_24_RGB(153, 217, 234);
						Pen->m_bFill = true;
					}
					// Erase 버튼일 때
					else if (Button->m_brush == ERASE)
					{
						BrushStatus = ERASE;
						ButtonInit();
						Erase->m_fgColor = KG_COLOR_24_RGB(153, 217, 234);
						Erase->m_bFill = true;
					}
					// Text 버튼일 때 
					else if (Button->m_brush == TEXT)
					{
						BrushStatus = TEXT;
						ButtonInit();
						Text->m_fgColor = KG_COLOR_24_RGB(153, 217, 234);
						Text->m_bFill = true;
					}
					// Spuit 버튼일 때
					else if (Button->m_brush == SPUIT)
					{
						BrushStatus = SPUIT;
						ButtonInit();
						Spuit->m_fgColor = KG_COLOR_24_RGB(153, 217, 234);
						Spuit->m_bFill = true;
					}
					// Rect shape 버튼일 때
					else if (Button->m_brush == SHAPE_RECT)
					{
						BrushStatus = SHAPE_RECT;
						ButtonInit();
						Shape_Rect->m_fgColor = KG_COLOR_24_RGB(153, 217, 234);
						Shape_Rect->m_bFill = true;
					}
					// Ellipse shape 버튼일 때
					else if (Button->m_brush == SHAPE_ELLIPSE)
					{
						BrushStatus = SHAPE_ELLIPSE;
						ButtonInit();
						Shape_Ellipse->m_fgColor = KG_COLOR_24_RGB(153, 217, 234);
						Shape_Ellipse->m_bFill = true;
					}
					// FullFill 버튼일 때
					else if (Button->m_brush == FILL)
					{
						BrushStatus = FILL;
						ButtonInit();
						Fullfill->m_fgColor = KG_COLOR_24_RGB(153, 217, 234);
						Fullfill->m_bFill = true;
					}

					// 파일 열기 버튼일 때
					if (Button->m_toolbar == READ)
					{
						ToolbarStatus = READ;
					}
					// 파일 저장 버튼일 때
					else if (Button->m_toolbar == WRITE)
					{
						if (ImageSave == false)
						{
							unsigned char* Palette = new unsigned char[ImageLayer->m_nH * ((ImageLayer->m_nW * 3 + 3) / 4 * 4)];
							int index = 0;
							for (int y = 0; y < ImageLayer->m_nH; y++)
							{
								for (int x = ImageLayer->m_nW - 1; x >= 0; x--)
								{
									Palette[index] = ImageLayer->m_ImageB[y][x];
									Palette[index + 1] = ImageLayer->m_ImageG[y][x];
									Palette[index + 2] = ImageLayer->m_ImageR[y][x];
									index += 3;
								}
							}
							MessageBox(m_pWinApplication->m_hWnd, (LPCSTR)"파일 저장 완료", "파일 저장", MB_OK+MB_ICONINFORMATION);
							SaveBmp("ouput.bmp", Palette, ImageLayer->m_nW, ImageLayer->m_nH);
							ImageSave = true;
							m_bMousePressed[0] = false;
						}
					}
					// 색상 선택 버튼일 때
					else if (Button->m_toolbar == CHOOSE)
					{
						PaletteLayer = new CKhuGleImageLayer(400, 300, KG_COLOR_24_RGB(245, 246, 247), CKgPoint(350, 200));
						PaletteExitButton = new CKhuGleSprite(GP_STYPE_RECT, GP_CTYPE_STATIC, CKgLine(CKgPoint(370, 10), CKgPoint(385, 25)), KG_COLOR_24_RGB(0, 0, 0), false, 0);
						PaletteExitButton->m_toolbar = EXIT;
						m_pScene->AddChild(PaletteLayer);
						PaletteLayer->AddChild(PaletteExitButton);
						PaletteLayer->DrawLine(
							PaletteLayer->m_ImageBgR, PaletteLayer->m_ImageBgG, PaletteLayer->m_ImageBgB,
							PaletteLayer->m_nW, PaletteLayer->m_nH,
							370,
							10,
							385,
							25,
							KG_COLOR_24_RGB(0, 0, 0));
						PaletteLayer->DrawLine(
							PaletteLayer->m_ImageBgR, PaletteLayer->m_ImageBgG, PaletteLayer->m_ImageBgB,
							PaletteLayer->m_nW, PaletteLayer->m_nH,
							385,
							10,
							370,
							25,
							KG_COLOR_24_RGB(0, 0, 0));

						struct RGB {
							int red;
							int green;
							int blue;
						};

						std::vector<RGB> palette;
						int stepSize = 256 / 8;

						for (int r = 0; r <= 256; r += stepSize) 
						{
							for (int g = 0; g <= 256; g += stepSize) 
							{
								for (int b = 0; b <= 256; b += stepSize) 
								{
									RGB color;
									if (r == 256) color.red = 255;
									else color.red = r;
									if (g == 256) color.green = 255;
									else color.green = g;
									if (b == 256) color.blue = 255;
									else color.blue = b;

									palette.push_back(color);
								}
							}
						}

						int count = 0;
						int col = 0;

						for (int i = 0; i < 729; i++)
						{
							if (count > 35)
							{
								count = 0;
								col++;
							}
							PaletteColor_Extention[i] = KG_COLOR_24_RGB(palette[i].red, palette[i].green, palette[i].blue);
							Palette_Extention[i] = new CKhuGleSprite(GP_STYPE_RECT, GP_CTYPE_STATIC, CKgLine(CKgPoint(20 + (10 * (i - (36 * col))), 30 + (10 * col)), CKgPoint(30 + (10 * (i - (36 * col))), 40 + (10 * col))), PaletteColor_Extention[i], true, 0);
							Palette_Extention[i]->m_brush = COLOR;
							PaletteLayer->AddChild(Palette_Extention[i]);
							count++;
						}

						CurrentColor2 = new CKhuGleSprite(GP_STYPE_RECT, GP_CTYPE_STATIC, CKgLine(CKgPoint(200, 250), CKgPoint(380, 280)), BrushColor, true, 0);
						PaletteLayer->AddChild(CurrentColor2);
						CompareColor = new CKhuGleSprite(GP_STYPE_RECT, GP_CTYPE_STATIC, CKgLine(CKgPoint(20, 250), CKgPoint(200, 280)), BrushColor, true, 0);
						PaletteLayer->AddChild(CompareColor);
					}
					// Sobel Filterfing 버튼일 때
					else if (Button->m_toolbar == SOBEL)
					{
						if (SobelSet == false)
						{
							if (SobelCheck == false)
							{
								// 필터링 전 이미지 저장
								for (int y = 0; y < ImageLayer->m_nH; y++)
								{
									for (int x = 0; x < ImageLayer->m_nW; x++)
									{
										TempR[y][x] = ImageLayer->m_ImageBgR[y][x];
										TempG[y][x] = ImageLayer->m_ImageBgG[y][x];
										TempB[y][x] = ImageLayer->m_ImageBgB[y][x];
									}
								}

								// 필터링
								for (int y = 0; y < ImageLayer->m_nH; y++)
								{
									for (int x = 0; x < ImageLayer->m_nW; x++)
									{
										ImageLayer->m_ImageBgR[y][x] = ImageLayer->m_ImageBgG[y][x] = ImageLayer->m_ImageBgB[y][x] = 0;
										if (x > 0 && x < ImageLayer->m_nW - 1 && y > 0 && y < ImageLayer->m_nH - 1)
										{
											double Rx = ImageLayer->m_ImageR[y - 1][x - 1] + 2 * ImageLayer->m_ImageR[y][x - 1] + ImageLayer->m_ImageR[y + 1][x - 1]
												- ImageLayer->m_ImageR[y - 1][x + 1] - 2 * ImageLayer->m_ImageR[y][x + 1] - ImageLayer->m_ImageR[y + 1][x + 1];
											double Ry = ImageLayer->m_ImageR[y - 1][x - 1] + 2 * ImageLayer->m_ImageR[y - 1][x] + ImageLayer->m_ImageR[y - 1][x + 1]
												- ImageLayer->m_ImageR[y + 1][x - 1] - 2 * ImageLayer->m_ImageR[y+1][x] - ImageLayer->m_ImageR[y + 1][x + 1];
											double Gx = ImageLayer->m_ImageG[y - 1][x - 1] + 2 * ImageLayer->m_ImageG[y][x - 1] + ImageLayer->m_ImageG[y + 1][x - 1]
												- ImageLayer->m_ImageG[y - 1][x + 1] - 2 * ImageLayer->m_ImageG[y][x + 1] - ImageLayer->m_ImageG[y + 1][x + 1];
											double Gy = ImageLayer->m_ImageG[y - 1][x - 1] + 2 * ImageLayer->m_ImageG[y - 1][x] + ImageLayer->m_ImageG[y - 1][x + 1]
												- ImageLayer->m_ImageG[y + 1][x - 1] - 2 * ImageLayer->m_ImageG[y + 1][x] - ImageLayer->m_ImageG[y + 1][x + 1];
											double Bx = ImageLayer->m_ImageB[y - 1][x - 1] + 2 * ImageLayer->m_ImageB[y][x - 1] + ImageLayer->m_ImageB[y + 1][x - 1]
												- ImageLayer->m_ImageB[y - 1][x + 1] - 2 * ImageLayer->m_ImageB[y][x + 1] - ImageLayer->m_ImageB[y + 1][x + 1];
											double By = ImageLayer->m_ImageR[y - 1][x - 1] + 2 * ImageLayer->m_ImageR[y - 1][x] + ImageLayer->m_ImageB[y - 1][x + 1]
												- ImageLayer->m_ImageB[y + 1][x - 1] - 2 * ImageLayer->m_ImageB[y + 1][x] - ImageLayer->m_ImageB[y + 1][x + 1];

											ImageLayer->m_ImageBgR[y][x] = sqrt(Rx * Rx + Ry * Ry);
											ImageLayer->m_ImageBgG[y][x] = sqrt(Gx * Gx + Gy * Gy);
											ImageLayer->m_ImageBgB[y][x] = sqrt(Bx * Bx + By * By);
										}
									}
								}
								SobelCheck = true;
							}
							else
							{
								for (int y = 0; y < ImageLayer->m_nH; y++)
								{
									for (int x = 0; x < ImageLayer->m_nW; x++)
									{
										ImageLayer->m_ImageBgR[y][x] = TempR[y][x];
										ImageLayer->m_ImageBgG[y][x] = TempG[y][x];
										ImageLayer->m_ImageBgB[y][x] = TempB[y][x];
									}
								}
								SobelCheck = false;
							}

							SobelSet = true;
						}
					}

					// YCbCr 변환 버튼일 때
					else if (Button->m_toolbar == YCBCR)
					{
						if (YCbCrSet == false)
						{
							if (YCbCrCheck == false)
							{
								// 변환 전 이미지 저장
								for (int y = 0; y < ImageLayer->m_nH; y++)
								{
									for (int x = 0; x < ImageLayer->m_nW; x++)
									{
										TempR[y][x] = ImageLayer->m_ImageBgR[y][x];
										TempG[y][x] = ImageLayer->m_ImageBgG[y][x];
										TempB[y][x] = ImageLayer->m_ImageBgB[y][x];
									}
								}
								//  YCbCr변환
								for (int y = 0; y < ImageLayer->m_nH; y++)
								{
									for (int x = 0; x < ImageLayer->m_nW; x++)
									{
										ImageLayer->m_ImageBgR[y][x] = (0.29900 * TempR[y][x]) + (0.58700 * TempG[y][x]) + (0.11400 * TempB[y][x]);
										ImageLayer->m_ImageBgG[y][x] = (-0.16874 * TempR[y][x]) + (-0.33126 * TempG[y][x]) + (0.50000 * TempB[y][x]);
										ImageLayer->m_ImageBgB[y][x] = (0.50000 * TempR[y][x]) + (-0.41869 * TempG[y][x]) + (-0.08131 * TempB[y][x]);
									}
								}

								YCbCrCheck = true;
							}
							else
							{
								for (int y = 0; y < ImageLayer->m_nH; y++)
								{
									for (int x = 0; x < ImageLayer->m_nW; x++)
									{
										ImageLayer->m_ImageBgR[y][x] = TempR[y][x];
										ImageLayer->m_ImageBgG[y][x] = TempG[y][x];
										ImageLayer->m_ImageBgB[y][x] = TempB[y][x];
									}
								}
								YCbCrCheck = false;
							}
							YCbCrSet = true;
						}
					}

					// HSV 변환 버튼일 때
					else if (Button->m_toolbar == HSV)
					{
						if (HSVSet == false)
						{
							if (HSVCheck == false)
							{
								// 변환 전 이미지 저장
								for (int y = 0; y < ImageLayer->m_nH; y++)
								{
									for (int x = 0; x < ImageLayer->m_nW; x++)
									{
										TempR[y][x] = ImageLayer->m_ImageBgR[y][x];
										TempG[y][x] = ImageLayer->m_ImageBgG[y][x];
										TempB[y][x] = ImageLayer->m_ImageBgB[y][x];
									}
								}
								// HSV 변환
								for (int y = 0; y < ImageLayer->m_nH; y++)
								{
									for (int x = 0; x < ImageLayer->m_nW; x++)
									{
										TempHSVR[y][x] = ImageLayer->m_ImageBgR[y][x] / 255.0;
										TempHSVG[y][x] = ImageLayer->m_ImageBgG[y][x] / 255.0;
										TempHSVB[y][x] = ImageLayer->m_ImageBgB[y][x] / 255.0;

										TempV[y][x] = max(TempHSVR[y][x], max(TempHSVG[y][x], TempHSVB[y][x]));

										if (TempV[y][x] == 0)
										{
											TempS[y][x] = 0;
											TempH[y][x] = 0;
										}
										else
										{
											double min = min(TempHSVR[y][x], min(TempHSVG[y][x], TempHSVB[y][x]));
											// S
											TempS[y][x] = 1 - (min / TempV[y][x]);
 											
											// H 
											if (TempV[y][x] == TempHSVR[y][x])
											{
												TempH[y][x] = 60 * (TempHSVG[y][x] - TempHSVB[y][x]) / (TempV[y][x] - min);
											}
											else if (TempV[y][x] == TempHSVG[y][x])
											{
												TempH[y][x] = 120 + (60 * (TempHSVB[y][x] - TempHSVR[y][x]) / (TempV[y][x] - min));
											}
											else if (TempV[y][x] == TempHSVB[y][x])
											{
												TempH[y][x] = 240 + (60 * (TempHSVR[y][x] - TempHSVG[y][x]) / (TempV[y][x] - min));
											}
											if (TempH[y][x] < 0) TempH[y][x] += 360;
											TempH[y][x] /= 360;
										}

										ImageLayer->m_ImageBgR[y][x] = TempV[y][x] * 255.0;
										ImageLayer->m_ImageBgG[y][x] = TempS[y][x] * 255.0;
										ImageLayer->m_ImageBgB[y][x] = TempH[y][x] * 255.0;
									}
								}

								HSVCheck = true;
							}
							else
							{
								for (int y = 0; y < ImageLayer->m_nH; y++)
								{
									for (int x = 0; x < ImageLayer->m_nW; x++)
									{
										ImageLayer->m_ImageBgR[y][x] = TempR[y][x];
										ImageLayer->m_ImageBgG[y][x] = TempG[y][x];
										ImageLayer->m_ImageBgB[y][x] = TempB[y][x];
									}
								}
								HSVCheck = false;
							}
							HSVSet = true;
						}
					}

					// CMYK 변환 버튼일 때
					else if (Button->m_toolbar == CMYK)
					{
						if (CMYKSet == false)
						{
							if (CMYKCheck == false)
							{
								// 변환 전 이미지 저장
								for (int y = 0; y < ImageLayer->m_nH; y++)
								{
									for (int x = 0; x < ImageLayer->m_nW; x++)
									{
										TempR[y][x] = ImageLayer->m_ImageBgR[y][x];
										TempG[y][x] = ImageLayer->m_ImageBgG[y][x];
										TempB[y][x] = ImageLayer->m_ImageBgB[y][x];
									}
								}
								// 변환
								for (int y = 0; y < ImageLayer->m_nH; y++)
								{
									for (int x = 0; x < ImageLayer->m_nW; x++)
									{
										TempCMYR[y][x] = ImageLayer->m_ImageBgR[y][x] / 255.0;
										TempCMYG[y][x] = ImageLayer->m_ImageBgG[y][x] / 255.0;
										TempCMYB[y][x] = ImageLayer->m_ImageBgB[y][x] / 255.0;

										double K = 1 - max(TempCMYR[y][x], max(TempCMYG[y][x], TempCMYB[y][x]));

										TempC[y][x] = (1 - K - TempCMYR[y][x]) / (1 - K);
										TempM[y][x] = (1 - K - TempCMYG[y][x]) / (1 - K);
										TempY[y][x] = (1 - K - TempCMYB[y][x]) / (1 - K);

										ImageLayer->m_ImageBgR[y][x] = TempC[y][x] * 255.0;
										ImageLayer->m_ImageBgG[y][x] = TempM[y][x] * 255.0;
										ImageLayer->m_ImageBgB[y][x] = TempY[y][x] * 255.0;
									}
								}
								CMYKCheck = true;
							}
							else
							{
								for (int y = 0; y < ImageLayer->m_nH; y++)
								{
									for (int x = 0; x < ImageLayer->m_nW; x++)
									{
										ImageLayer->m_ImageBgR[y][x] = TempR[y][x];
										ImageLayer->m_ImageBgG[y][x] = TempG[y][x];
										ImageLayer->m_ImageBgB[y][x] = TempB[y][x];
									}
								}
								CMYKCheck = false;
							}
							CMYKSet = true;
						}
					}
				}
			}
		}
		// Mouse가 ImageLayer에 위치해있을 시 
		else
		{
			// Palette Layer 동작
			if (find(m_pScene->m_Children.begin(), m_pScene->m_Children.end(), PaletteLayer) != m_pScene->m_Children.end())
			{
				for (auto& SpriteA : PaletteLayer->m_Children)
				{
					LButtonPoint = CKgPoint(m_MousePosX, m_MousePosY);
					LButtonPoint.Y -= 200;
					LButtonPoint.X -= 350;
					CKhuGleSprite* Button = (CKhuGleSprite*)SpriteA;
					if (LButtonPoint.X > Button->m_rtBoundBox.Left
						&& LButtonPoint.X < Button->m_rtBoundBox.Right
						&& LButtonPoint.Y > Button->m_rtBoundBox.Top
						&& LButtonPoint.Y < Button->m_rtBoundBox.Bottom)
					{
						if (Button->m_brush == COLOR)
						{
							BrushColor = Button->m_fgColor;
							CurrentColor->m_fgColor = BrushColor;
							CurrentColor2->m_fgColor = BrushColor;
						}
						if (Button->m_toolbar == EXIT)
						{
							Palette[PaletteNum]->m_fgColor = BrushColor;
							if (PaletteNum < 30) PaletteNum++;
							else PaletteNum = 0;

							m_pScene->DeleteChild();

							m_pScene->AddChild(ImageLayer);
							m_pScene->AddChild(ToolbarLayer);
						}
					}
				}
			}
			// Palette Layer 없을 때 동작
			else
			{
				// Pen 기능
				if (BrushStatus == PAINT)
				{
					// 오른쪽 클릭 시 
					GetMousePointXY();

					// 마우스의 위치가 움직일 때만 동작
					if (LButtonTemp.X != LButtonPoint.X && LButtonTemp.Y != LButtonPoint.Y)
					{
						ImageLayer->DrawLine(
							ImageLayer->m_ImageBgR, ImageLayer->m_ImageBgG, ImageLayer->m_ImageBgB,
							ImageLayer->m_nW, ImageLayer->m_nH,
							LButtonTemp.X,
							LButtonTemp.Y,
							LButtonPoint.X,
							LButtonPoint.Y,
							BrushColor);
						LButtonTemp = LButtonPoint;
					}
				}

				// Erase 기능
				else if (BrushStatus == ERASE)
				{
					GetMousePointXY();
					EraseBall->MoveTo(LButtonPoint.X, LButtonPoint.Y);

					for (int y = EraseBall->m_Center.y - EraseBall->m_Radius; y < EraseBall->m_Center.y + EraseBall->m_Radius; y++)
					{
						if (y < 0) continue;
						else if (y > ImageLayer->m_nH - 1) continue;

						for (int x = EraseBall->m_Center.x - EraseBall->m_Radius; x < EraseBall->m_Center.x + EraseBall->m_Radius; x++)
						{
							if (x < 0) continue;
							else if (x > ImageLayer->m_nW - 1) continue;

							double distance = sqrt(pow(x - EraseBall->m_Center.x, 2) + pow(y - EraseBall->m_Center.y, 2));
							if (distance < EraseBall->m_Radius)
							{
								ImageLayer->m_ImageBgR[y][x] = 255;
								ImageLayer->m_ImageBgG[y][x] = 255;
								ImageLayer->m_ImageBgB[y][x] = 255;
							}
						}
					}
				}

				// Text 기능
				else if (BrushStatus == TEXT)
				{
					if (TextSet == false)
					{
						GetMousePointXY();
						Text_Point = LButtonPoint;

						m_pWinApplication->ButtonCheck2 = true;

						TextSet = true;
					}
				}

				// Spuit 기능
				else if (BrushStatus == SPUIT)
				{
					BrushColor = KG_COLOR_24_RGB(ImageLayer->m_ImageR[LButtonPoint.Y][LButtonPoint.X], ImageLayer->m_ImageG[LButtonPoint.Y][LButtonPoint.X], ImageLayer->m_ImageB[LButtonPoint.Y][LButtonPoint.X]);
					CurrentColor->m_fgColor = BrushColor;
				}

				// Rect Create 기능
				else if (BrushStatus == SHAPE_RECT)
				{
					if (ShapeSet == false)
					{
						GetMousePointXY();
						Shape_Point1 = LButtonPoint;
					}
					GetMousePointXY();
					Shape_Point2 = LButtonPoint;
					ShapeSet = true;
				}

				// Ellipse Create 기능
				else if (BrushStatus == SHAPE_ELLIPSE)
				{
					if (ShapeSet == false)
					{
						GetMousePointXY();
						Shape_Point1 = LButtonPoint;
					}
					GetMousePointXY();
					Shape_Point2 = LButtonPoint;
					ShapeSet = true;
				}

				// Fullfill 기능
				else if (BrushStatus == FILL)
				{
					if (FillSet == false)
					{
						GetMousePointXY();
						FillPoint = LButtonPoint;
						FillColor = KG_COLOR_24_RGB(ImageLayer->m_ImageR[FillPoint.Y][FillPoint.X], ImageLayer->m_ImageG[FillPoint.Y][FillPoint.X], ImageLayer->m_ImageB[FillPoint.Y][FillPoint.X]);
						RecursionFill1(FillPoint.X, FillPoint.Y, FillColor);
						ResetFillCheck();
						RecursionFill2(FillPoint.X, FillPoint.Y, FillColor);
						ResetFillCheck();
						RecursionFill3(FillPoint.X, FillPoint.Y, FillColor);
						ResetFillCheck();
						RecursionFill4(FillPoint.X, FillPoint.Y, FillColor);
						ResetFillCheck();
						FillSet = true;
					}
				}

				// 파일 열기 기능
				if (ToolbarStatus == READ)
				{
					if (ImageSet == false)
					{
						GetMousePointXY();
						Text_Point = LButtonPoint;

						m_pWinApplication->ButtonCheck = true;

						ToolbarStatus = IDLE;
						ImageSet = true;
					}
				}
			}
		}
	}
	// 마우스 클릭 안할 때는 위치 정보만 수정.
	else
	{
		GetMousePointXY();
		LButtonTemp = LButtonPoint;
		if (BrushStatus == ERASE)
		{
			EraseBall->MoveTo(LButtonPoint.X, LButtonPoint.Y);
		}
		else if (BrushStatus == TEXT)
		{
			TextSet = false;
		}
		// Rect 그리기
		else if (BrushStatus == SHAPE_RECT)
		{
			if (ShapeSet == true)
			{
				ImageLayer->DrawLine(
					ImageLayer->m_ImageBgR, ImageLayer->m_ImageBgG, ImageLayer->m_ImageBgB,
					ImageLayer->m_nW, ImageLayer->m_nH,
					Shape_Point1.X,
					Shape_Point1.Y,
					Shape_Point2.X,
					Shape_Point1.Y,
					BrushColor);
				ImageLayer->DrawLine(
					ImageLayer->m_ImageBgR, ImageLayer->m_ImageBgG, ImageLayer->m_ImageBgB,
					ImageLayer->m_nW, ImageLayer->m_nH,
					Shape_Point1.X,
					Shape_Point1.Y,
					Shape_Point1.X,
					Shape_Point2.Y,
					BrushColor);
				ImageLayer->DrawLine(
					ImageLayer->m_ImageBgR, ImageLayer->m_ImageBgG, ImageLayer->m_ImageBgB,
					ImageLayer->m_nW, ImageLayer->m_nH,
					Shape_Point1.X,
					Shape_Point2.Y,
					Shape_Point2.X,
					Shape_Point2.Y,
					BrushColor);
				ImageLayer->DrawLine(
					ImageLayer->m_ImageBgR, ImageLayer->m_ImageBgG, ImageLayer->m_ImageBgB,
					ImageLayer->m_nW, ImageLayer->m_nH,
					Shape_Point2.X,
					Shape_Point1.Y,
					Shape_Point2.X,
					Shape_Point2.Y,
					BrushColor);

				ShapeSet = false;
			}
		}
		// Ellipse 그리기
		else if (BrushStatus == SHAPE_ELLIPSE)
		{
			if (ShapeSet == true)
			{
				CKgRect RtBoundBox = CKgRect(Shape_Point1.X, Shape_Point1.Y, Shape_Point2.X, Shape_Point2.Y);
				double RX = (RtBoundBox.Right - RtBoundBox.Left) / 2.;
				double RY = (RtBoundBox.Bottom - RtBoundBox.Top) / 2.;
				double CX = (RtBoundBox.Right + RtBoundBox.Left) / 2.;
				double CY = (RtBoundBox.Bottom + RtBoundBox.Top) / 2.;

				int nSlice = 100;

				for (int i = 0; i < nSlice; i++)
				{
					double theta1 = 2. * Pi / nSlice * i;
					double theta2 = 2. * Pi / nSlice * (i + 1);

					ImageLayer->DrawLine(ImageLayer->m_ImageBgR, ImageLayer->m_ImageBgG, ImageLayer->m_ImageBgB,
						ImageLayer->m_nW, ImageLayer->m_nH,
						(int)(CX + cos(theta1) * RX), (int)(CY + sin(theta1) * RY),
						(int)(CX + cos(theta2) * RX), (int)(CY + sin(theta2) * RY), BrushColor);
				}

				ShapeSet = false;
			}
		}

		if (FillSet == true)
		{
			FillSet = false;
		}

		if (ImageSet == true)
		{
			ImageSet = false;
		}

		if (ImageSave == true)
		{
			ImageSave = false;
		}

		if (SobelSet == true)
		{
			SobelSet = false;
		}

		if (YCbCrSet == true)
		{
			YCbCrSet = false;
		}

		if (HSVSet == true)
		{
			HSVSet = false;
		}

		if (CMYKSet == true)
		{
			CMYKSet = false;
		}
	}

	// Pen 버튼일 때 
	if (m_bKeyPressed['B'])
	{
		BrushStatus = PAINT;
		ButtonInit();
		Pen->m_fgColor = KG_COLOR_24_RGB(153, 217, 234);
		Pen->m_bFill = true;
	}
	// Erase 버튼일 때
	if (m_bKeyPressed['E'])
	{
		BrushStatus = ERASE;
		ButtonInit();
		Erase->m_fgColor = KG_COLOR_24_RGB(153, 217, 234);
		Erase->m_bFill = true;
	}
	// Text 버튼일 때
	if (m_bKeyPressed['T'])
	{
		BrushStatus = TEXT;
		ButtonInit();
		Text->m_fgColor = KG_COLOR_24_RGB(153, 217, 234);
		Text->m_bFill = true;
	}
	// Spuit 버튼일 때
	if (m_bKeyPressed['S'])
	{
		BrushStatus = SPUIT;
		ButtonInit();
		Spuit->m_fgColor = KG_COLOR_24_RGB(153, 217, 234);
		Spuit->m_bFill = true;
	}
	// Rect shape 버튼일 때
	if (m_bKeyPressed['A'])
	{
		BrushStatus = SHAPE_RECT;
		ButtonInit();
		Shape_Rect->m_fgColor = KG_COLOR_24_RGB(153, 217, 234);
		Shape_Rect->m_bFill = true;
	}
	// Ellipse shape 버튼일 때
	if (m_bKeyPressed['O'])
	{
		BrushStatus = SHAPE_ELLIPSE;
		ButtonInit();
		Shape_Ellipse->m_fgColor = KG_COLOR_24_RGB(153, 217, 234);
		Shape_Ellipse->m_bFill = true;
	}
	// 전체 클리어
	if (m_bKeyPressed['C'])
	{
		for (int y = 0; y < ImageLayer->m_nH; y++)
		{
			for (int x = 0; x < ImageLayer->m_nW; x++)
			{
				ImageLayer->m_ImageBgR[y][x] = 255;
				ImageLayer->m_ImageBgG[y][x] = 255;
				ImageLayer->m_ImageBgB[y][x] = 255;
			}
		}

		contents.clear();
	}

	// File read
	if (m_pWinApplication->textbuf[0] != 0)
	{
		char ExePath[MAX_PATH], ImagePath[MAX_PATH];

		GetModuleFileName(NULL, ExePath, MAX_PATH);

		int i;
		int LastBackSlash = -1;
		int nLen = strlen(ExePath);
		for (i = nLen - 1; i >= 0; i--)
		{
			if (ExePath[i] == '\\') {
				LastBackSlash = i;
				break;
			}
		}

		if (LastBackSlash >= 0)
			ExePath[LastBackSlash] = '\0';

		sprintf(ImagePath, "%s\\%s", ExePath, textbuf);
		ImageLayer->m_Image.ReadBmp(ImagePath);
		ImageLayer->DrawBackgroundImage(Text_Point.X, Text_Point.Y);

		m_pWinApplication->textbuf[0] = '\0';
	}

	if (m_pWinApplication->textbuf2[0] != 0)
	{
		contents.push_back({ textbuf2, Text_Point });
		m_pWinApplication->textbuf2[0] = '\0';
	}

	m_pScene->Render();

	if (!contents.empty())
	{
		for (auto content : contents)
		{
			DrawSceneTextPos(const_cast<char*>(content.first.c_str()), content.second);
		}
	}

	DrawSceneTextPos("색", CKgPoint(770, 50));

	DrawSceneTextPos("색상", CKgPoint(707, 13));
	DrawSceneTextPos("선택", CKgPoint(707, 33));

	DrawSceneTextPos("YCb", CKgPoint(524, 13));
	DrawSceneTextPos("Cr", CKgPoint(533, 33));

	DrawSceneTextPos("HSV", CKgPoint(582, 23));

	DrawSceneTextPos("CM", CKgPoint(647, 13));
	DrawSceneTextPos("YK", CKgPoint(649, 33));

	DrawSceneTextPos("파일", CKgPoint(107, 13));
	DrawSceneTextPos("열기", CKgPoint(107, 33));

	DrawSceneTextPos("파일", CKgPoint(167, 13));
	DrawSceneTextPos("저장", CKgPoint(167, 33));

	DrawSceneTextPos("외곽", CKgPoint(347, 13));
	DrawSceneTextPos("검출", CKgPoint(347, 33));

	DrawSceneTextPos("B", CKgPoint(402, 9));
	DrawSceneTextPos("E", CKgPoint(402, 39));
	DrawSceneTextPos("T", CKgPoint(433, 9));
	DrawSceneTextPos("S", CKgPoint(433, 39));
	DrawSceneTextPos("ㅁ", CKgPoint(461, 6));
	DrawSceneTextPos("o", CKgPoint(464, 37));
	DrawSceneTextPos("F", CKgPoint(494, 9));

	CKhuGleWin::Update();
}

void KhuPaintBrush::ButtonInit()
{
	Pen->m_fgColor = KG_COLOR_24_RGB(0, 0, 0);
	Pen->m_bFill = false;
	Erase->m_fgColor = KG_COLOR_24_RGB(0, 0, 0);
	Erase->m_bFill = false;
	Spuit->m_fgColor = KG_COLOR_24_RGB(0, 0, 0);
	Spuit->m_bFill = false;
	Text->m_fgColor = KG_COLOR_24_RGB(0, 0, 0);
	Text->m_bFill = false;
	Shape_Ellipse->m_fgColor = KG_COLOR_24_RGB(0, 0, 0);
	Shape_Ellipse->m_bFill = false;
	Shape_Rect->m_fgColor = KG_COLOR_24_RGB(0, 0, 0);
	Shape_Rect->m_bFill = false;
	Fullfill->m_fgColor = KG_COLOR_24_RGB(0, 0, 0);
	Fullfill->m_bFill = false;
}

void KhuPaintBrush::GetMousePointXY()
{
	LButtonPoint = CKgPoint(m_MousePosX, m_MousePosY);
	LButtonPoint.Y -= 90;
	LButtonPoint.X -= 10;
}

void KhuPaintBrush::DeleteSprite(CKhuGleImageLayer* targetlayer, CKhuGleSprite* target)
{
	auto NewDelete = std::remove(targetlayer->m_Children.begin(), targetlayer->m_Children.end(), target);
	targetlayer->m_Children.erase(NewDelete, targetlayer->m_Children.end());
	delete target;
}

void KhuPaintBrush::RecursionFill1(int x, int y, KgColor24 color)
{
	if (x < 0 || x > ImageLayer->m_nW - 1 || y < 0 || y > ImageLayer->m_nH - 1 || Fillcheck[y][x] == true)
	{
		return ;
	}
	KgColor24 tempcolor = KG_COLOR_24_RGB(ImageLayer->m_ImageR[y][x], ImageLayer->m_ImageG[y][x], ImageLayer->m_ImageB[y][x]);
	if (tempcolor == BrushColor && tempcolor != color) return;

	if (tempcolor != BrushColor && Fillcheck[y][x] == false && tempcolor == color)
	{
		ImageLayer->m_ImageBgR[y][x] = KgGetRed(BrushColor);
		ImageLayer->m_ImageBgG[y][x] = KgGetGreen(BrushColor);
		ImageLayer->m_ImageBgB[y][x] = KgGetBlue(BrushColor);
		Fillcheck[y][x] = true;

		RecursionFill1(x - 1, y, color);
		RecursionFill1(x, y - 1, color);
	}	
}

void KhuPaintBrush::RecursionFill2(int x, int y, KgColor24 color)
{
	if (x < 0 || x > ImageLayer->m_nW - 1 || y < 0 || y > ImageLayer->m_nH - 1 || Fillcheck[y][x] == true)
	{
		return;
	}
	KgColor24 tempcolor = KG_COLOR_24_RGB(ImageLayer->m_ImageR[y][x], ImageLayer->m_ImageG[y][x], ImageLayer->m_ImageB[y][x]);
	if (tempcolor == BrushColor && tempcolor != color) return;

	if (tempcolor != BrushColor && Fillcheck[y][x] == false && tempcolor == color)
	{
		ImageLayer->m_ImageBgR[y][x] = KgGetRed(BrushColor);
		ImageLayer->m_ImageBgG[y][x] = KgGetGreen(BrushColor);
		ImageLayer->m_ImageBgB[y][x] = KgGetBlue(BrushColor);
		Fillcheck[y][x] = true;

		RecursionFill2(x + 1, y, color);
		RecursionFill2(x, y - 1, color);
	}
}

void KhuPaintBrush::RecursionFill3(int x, int y, KgColor24 color)
{
	if (x < 0 || x > ImageLayer->m_nW - 1 || y < 0 || y > ImageLayer->m_nH - 1 || Fillcheck[y][x] == true)
	{
		return;
	}
	KgColor24 tempcolor = KG_COLOR_24_RGB(ImageLayer->m_ImageR[y][x], ImageLayer->m_ImageG[y][x], ImageLayer->m_ImageB[y][x]);
	if (tempcolor == BrushColor && tempcolor != color) return;

	if (tempcolor != BrushColor && Fillcheck[y][x] == false &&tempcolor == color)
	{
		ImageLayer->m_ImageBgR[y][x] = KgGetRed(BrushColor);
		ImageLayer->m_ImageBgG[y][x] = KgGetGreen(BrushColor);
		ImageLayer->m_ImageBgB[y][x] = KgGetBlue(BrushColor);
		Fillcheck[y][x] = true;

		RecursionFill3(x - 1, y, color);
		RecursionFill3(x, y + 1, color);
	}
}

void KhuPaintBrush::RecursionFill4(int x, int y, KgColor24 color)
{
	if (x < 0 || x > ImageLayer->m_nW - 1 || y < 0 || y > ImageLayer->m_nH - 1 || Fillcheck[y][x] == true)
	{
		return;
	}
	KgColor24 tempcolor = KG_COLOR_24_RGB(ImageLayer->m_ImageR[y][x], ImageLayer->m_ImageG[y][x], ImageLayer->m_ImageB[y][x]);
	if (tempcolor == BrushColor&& tempcolor != color) return;

	if (tempcolor != BrushColor && Fillcheck[y][x] == false &&tempcolor == color)
	{
		ImageLayer->m_ImageBgR[y][x] = KgGetRed(BrushColor);
		ImageLayer->m_ImageBgG[y][x] = KgGetGreen(BrushColor);
		ImageLayer->m_ImageBgB[y][x] = KgGetBlue(BrushColor);
		Fillcheck[y][x] = true;

		RecursionFill4(x + 1, y, color);
		RecursionFill4(x, y + 1, color);
	}
}

void KhuPaintBrush::ResetFillCheck()
{
	for (int y = 0; y < ImageLayer->m_nH; y++)
	{
		for (int x = 0; x < ImageLayer->m_nW; x++)
		{
			Fillcheck[y][x] = false;
		}
	}
}

int main()
{
	KhuPaintBrush* KhuPaint = new KhuPaintBrush(1066, 685);
	KhuGleWinInit(KhuPaint);

	return 0;
} 
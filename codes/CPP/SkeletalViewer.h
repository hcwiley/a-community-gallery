/************************************************************************
*                                                                       *
*   SkeletalViewer.h -- Declares of CSkeletalViewerApp class            *
*                                                                       *
* Copyright (c) Microsoft Corp. All rights reserved.                    *
*                                                                       *
* This code is licensed under the terms of the                          *
* Microsoft Kinect for Windows SDK (Beta) from Microsoft Research       *
* License Agreement: http://research.microsoft.com/KinectSDK-ToU        *
*                                                                       *
************************************************************************/

#pragma once

#include "resource.h"
#include "MSR_NuiApi.h"
#include "DrawDevice.h"
#include "cv.h"
#include "highgui.h"

#define SZ_APPDLG_WINDOW_CLASS        _T("SkeletalViewerAppDlgWndClass")

class CSkeletalViewerApp
{
public:
    HRESULT Nui_Init();
    void                    Nui_UnInit( );
    void                    Nui_GotDepthAlert( );
    void                    Nui_GotVideoAlert( );
    void                    Nui_GotSkeletonAlert( );
    void                    Nui_Zero();
    void                    Nui_BlankSkeletonScreen( HWND hWnd );
    void                    Nui_DoDoubleBuffer(HWND hWnd,HDC hDC);
    void                    Nui_DrawSkeleton( bool bBlank, NUI_SKELETON_DATA * pSkel, HWND hWnd, int WhichSkeletonColor );
    void                    Nui_DrawSkeletonSegment( NUI_SKELETON_DATA * pSkel, int numJoints, ... );
	void					changeImage();
	void					watchSkeleton();
	void					myInit();
	void					cvOverlayImage(IplImage* src, IplImage* overlay, CvPoint location, float S, float D);
	bool					checkKill();
	void					destroyer();
    RGBQUAD                 Nui_ShortToQuad_Depth( USHORT s );

    static LONG CALLBACK    WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static const int D_LENGTH = 70;
    HWND m_hWnd;

	struct IMAGE{
		IplImage* pic;
		int imgNum;
		int galNum;
		float opacity;
		int left;
		int right;
		int top;
		int bottom;
		int _x;
		int _y;
		int _x0;
		int _y0;
		int _width;
		int _height;
		char* title;
		char* artist;
		char* materials;
		char* year;
		char description[4][D_LENGTH];

		int x(int x=0){
			if(x){
				if(_x0 < 0)
					_x0 = x;
				_x = x;
			}
			return _x;
		};
		int y(int y=0){
			if(y){
				if(_y0 < 0)
					_y0 = y;
				_y = y;
			}
			return _y;
		};
		int width(int width=0){
			if(width)
				_width = width;
			else
				_width = abs(left-right);
			return _width;
		};
		int height(int height=0){
			if(height)
				_height = height;
			else
				_height = abs(top-bottom);
			return _height;
		};
		bool over(int x, int y){
			if(x > _x && x < _x + _width){
				if(y > _y && y < _y + _height)
					return true;
			}
			return false;
		}
	};

	void copyImage(IMAGE* dst, IMAGE* src);
	void clearImage(IMAGE* image);
	void newBg(int not);
	void newBgImages(int dir);

	struct PERSON{
		int number;
		int left;
		int right;
		int top;
		int bottom;
		int _x;
		int _y;
		int _width;
		int _height;
		int _area;
		int lx;
		int ly;
		int rx;
		int ry;
		float distance;
		CvScalar color;
		int present;

		void leftHand(int x=0, int y=0){
			if(x && y){
				lx = x * (WIDTH/ K_WIDTH);
				ly = y * (HEIGHT/ K_HEIGHT);
			}
			if (lx < 0)
				lx = 0;
			if (ly < 0)
				ly = 0;
		}
		void rightHand(int x=0, int y=0){
			if(x && y){
				rx = x * (WIDTH/ K_WIDTH);
				ry = y * (HEIGHT/ K_HEIGHT);
			}
			if (rx < 0)
				rx = 0;
			if (ry < 0)
				ry = 0;
		}
		int x(int x=0){
			if(x)
				_x = x;
			else
				_x = ( left + right )/2;
			return _x;
		};
		int y(int y=0){
			if(y)
				_y = y;
			else
				_y = ( top + bottom)/2;
			return _y;
		};
		int width(int width=0){
			if(width)
				_width = width;
			else
				_width = abs(left-right);
			return _width;
		};
		int height(int height=0){
			if(height)
				_height = height;
			else
				_height = abs(top-bottom);
			return _height;
		};
		int area(){
			return 0;
		};
	} Person;

private:
    static DWORD WINAPI     Nui_ProcessThread(LPVOID pParam);


    // NUI and draw stuff
    DrawDevice    m_DrawDepth;
    DrawDevice    m_DrawVideo;

    // thread handling
    HANDLE        m_hThNuiProcess;
    HANDLE        m_hEvNuiProcessStop;

    HANDLE        m_hNextDepthFrameEvent;
    HANDLE        m_hNextVideoFrameEvent;
    HANDLE        m_hNextSkeletonEvent;
    HANDLE        m_pDepthStreamHandle;
    HANDLE        m_pVideoStreamHandle;
    HFONT         m_hFontFPS;
    HPEN          m_Pen[6];
    HDC           m_SkeletonDC;
    HBITMAP       m_SkeletonBMP;
    HGDIOBJ       m_SkeletonOldObj;
    int           m_PensTotal;
    POINT         m_Points[NUI_SKELETON_POSITION_COUNT];
    RGBQUAD       m_rgbWk[640*480];
    int           m_LastSkeletonFoundTime;
    bool          m_bScreenBlanked;
    int           m_FramesTotal;
    int           m_LastFPStime;
    int           m_LastFramesTotal;
	int imgNum;
	char imagePath[20];
	static const int numImages = 4;
	static const int numGalleries = 3;
	int dir;
	int gotPerson;
	bool initDone;
	PERSON person;
	static const int IMG_WIDTH = 200;
	static const int IMG_HEIGHT = 180;
	static const int WIDTH = 1680;
	static const int HEIGHT = 1050;
	static const int K_WIDTH = 320;
	static const int K_HEIGHT = 240;
	IMAGE* images[numGalleries][numImages];
	//IMAGE*  images2[numImages];
	//IMAGE*  images3[numImages];
	//IMAGE*  images4[numImages];
	int gallery;
	char text[30];
	char helpText1[40];
	char helpText2[40];
	IMAGE* curImage;
	IMAGE* lastImage;
	IplImage* tmpImg;
	IplImage* bgImg0;
	IplImage* bgImg;
	IplImage* lastbgImg;
	IplImage* alphaImg;
	IMAGE* leftHand;
	IMAGE* rightHand;
	IplImage* leftAlpha;
	IplImage* rightAlpha;
	IplImage* leftFull;
	IplImage* rightFull;
	IplImage* leftGreen;
	IplImage* rightGreen;
	IMAGE* leftArrow;
	IMAGE* rightArrow;
	static const int BORDER = 60;
	int curNum;
	time_t imageTimer, curTime;
	CvFont font;
	CvFont titleSmall;
	CvFont title;
	CvFont artist;
	CvFont p;
	double scale;
	static const bool showDistance = false;
	static const int RESET_X = 150;
	static const int RESET_Y = HEIGHT - 220;
	static const int LEFT_ICON_X = 150;
	static const int LEFT_ICON_WIDTH = 85;
	static const int LEFT_ICON_HEIGHT = 70;
	static const int LEFT_ICON_Y = HEIGHT / 2 - LEFT_ICON_HEIGHT/2;
	static const int RIGHT_ICON_X= WIDTH - LEFT_ICON_X - LEFT_ICON_WIDTH;
	static const int RIGHT_ICON_WIDTH = LEFT_ICON_WIDTH;
	static const int RIGHT_ICON_HEIGHT = LEFT_ICON_HEIGHT;
	static const int RIGHT_ICON_Y = HEIGHT / 2 - RIGHT_ICON_HEIGHT/2;
	static const int INFO_BOX_X = 200;
	static const int INFO_BOX_Y = 35;
	static const int INFO_BOX_WIDTH = WIDTH - 400;
	static const int INFO_BOX_HEIGHT = IMG_HEIGHT + 30;
	static const int HELP_TEXT_X = WIDTH / 2 ;
	static const int HELP_TEXT_Y = HEIGHT / 2;
	static const int thresh0 = 2200;
	static const int thresh1 = 2550;
	static const int thresh2 = 3050;
	static const int thresh3 = 3200;
	static const int thresh4 = 3800;
};

int MessageBoxResource(HWND hwnd,UINT nID,UINT nType);




/************************************************************************
*                                                                       *
*   NuiImpl.cpp -- Implementation of CSkeletalViewerApp methods dealing *
*                  with NUI processing                                  *
*                                                                       *
* Copyright (c) Microsoft Corp. All rights reserved.                    *
*                                                                       *
* This code is licensed under the terms of the                          *
* Microsoft Kinect for Windows SDK (Beta) from Microsoft Research       *
* License Agreement: http://research.microsoft.com/KinectSDK-ToU        *
*                                                                       *
************************************************************************/

#include "stdafx.h"
#include <stdio.h>
#include "SkeletalViewer.h"
#include "resource.h"
#include <mmsystem.h>
#include <iostream>
#include <fstream>
using namespace std;
//#include "highgui.h"
//#include "cv.h."

static const COLORREF g_JointColorTable[NUI_SKELETON_POSITION_COUNT] = 
{
    RGB(169, 176, 155), // NUI_SKELETON_POSITION_HIP_CENTER
    RGB(169, 176, 155), // NUI_SKELETON_POSITION_SPINE
    RGB(168, 230, 29), // NUI_SKELETON_POSITION_SHOULDER_CENTER
    RGB(200, 0,   0), // NUI_SKELETON_POSITION_HEAD
    RGB(79,  84,  33), // NUI_SKELETON_POSITION_SHOULDER_LEFT
    RGB(84,  33,  42), // NUI_SKELETON_POSITION_ELBOW_LEFT
    RGB(255, 126, 0), // NUI_SKELETON_POSITION_WRIST_LEFT
    RGB(215,  86, 0), // NUI_SKELETON_POSITION_HAND_LEFT
    RGB(33,  79,  84), // NUI_SKELETON_POSITION_SHOULDER_RIGHT
    RGB(33,  33,  84), // NUI_SKELETON_POSITION_ELBOW_RIGHT
    RGB(77,  109, 243), // NUI_SKELETON_POSITION_WRIST_RIGHT
    RGB(37,   69, 243), // NUI_SKELETON_POSITION_HAND_RIGHT
    RGB(77,  109, 243), // NUI_SKELETON_POSITION_HIP_LEFT
    RGB(69,  33,  84), // NUI_SKELETON_POSITION_KNEE_LEFT
    RGB(229, 170, 122), // NUI_SKELETON_POSITION_ANKLE_LEFT
    RGB(255, 126, 0), // NUI_SKELETON_POSITION_FOOT_LEFT
    RGB(181, 165, 213), // NUI_SKELETON_POSITION_HIP_RIGHT
    RGB(71, 222,  76), // NUI_SKELETON_POSITION_KNEE_RIGHT
    RGB(245, 228, 156), // NUI_SKELETON_POSITION_ANKLE_RIGHT
    RGB(77,  109, 243) // NUI_SKELETON_POSITION_FOOT_RIGHT
};




void CSkeletalViewerApp::Nui_Zero()
{
    m_hNextDepthFrameEvent = NULL;
    m_hNextVideoFrameEvent = NULL;
    m_hNextSkeletonEvent = NULL;
    m_pDepthStreamHandle = NULL;
    m_pVideoStreamHandle = NULL;
    m_hThNuiProcess=NULL;
    m_hEvNuiProcessStop=NULL;
    ZeroMemory(m_Pen,sizeof(m_Pen));
    m_SkeletonDC = NULL;
    m_SkeletonBMP = NULL;
    m_SkeletonOldObj = NULL;
    m_PensTotal = 6;
    ZeroMemory(m_Points,sizeof(m_Points));
    m_LastSkeletonFoundTime = -1;
    m_bScreenBlanked = false;
    m_FramesTotal = 0;
    m_LastFPStime = -1;
    m_LastFramesTotal = 0;
}



HRESULT CSkeletalViewerApp::Nui_Init()
{
    HRESULT                hr;
    RECT                rc;

    m_hNextDepthFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
    m_hNextVideoFrameEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
    m_hNextSkeletonEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

    GetWindowRect(GetDlgItem( m_hWnd, IDC_SKELETALVIEW ), &rc );
    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;
    HDC hdc = GetDC(GetDlgItem( m_hWnd, IDC_SKELETALVIEW));
    m_SkeletonBMP = CreateCompatibleBitmap( hdc, width, height );
    m_SkeletonDC = CreateCompatibleDC( hdc );
    ::ReleaseDC(GetDlgItem(m_hWnd,IDC_SKELETALVIEW), hdc );
    m_SkeletonOldObj = SelectObject( m_SkeletonDC, m_SkeletonBMP );

    hr = m_DrawDepth.CreateDevice( GetDlgItem( m_hWnd, IDC_DEPTHVIEWER ) );
    if( FAILED( hr ) )
    {
        MessageBoxResource( m_hWnd,IDS_ERROR_D3DCREATE,MB_OK | MB_ICONHAND);
        return hr;
    }
	
    hr = m_DrawDepth.SetVideoType( 320, 240, 320 * 4 );
    if( FAILED( hr ) )
    {
        MessageBoxResource( m_hWnd,IDS_ERROR_D3DVIDEOTYPE,MB_OK | MB_ICONHAND);
        return hr;
    }
	
    hr = m_DrawVideo.CreateDevice( GetDlgItem( m_hWnd, IDC_VIDEOVIEW ) );
    if( FAILED( hr ) )
    {
        MessageBoxResource( m_hWnd,IDS_ERROR_D3DCREATE,MB_OK | MB_ICONHAND);
        return hr;
    }

    hr = m_DrawVideo.SetVideoType( 640, 480, 640 * 4 );
    if( FAILED( hr ) )
    {
        MessageBoxResource( m_hWnd,IDS_ERROR_D3DVIDEOTYPE,MB_OK | MB_ICONHAND);
        return hr;
    }
    
    hr = NuiInitialize( 
        NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_SKELETON | NUI_INITIALIZE_FLAG_USES_COLOR);
    if( FAILED( hr ) )
    {
        MessageBoxResource(m_hWnd,IDS_ERROR_NUIINIT,MB_OK | MB_ICONHAND);
        return hr;
    }

    hr = NuiSkeletonTrackingEnable( m_hNextSkeletonEvent, 0 );
    if( FAILED( hr ) )
    {
        MessageBoxResource(m_hWnd,IDS_ERROR_SKELETONTRACKING,MB_OK | MB_ICONHAND);
        return hr;
    }

    hr = NuiImageStreamOpen(
        NUI_IMAGE_TYPE_COLOR,
        NUI_IMAGE_RESOLUTION_640x480,
        0,
        2,
        m_hNextVideoFrameEvent,
        &m_pVideoStreamHandle );
    if( FAILED( hr ) )
    {
        MessageBoxResource(m_hWnd,IDS_ERROR_VIDEOSTREAM,MB_OK | MB_ICONHAND);
        return hr;
    }

    hr = NuiImageStreamOpen(
        NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX,
        NUI_IMAGE_RESOLUTION_320x240,
        0,
        2,
        m_hNextDepthFrameEvent,
        &m_pDepthStreamHandle );
    if( FAILED( hr ) )
    {
        MessageBoxResource(m_hWnd,IDS_ERROR_DEPTHSTREAM,MB_OK | MB_ICONHAND);
        return hr;
    }

    // Start the Nui processing thread
    m_hEvNuiProcessStop=CreateEvent(NULL,FALSE,FALSE,NULL);
    m_hThNuiProcess=CreateThread(NULL,0,Nui_ProcessThread,this,0,NULL);

	myInit();
    return hr;
}


void CSkeletalViewerApp::Nui_UnInit( )
{
    ::SelectObject( m_SkeletonDC, m_SkeletonOldObj );
    DeleteDC( m_SkeletonDC );
    DeleteObject( m_SkeletonBMP );

    if( m_Pen[0] != NULL )
    {
        DeleteObject(m_Pen[0]);
        DeleteObject(m_Pen[1]);
        DeleteObject(m_Pen[2]);
        DeleteObject(m_Pen[3]);
        DeleteObject(m_Pen[4]);
        DeleteObject(m_Pen[5]);
        ZeroMemory(m_Pen,sizeof(m_Pen));
    }

    // Stop the Nui processing thread
    if(m_hEvNuiProcessStop!=NULL)
    {
        // Signal the thread
        SetEvent(m_hEvNuiProcessStop);

        // Wait for thread to stop
        if(m_hThNuiProcess!=NULL)
        {
            WaitForSingleObject(m_hThNuiProcess,INFINITE);
            CloseHandle(m_hThNuiProcess);
        }
        CloseHandle(m_hEvNuiProcessStop);
    }

    NuiShutdown( );
    if( m_hNextSkeletonEvent && ( m_hNextSkeletonEvent != INVALID_HANDLE_VALUE ) )
    {
        CloseHandle( m_hNextSkeletonEvent );
        m_hNextSkeletonEvent = NULL;
    }
    if( m_hNextDepthFrameEvent && ( m_hNextDepthFrameEvent != INVALID_HANDLE_VALUE ) )
    {
        CloseHandle( m_hNextDepthFrameEvent );
        m_hNextDepthFrameEvent = NULL;
    }
    if( m_hNextVideoFrameEvent && ( m_hNextVideoFrameEvent != INVALID_HANDLE_VALUE ) )
    {
        CloseHandle( m_hNextVideoFrameEvent );
        m_hNextVideoFrameEvent = NULL;
    }
    m_DrawDepth.DestroyDevice( );
    m_DrawVideo.DestroyDevice( );
	cvDestroyWindow("virtual gallery");
	for(int i =0; i < numImages; i++){
		cvReleaseImage(&images[gallery][i]->pic);
		free(&images[gallery][i]);
	}
	cvReleaseImage(&tmpImg);
	cvReleaseImage(&alphaImg);
	cvReleaseImage(&bgImg);
	free(&curImage);
	free(&lastImage);
}

void CSkeletalViewerApp::myInit(){
	cvNamedWindow("virtual gallery", CV_WINDOW_FULLSCREEN);
	initDone = false;
	scale = 1.0;
	imgNum = 1;
	person.present = -1;
	person.left = -1;
	person.right = -1;
	curImage = new IMAGE;
	lastImage = new IMAGE;
	bgImg = cvCreateImage(cvSize(WIDTH, HEIGHT),8,3);
	cvRectangle(bgImg,cvPoint(0,0),cvPoint(WIDTH, HEIGHT), cvScalar(255,255,255), CV_FILLED);
	tmpImg = cvCreateImage(cvSize(1280,1024),8,3);
	curNum = -1;
	sprintf(imagePath,"leftHand.png");
	leftHand = new IMAGE;
	leftFull = cvLoadImage(imagePath);
	leftHand->opacity = 1;
	sprintf(imagePath,"leftHand-alpha.png");
	leftAlpha = cvLoadImage(imagePath);
	sprintf(imagePath,"leftHand-green.png");
	leftGreen = cvLoadImage(imagePath);
	leftHand->pic = leftFull;
	leftHand->width(leftHand->pic->width);
	leftHand->height(leftHand->pic->height);
	sprintf(imagePath,"rightHand.png");
	rightHand = new IMAGE;
	rightHand->opacity = 1;
	rightFull = cvLoadImage(imagePath);
	sprintf(imagePath,"rightHand-alpha.png");
	rightAlpha = cvLoadImage(imagePath);
	sprintf(imagePath,"rightHand-green.png");
	rightGreen = cvLoadImage(imagePath);
	rightHand->pic = rightFull;
	rightHand->width(rightHand->pic->width);
	rightHand->height(rightHand->pic->height);
	sprintf(imagePath,"leftArrow.png");
	leftArrow = new IMAGE;
	leftArrow->pic = cvLoadImage(imagePath);
	leftArrow->x(LEFT_ICON_X);
	leftArrow->y(LEFT_ICON_Y);
	leftArrow->width(LEFT_ICON_WIDTH);
	leftArrow->height(LEFT_ICON_HEIGHT);
	rightArrow = new IMAGE;
	sprintf(imagePath,"rightArrow.png");
	rightArrow->pic = cvLoadImage(imagePath);
	rightArrow->x(RIGHT_ICON_X);
	rightArrow->y(RIGHT_ICON_Y);
	rightArrow->width(RIGHT_ICON_WIDTH);
	rightArrow->height(RIGHT_ICON_HEIGHT);
	for(int j = 0; j < numGalleries; j++){
		for(int i = 0; i < numImages; i++){
			sprintf(imagePath,"..\\gallery%d\\%d.jpg", j+1, i+1);
			images[j][i] = new IMAGE;
			images[j][i]->pic = cvLoadImage(imagePath);
			IplImage* tmp = cvCreateImage(cvSize(images[j][i]->pic->width-BORDER, images[j][i]->pic->height-BORDER),8,3);
			cvResize(images[j][i]->pic, tmp);
			cvRectangle(images[j][i]->pic, cvPoint(0,0),cvPoint(images[j][i]->pic->width, images[j][i]->pic->height), cvScalar(255,255,255), CV_FILLED);
			cvOverlayImage(images[j][i]->pic, tmp, cvPoint(BORDER/2,BORDER/2), .8, .8);
			cvReleaseImage(&tmp);
			images[j][i]->x(IMAGE_START_X +(i*(IMG_WIDTH + 100)));
			images[j][i]->y(40);
			images[j][i]->width(IMG_WIDTH);
			images[j][i]->height(IMG_HEIGHT);
			images[j][i]->imgNum = i;
			images[j][i]->galNum = j;
			int length;
			ifstream is;
			//title
			sprintf(imagePath,"..\\gallery%d\\%d-title.txt", j+1, i+1);
			is.open(imagePath);
			is.seekg(0, ios::end);
			length = is.tellg();
			is.seekg (0, ios::beg);
			images[j][i]->title = new char[length+1];
			is.getline(images[j][i]->title, length);
			is.close();
			//artist
			sprintf(imagePath,"..\\gallery%d\\%d-artist.txt", j+1, i+1);
			is.open(imagePath);
			is.seekg(0, ios::end);
			length = is.tellg();
			is.seekg (0, ios::beg);
			images[j][i]->artist = new char[length+1];
			is.getline(images[j][i]->artist, length);
			is.close();
			//materials
			sprintf(imagePath,"..\\gallery%d\\%d-materials.txt", j+1, i+1);
			is.open(imagePath);
			is.seekg(0, ios::end);
			length = is.tellg();
			is.seekg (0, ios::beg);
			images[j][i]->materials = new char[length+1];
			is.getline(images[j][i]->materials, length);
			is.close();
			//year
			sprintf(imagePath,"..\\gallery%d\\%d-year.txt", j+1, i+1);
			is.open(imagePath);
			is.seekg(0, ios::end);
			length = is.tellg();
			is.seekg (0, ios::beg);
			images[j][i]->year = new char[length+1];
			is.getline(images[j][i]->year, length);
			is.close();
			sprintf(imagePath,"..\\gallery%d\\%d-materials.txt", j+1, i+1);
			is.open(imagePath);
			is.seekg(0, ios::end);
			length = is.tellg();
			is.seekg (0, ios::beg);
			images[j][i]->materials = new char[length+1];
			is.getline(images[j][i]->materials, length);
			is.close();
			//description
		}
	}
	gallery = 0;
	cvInitFont(&font, CV_FONT_HERSHEY_PLAIN, 3, 3, (0,0), 3);
	cvInitFont(&titleSmall, CV_FONT_HERSHEY_PLAIN, 1, 1, (0,0), 1);
	cvInitFont(&title, CV_FONT_HERSHEY_PLAIN, 2, 2, (0,0), 1);
	cvInitFont(&artist, CV_FONT_HERSHEY_PLAIN, 1.7, 1.7, (0,0), 1);
	cvInitFont(&p, CV_FONT_HERSHEY_PLAIN, 1, 1, (0,0), 1);
	bgImg0 = cvCloneImage(bgImg);
	newBg(-1);
	person.number = 0;
	person.lry = 5;
	person.rry = 5;
	sprintf(helpText1 , "Step forward to grab an image");
	sprintf(helpText2 , "");
	CvSize tmp;
	int base;
	cvGetTextSize(helpText1, &font, &tmp, &base);
	int x = HELP_TEXT_X - tmp.width/2;
	int y = HELP_TEXT_Y - tmp.height / 2 - 10;
	cvPutText(bgImg0, helpText1,cvPoint(x,y),&font, cvScalar(217,149,0));
	cvShowImage("virtual gallery",bgImg0);
	//curImage = images1[0];
	initDone = true;
}

void CSkeletalViewerApp::newBg(int not){
	if(not == -1 || not != curNum){
		cvRectangle(bgImg0,cvPoint(0,0),cvPoint(WIDTH, HEIGHT), cvScalar(255,255,255), CV_FILLED);
		for(int i = 0; i < numImages; i++){
			if(i!=not){
				tmpImg = cvCreateImage(cvSize(IMG_WIDTH, IMG_HEIGHT),8,3);
				cvResize(images[gallery][i]->pic, tmpImg, 1);
				cvOverlayImage(bgImg0, tmpImg, cvPoint(images[gallery][i]->_x0, images[gallery][i]->_y0),0.2, 0.4);
				CvSize tmp;
				int base;
				cvGetTextSize(images[gallery][i]->title, &titleSmall, &tmp, &base);
				int x = images[gallery][i]->_x0 + (images[gallery][i]->_width / 2) - tmp.width/2; 
				cvPutText(bgImg0, images[gallery][i]->title,cvPoint(x, images[gallery][i]->_y0 + images[gallery][i]->_height + 20),&titleSmall,cvScalar(0,0,0));
				cvReleaseImage(&tmpImg);
			}
			else{
				tmpImg = cvCreateImage(cvSize(IMG_WIDTH, IMG_HEIGHT),8,3);
				cvResize(images[gallery][i]->pic, tmpImg, 1);
				cvOverlayImage(bgImg0, tmpImg, cvPoint(images[gallery][i]->_x0, images[gallery][i]->_y0),1, 1);
				CvSize tmp;
				int base;
				cvGetTextSize(images[gallery][i]->title, &titleSmall, &tmp, &base);
				int x = images[gallery][i]->_x0 + (images[gallery][i]->_width / 2) - tmp.width/2; 
				cvPutText(bgImg0, images[gallery][i]->title,cvPoint(x, images[gallery][i]->_y0 + images[gallery][i]->_height + 20),&titleSmall,cvScalar(0,0,0));
				cvReleaseImage(&tmpImg);
			}
		}
		curNum = not;
	}
}

void CSkeletalViewerApp::newBgImages(int dir){
	if(dir > 0 && gallery + dir >= numGalleries)
		gallery = 0;
	else if(dir < 0 && gallery + dir < 0)
		gallery = numGalleries - 1;
	else
		gallery += dir;
	if(curImage->_width > 0)
		clearImage(curImage);
	newBg(-1);
}

void CSkeletalViewerApp::clearImage(IMAGE* image){
	cvReleaseImage(&(image->pic));
	image->x(-1);
	image->y(-1);
	image->_x0 = -1;
	image->_y0 = -1;
	image->width(-1);
	image->height(-1);
}

void  CSkeletalViewerApp::cvOverlayImage(IplImage* src, IplImage* overlay, CvPoint location, float S, float D)
{
	int x,y,i;
	int w = overlay->width;
	int h = overlay->height;
	IplImage* tmp = cvCreateImage(cvSize(w,h),8,3);
	cvRectangle(tmp, cvPoint(0,0),cvPoint(w,h), cvScalar(255,255,255), CV_FILLED);
	cvAddWeighted(overlay,D,tmp,1-D,0.0,tmp);
	for(x=0;x < tmp->width;x++)
	{
		if(x+location.x >= src->width) continue;
		for(y=0;y < tmp->height;y++)
		{
			CvPoint pt = {x,y};
			uchar* temp_ptr_over = &((uchar*)(tmp->imageData + tmp->widthStep*pt.y))[pt.x*3];
			pt.x = x+location.x;
			pt.y = y+location.y;
			uchar* temp_ptr_src = &((uchar*)(src->imageData + src->widthStep*pt.y))[pt.x*3];
			if (pt.x < WIDTH && pt.x > 0 && pt.y < HEIGHT && pt.y > 0)
				for(i=0;i<sizeof(temp_ptr_src);i++)
					temp_ptr_src[i] = temp_ptr_over[i];
		}
	}
}

void CSkeletalViewerApp::copyImage(IMAGE* dst, IMAGE* src){
	dst->pic = cvCloneImage(src->pic);
	dst->x(src->_x);
	dst->y(src->_y);
	dst->width(src->_width);
	dst->height(src->_height);
	dst->galNum = src->galNum;
	dst->imgNum = src->imgNum;
	dst->title = src->title;
	dst->artist = src->artist;
	dst->year = src->year;
	dst->materials = src->materials;
	/*
	dst->description[0]= src->description[0];
	dst->description[1]= src->description[1];
	dst->description[2]= src->description[2];
	dst->description[3]= src->description[3];
	*/
}
DWORD WINAPI CSkeletalViewerApp::Nui_ProcessThread(LPVOID pParam)
{
    CSkeletalViewerApp *pthis=(CSkeletalViewerApp *) pParam;
    HANDLE                hEvents[4];
    int                    nEventIdx,t,dt;

    // Configure events to be listened on
    hEvents[0]=pthis->m_hEvNuiProcessStop;
    hEvents[1]=pthis->m_hNextDepthFrameEvent;
    hEvents[2]=pthis->m_hNextVideoFrameEvent;
    hEvents[3]=pthis->m_hNextSkeletonEvent;
    // Main thread loop
    while(1)
    {
        // Wait for an event to be signalled
        nEventIdx=WaitForMultipleObjects(sizeof(hEvents)/sizeof(hEvents[0]),hEvents,FALSE,100);

        // If the stop event, stop looping and exit
        if(nEventIdx==0)
            break;            

        // Perform FPS processing
        t = timeGetTime( );
        if( pthis->m_LastFPStime == -1 )
        {
            pthis->m_LastFPStime = t;
            pthis->m_LastFramesTotal = pthis->m_FramesTotal;
        }
        dt = t - pthis->m_LastFPStime;
        if( dt > 1000 )
        {
            pthis->m_LastFPStime = t;
            int FrameDelta = pthis->m_FramesTotal - pthis->m_LastFramesTotal;
            pthis->m_LastFramesTotal = pthis->m_FramesTotal;
            SetDlgItemInt( pthis->m_hWnd, IDC_FPS, FrameDelta,FALSE );
        }

        // Perform skeletal panel blanking
        if( pthis->m_LastSkeletonFoundTime == -1 )
            pthis->m_LastSkeletonFoundTime = t;
        dt = t - pthis->m_LastSkeletonFoundTime;
        if( dt > 250 )
        {
            if( !pthis->m_bScreenBlanked )
            {
                pthis->Nui_BlankSkeletonScreen( GetDlgItem( pthis->m_hWnd, IDC_SKELETALVIEW ) );
                pthis->m_bScreenBlanked = true;
            }
        }

        // Process signal events
        switch(nEventIdx)
        {
			case 1:
				if(pthis->checkKill()){
					return (0);
				}
                pthis->Nui_GotDepthAlert();
                pthis->m_FramesTotal++;
                break;

            case 2:
                pthis->Nui_GotVideoAlert();
                break;

            case 3:
                pthis->Nui_GotSkeletonAlert( );
                break;
        }
    }

    return (0);
}

bool CSkeletalViewerApp::checkKill(){
	if(initDone){
		try
		{
			cvGetWindowName(cvGetWindowHandle("virtual gallery"));
		}
		catch (...)
		{
			destroyer();
		}
	}
	return false;
}

void CSkeletalViewerApp::Nui_GotVideoAlert( ){

    const NUI_IMAGE_FRAME * pImageFrame = NULL;

    HRESULT hr = NuiImageStreamGetNextFrame(
        m_pVideoStreamHandle,
        0,
        &pImageFrame );
    if( FAILED( hr ) )
    {
        return;
    }

    NuiImageBuffer * pTexture = pImageFrame->pFrameTexture;
    KINECT_LOCKED_RECT LockedRect;
    pTexture->LockRect( 0, &LockedRect, NULL, 0 );
    if( LockedRect.Pitch != 0 )
    {
        BYTE * pBuffer = (BYTE*) LockedRect.pBits;

        m_DrawVideo.DrawFrame( (BYTE*) pBuffer );

    }
    else
    {
        OutputDebugString( L"Buffer length of received texture is bogus\r\n" );
    }

    NuiImageStreamReleaseFrame( m_pVideoStreamHandle, pImageFrame );
}

void CSkeletalViewerApp::Nui_GotDepthAlert( )
{
    const NUI_IMAGE_FRAME * pImageFrame = NULL;

    HRESULT hr = NuiImageStreamGetNextFrame(
        m_pDepthStreamHandle,
        0,
        &pImageFrame );

    if( FAILED( hr ) )
    {
        return;
    }

    NuiImageBuffer * pTexture = pImageFrame->pFrameTexture;
    KINECT_LOCKED_RECT LockedRect;
    pTexture->LockRect( 0, &LockedRect, NULL, 0 );
    if( LockedRect.Pitch != 0 )
    {
        BYTE * pBuffer = (BYTE*) LockedRect.pBits;

        // draw the bits to the bitmap
        RGBQUAD * rgbrun = m_rgbWk;
        USHORT * pBufferRun = (USHORT*) pBuffer;
        for( int y = 0 ; y < 240 ; y++ )
        {
            for( int x = 0 ; x < 320 ; x++ )
            {
                RGBQUAD quad = Nui_ShortToQuad_Depth( *pBufferRun );
                pBufferRun++;
                *rgbrun = quad;
                rgbrun++;
            }
        }
        m_DrawDepth.DrawFrame( (BYTE*) m_rgbWk );
    }
    else
    {
        OutputDebugString( L"Buffer length of received texture is bogus\r\n" );
    }

    NuiImageStreamReleaseFrame( m_pDepthStreamHandle, pImageFrame );
}



void CSkeletalViewerApp::Nui_BlankSkeletonScreen(HWND hWnd)
{
    HDC hdc = GetDC( hWnd );
    RECT rct;
    GetClientRect(hWnd, &rct);
    int width = rct.right;
    int height = rct.bottom;
    PatBlt( hdc, 0, 0, width, height, BLACKNESS );
    ReleaseDC( hWnd, hdc );
}

void CSkeletalViewerApp::Nui_DrawSkeletonSegment( NUI_SKELETON_DATA * pSkel, int numJoints, ... )
{
    va_list vl;
    va_start(vl,numJoints);
    POINT segmentPositions[NUI_SKELETON_POSITION_COUNT];

    for (int iJoint = 0; iJoint < numJoints; iJoint++)
    {
        NUI_SKELETON_POSITION_INDEX jointIndex = va_arg(vl,NUI_SKELETON_POSITION_INDEX);
		segmentPositions[iJoint].x = m_Points[jointIndex].x;
        segmentPositions[iJoint].y = m_Points[jointIndex].y;
    }
	
    Polyline(m_SkeletonDC, segmentPositions, numJoints);

    va_end(vl);
}

void CSkeletalViewerApp::watchSkeleton(){
	//if(time(&curTime) - imageTimer > 0.002){
	if(initDone){
		time(&imageTimer);
		//lastbgImg = cvCloneImage(bgImg);
		cvReleaseImage(&bgImg);
		bgImg = cvCloneImage(bgImg0);
		cvReleaseImage(&tmpImg);
		scale = 1;
		scale = abs(person.rx);
		scale /= WIDTH/2;
		scale += 1.0;
		sprintf(text,"%f   %i",person.distance, person.number);
		if( scale <= .8)
			scale = .8;
		if( scale > 3)
			scale = 3;
		//scale = 1.0;
		int x = 0;
		int y = 0;
		x = person.lx;
		y = person.ly;
		if(person.distance < thresh1){
			for(int i=0; i< numImages; i++){
				if(images[gallery][i]->over(person.lx,person.ly) || images[gallery][i]->over(person.rx,person.ry)){
					leftHand->pic = leftGreen;
					rightHand->pic = rightGreen;
					if(curImage->_width > 0){
						copyImage(lastImage, curImage);
						lastImage->x(lastImage->_x0);
						lastImage->y(lastImage->_y0);
					}
					copyImage(curImage, images[gallery][i]);
					newBg(i);
				}
			}
			if(time(&curTime) - imageTimer > 0.01){
				time(&imageTimer);
				if(leftArrow->over(person.lx,person.ly) || leftArrow->over(person.rx,person.ry)){
					newBgImages(-1);
				}
				else if(rightArrow->over(person.lx,person.ly) || rightArrow->over(person.rx,person.ry)){
					newBgImages(1);
				}
				if(x < RESET_X && y > RESET_Y){
					newBg(-1);
				}
			}
			cvOverlayImage(bgImg, leftArrow->pic, cvPoint(LEFT_ICON_X, LEFT_ICON_Y),1, 1);
			cvOverlayImage(bgImg, rightArrow->pic, cvPoint(RIGHT_ICON_X, RIGHT_ICON_Y),1, 1);
		}
		else if(person.distance < thresh2){
			leftHand->pic = leftAlpha;
			rightHand->pic= rightAlpha;
			if(curImage->_width > 0){
				//if(time(&curTime) - imageTimer > 0.01){
					//time(&imageTimer);
					int w = IMG_WIDTH * scale;
					int h = IMG_HEIGHT * scale;
					w = person.rx - person.lx;
					w *= 1.2;
					scale = w / IMG_WIDTH;
					h = IMG_HEIGHT * scale;
					if(w >= WIDTH)
						w = WIDTH;
					if(h >= HEIGHT)
						h = HEIGHT;
					if(w < 70)
						w = 70;
					if(h < 70)
						h = 70;
					tmpImg = cvCreateImage(cvSize(w,h),8,3);
					if(w > curImage->_width || h > curImage->_height){
						cvResize(images[curImage->galNum][curImage->imgNum]->pic, tmpImg, 1);
					}
					else{
						cvResize(curImage->pic, tmpImg, 1);
					}
					cvReleaseImage(&(curImage->pic));
					curImage->pic = cvCloneImage(tmpImg);
					curImage->width(w);
					curImage->height(h);
					cvReleaseImage(&tmpImg);
					
				if(person.ry < RESET_Y){
					curImage->x(x);
					curImage->y(y);
				}
				//x = curImage->_x - curImage->_width/2;
				//y = curImage->_y - curImage->_height/2;
				if(x < 0)
					x =0;
				if(y<0)
					y=0;
				if(x > WIDTH)
					x= WIDTH;
				if(y> HEIGHT)
					y= HEIGHT;
				curImage->x(x);
				curImage->y(y);
				cvOverlayImage(bgImg, curImage->pic, cvPoint(x,y),1, 1);
				//}
			}
		}
		else{
			if(curNum != -1){
				cvOverlayImage(bgImg, curImage->pic, cvPoint(curImage->_x,curImage->_y), 1, 1);
			}
		}
		if(showDistance){
			cvRectangle(bgImg,cvPoint(0,50),cvPoint(500,100), cvScalar(0,0,0), CV_FILLED);
			cvPutText(bgImg, text,cvPoint(100,100),&font,cvScalar(0,0,255));
		}
		//cvLine(bgImg, cvPoint(0, RESET_Y), cvPoint(WIDTH, RESET_Y), cvScalar(30,220,30),20);
		//cvReleaseImage(&lastbgImg);
		if(person.distance > thresh1 && curImage->_width > 0){
			CvSize tmp;
			int base;
			cvGetTextSize(curImage->title, &title, &tmp, &base);
			int w = INFO_BOX_WIDTH;
			if (tmp.width > w )
				w = tmp.width + 20;
			int x = INFO_BOX_X + w - tmp.width - 10;
			int y = INFO_BOX_Y + 20 + tmp.height;
			cvRectangle(bgImg,cvPoint(INFO_BOX_X, INFO_BOX_Y),cvPoint(INFO_BOX_X + w, INFO_BOX_Y+INFO_BOX_HEIGHT), cvScalar(255,255,255), CV_FILLED);
			cvPutText(bgImg, curImage->title,cvPoint(x,y),&title,cvScalar(0,0,0));
			cvGetTextSize(curImage->artist, &artist, &tmp, &base);
			x = INFO_BOX_X + w - tmp.width - 20;
			y += tmp.height + 10;
			cvPutText(bgImg, curImage->artist,cvPoint(x,y),&artist,cvScalar(0,0,0));
			cvGetTextSize(curImage->materials, &p, &tmp, &base);
			x = INFO_BOX_X + w - tmp.width - 20;
			y += tmp.height + 10;
			cvPutText(bgImg, curImage->materials,cvPoint(x,y),&p,cvScalar(0,0,0));
			x = INFO_BOX_X + 10;
			y += tmp.height + 10;
			sprintf(imagePath,"..\\gallery%d\\%d-description.txt", curImage->galNum+1, curImage->imgNum+1);
			int length;
			ifstream is;
			char* tmpC;
			is.open(imagePath);
			is.seekg(0, ios::end);
			length = is.tellg();
			is.seekg (0, ios::beg);
			tmpC = new char[D_LENGTH+1];
			is.getline(tmpC, D_LENGTH, '\n');
			cvPutText(bgImg, tmpC,cvPoint(x,y),&p,cvScalar(0,0,0));
			while(is.good()){
				is.getline(tmpC, D_LENGTH, '\n');
				y += tmp.height + 10;
				cvPutText(bgImg, tmpC,cvPoint(x,y),&p,cvScalar(0,0,0));
			}
			is.close();
		}
		else if( person.distance < thresh1){
			cvRectangle(bgImg,cvPoint(LEFT_ICON_X + LEFT_ICON_WIDTH + 20,HELP_TEXT_Y - 200),cvPoint(RIGHT_ICON_X - 20,HELP_TEXT_Y + 200), cvScalar(255,255,255), CV_FILLED);
			sprintf(helpText1 , "Put your hand over an image");
			sprintf(helpText2 , "Step back to grab it off the wall");
			CvSize tmp;
			int base;
			cvGetTextSize(helpText1, &font, &tmp, &base);
			int x = HELP_TEXT_X - tmp.width/2;
			int y = HELP_TEXT_Y - tmp.height / 2 - 10;
			cvPutText(bgImg, helpText1,cvPoint(x,y),&font, cvScalar(217,149,0));
			cvGetTextSize(helpText2, &font, &tmp, &base);
			x = HELP_TEXT_X - tmp.width/2;
			y = HELP_TEXT_Y + tmp.height / 2  + 10;
			cvPutText(bgImg, helpText2,cvPoint(x,y),&font, cvScalar(217,149,0));
			leftHand->pic = leftFull;
			rightHand->pic = rightFull;
		}
		if( person.distance < thresh2){
			leftHand->x(person.lx - (leftHand->_width / 2));
			leftHand->y(person.ly - (leftHand->_height / 2));
			rightHand->x(person.rx - (rightHand->_width / 2));
			rightHand->y(person.ry - (rightHand->_height / 2));
			cvOverlayImage(bgImg, leftHand->pic, cvPoint(leftHand->_x, leftHand->_y), 1,leftHand->opacity);
			cvOverlayImage(bgImg, rightHand->pic, cvPoint(rightHand->_x, rightHand->_y), 1,rightHand->opacity);
		}
		if( (person.lry < 0 || person.rry < 0) && person.distance < thresh2){
			cvRectangle(bgImg,cvPoint(LEFT_ICON_X + LEFT_ICON_WIDTH + 20,HELP_TEXT_Y - 200),cvPoint(RIGHT_ICON_X - 20,HELP_TEXT_Y + 200), cvScalar(255,255,255), CV_FILLED);
			sprintf(helpText1 , "Your hands are too high");
			sprintf(helpText2 , "Lower them slowly till the hands respond");
			CvSize tmp;
			int base;
			cvGetTextSize(helpText1, &font, &tmp, &base);
			int x = HELP_TEXT_X - tmp.width/2;
			int y = HELP_TEXT_Y - tmp.height / 2 - 10;
			cvPutText(bgImg, helpText1,cvPoint(x,y),&font, cvScalar(217,149,0));
			cvGetTextSize(helpText2, &font, &tmp, &base);
			x = HELP_TEXT_X - tmp.width/2;
			y = HELP_TEXT_Y + tmp.height / 2  + 10;
			cvPutText(bgImg, helpText2,cvPoint(x,y),&font, cvScalar(217,149,0));
			leftHand->pic = leftFull;
			rightHand->pic = rightFull;
		}
		cvShowImage("virtual gallery",bgImg);
	}
}

void CSkeletalViewerApp::changeImage(){
		//cvRectangle(curImage,cvPoint(0,50),cvPoint(600,100), cvScalar(255,255,255), 40);
		//cvPutText(curImage, text,cvPoint(100,100),&font,cvScalar(0,0,255));
	/*
		if( &curImage ){
			int xs = normalD * 1280 * zoom;//.1;
			int ys = normalD * 1024 * zoom;//.1;
			tmpImg = cvCreateImage(cvSize(1280 + xs,1024 + ys),8,3);
			cvResize(curImage, tmpImg, 1);
			cvReleaseImage(&curImage);
			curImage = cvCloneImage(tmpImg);
			cvSetImageROI(curImage, cvRect(xs/2, ys/2, 1280, 1024));
			cvReleaseImage(&tmpImg);
			double alpha = 0;
			double beta = 1;
			float step = 1.0 / loop;
			for(int i = 0; i < loop; i++){
				cvAddWeighted(curImage, alpha, lastImage, beta, 0.0, alphaImg);
				alpha += step;
				beta -= step;
				cvShowImage("virtual gallery", alphaImg);
			}
			cvShowImage("virtual gallery", curImage);
		}
	}
	*/
}

void CSkeletalViewerApp::Nui_DrawSkeleton( bool bBlank, NUI_SKELETON_DATA * pSkel, HWND hWnd, int WhichSkeletonColor )
{
    HGDIOBJ hOldObj = SelectObject(m_SkeletonDC,m_Pen[WhichSkeletonColor % m_PensTotal]);
    
    RECT rct;
    GetClientRect(hWnd, &rct);
    int width = rct.right;
    int height = rct.bottom;

    if( m_Pen[0] == NULL )
    {
        m_Pen[0] = CreatePen( PS_SOLID, width / 80, RGB(255, 0, 0) );
        m_Pen[1] = CreatePen( PS_SOLID, width / 80, RGB( 0, 255, 0 ) );
        m_Pen[2] = CreatePen( PS_SOLID, width / 80, RGB( 64, 255, 255 ) );
        m_Pen[3] = CreatePen( PS_SOLID, width / 80, RGB(255, 255, 64 ) );
        m_Pen[4] = CreatePen( PS_SOLID, width / 80, RGB( 255, 64, 255 ) );
        m_Pen[5] = CreatePen( PS_SOLID, width / 80, RGB( 128, 128, 255 ) );
    }

    if( bBlank )
    {
        PatBlt( m_SkeletonDC, 0, 0, width, height, BLACKNESS );
    }

    int scaleX = width; //scaling up to image coordinates
    int scaleY = height;
    float fx=0,fy=0;
    int i;
    for (i = 0; i < NUI_SKELETON_POSITION_COUNT; i++)
    {
        NuiTransformSkeletonToDepthImageF( pSkel->SkeletonPositions[i], &fx, &fy );
        m_Points[i].x = (int) ( fx * scaleX + 0.5f );
        m_Points[i].y = (int) ( fy * scaleY + 0.5f );
    }

	//if (WhichSkeletonColor != person.number)
		//return;

	SelectObject(m_SkeletonDC,m_Pen[0]);
    
    Nui_DrawSkeletonSegment(pSkel,4,NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_SPINE, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_HEAD);
    Nui_DrawSkeletonSegment(pSkel,5,NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_LEFT, NUI_SKELETON_POSITION_ELBOW_LEFT, NUI_SKELETON_POSITION_WRIST_LEFT, NUI_SKELETON_POSITION_HAND_LEFT);
    Nui_DrawSkeletonSegment(pSkel,5,NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_RIGHT, NUI_SKELETON_POSITION_ELBOW_RIGHT, NUI_SKELETON_POSITION_WRIST_RIGHT, NUI_SKELETON_POSITION_HAND_RIGHT);
    Nui_DrawSkeletonSegment(pSkel,5,NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_LEFT, NUI_SKELETON_POSITION_KNEE_LEFT, NUI_SKELETON_POSITION_ANKLE_LEFT, NUI_SKELETON_POSITION_FOOT_LEFT);
    Nui_DrawSkeletonSegment(pSkel,5,NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_RIGHT, NUI_SKELETON_POSITION_KNEE_RIGHT, NUI_SKELETON_POSITION_ANKLE_RIGHT, NUI_SKELETON_POSITION_FOOT_RIGHT);
    
    // Draw the joints in a different color
    for (i = 0; i < NUI_SKELETON_POSITION_COUNT ; i++)
    {
        HPEN hJointPen;
        
        hJointPen=CreatePen(PS_SOLID,9, g_JointColorTable[i]);
		if(i == 7){
			person.leftHand(m_Points[i].x, m_Points[i].y);
			watchSkeleton();
		}
		else if(i == 11){
			person.rightHand(m_Points[i].x, m_Points[i].y);
			watchSkeleton();
		}
        hOldObj=SelectObject(m_SkeletonDC,hJointPen);

        MoveToEx( m_SkeletonDC, m_Points[i].x, m_Points[i].y, NULL );
        LineTo( m_SkeletonDC, m_Points[i].x, m_Points[i].y );

        SelectObject( m_SkeletonDC, hOldObj );
        DeleteObject(hJointPen);
    }

    return;

}




void CSkeletalViewerApp::Nui_DoDoubleBuffer(HWND hWnd,HDC hDC)
{
    RECT rct;
    GetClientRect(hWnd, &rct);
    int width = rct.right;
    int height = rct.bottom;

    HDC hdc = GetDC( hWnd );

    BitBlt( hdc, 0, 0, width, height, hDC, 0, 0, SRCCOPY );

    ReleaseDC( hWnd, hdc );

}

void CSkeletalViewerApp::Nui_GotSkeletonAlert( )
{
    NUI_SKELETON_FRAME SkeletonFrame;

    HRESULT hr = NuiSkeletonGetNextFrame( 0, &SkeletonFrame );

    bool bFoundSkeleton = true;
    for( int i = 0 ; i < NUI_SKELETON_COUNT ; i++ )
    {
        if( SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED)
        {
            bFoundSkeleton = false;
        }
    }

    // no skeletons!
    //
    if( bFoundSkeleton )
    {
        return;
    }

    // smooth out the skeleton data
    NuiTransformSmooth(&SkeletonFrame,NULL);

    // we found a skeleton, re-start the timer
    m_bScreenBlanked = false;
    m_LastSkeletonFoundTime = -1;

    // draw each skeleton color according to the slot within they are found.
    //
    bool bBlank = true;
    for( int i = 0 ; i < 7 ; i++ )
    {
        if( SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED)
        {
			if(SkeletonFrame.SkeletonData[i].Position.x > -0.4 && SkeletonFrame.SkeletonData[i].Position.x < 0.4 && person.distance > thresh0 && person.distance < thresh3){
				Nui_DrawSkeleton( bBlank, &SkeletonFrame.SkeletonData[i], GetDlgItem( m_hWnd, IDC_SKELETALVIEW ), i);
				bBlank = false;
				break;
			}
			else{
				person.present = -1;
				watchSkeleton();
			}
		}
    }

    Nui_DoDoubleBuffer(GetDlgItem(m_hWnd,IDC_SKELETALVIEW), m_SkeletonDC);
}



RGBQUAD CSkeletalViewerApp::Nui_ShortToQuad_Depth( USHORT s )
{
    USHORT RealDepth = (s & 0xfff8) >> 3;
    USHORT Player = s & 7;

    // transform 13-bit depth information into an 8-bit intensity appropriate
    // for display (we disregard information in most significant bit)
    BYTE l = 255 - (BYTE)(256*RealDepth/0x0fff);

    RGBQUAD q;
    q.rgbRed = q.rgbBlue = q.rgbGreen = 0;
	if (Player > 0 && Player < 7){
		int i = 0;
	}
	if(person.present == 1 && Player == person.number){
		q.rgbRed = 0;
        q.rgbBlue = 1;
        q.rgbGreen = 0;
		person.distance = RealDepth;
	}
	else{
		if (Player > 0 && Player < 7){
			person.number = Player;
			person.present = 1;
			q.rgbRed = 1;
			q.rgbBlue = 0;
			q.rgbGreen = 0;
			person.distance = RealDepth;
		}
		else{
			person.present = -1;
		}
	}
    return q;
}

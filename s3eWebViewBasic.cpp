/*
 * This file is part of the Marmalade SDK Code Samples.
 *
 * Copyright (C) 2001-2011 Ideaworks3D Ltd.
 * All Rights Reserved.
 *
 * This source code is intended only as a supplement to Ideaworks Labs
 * Development Tools and/or on-line documentation.
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 */

#include "ExamplesMain.h"
#include "s3eWebView.h"
#include "UltraTinyHttpd.h"

/**
*
* @page ExampleS3EWebViewBasic S3E Web View Basic Example
*
* Modified verison of SimpleWebView - which starts up a vary basic webserver and uses that to serve local 
* files
*/




//static const char* g_URL = "http://google.com";
static const char* g_URL = "http://127.0.0.1:7777/index.html";		//Default  port 7777

//these are some other tests trying to load local files from file://
//static const char* g_URL = "file:///data/data/com.mycompany.s3ewebviewbasic/files/hello.html";
//static const char* g_URL = "file:///ram://test.html";

static Button* g_ButtonModal = 0;
static Button* g_ButtonShow = 0;
static Button* g_ButtonHide = 0;
static s3eResult g_Result = S3E_RESULT_SUCCESS;
static s3eWebView* g_WebView = NULL;


UltraTinyHttpd httpd;

//end of s3e socket

#include <fstream.h>
#include <iostream.h>

void ExampleInit()
{
	
	
	//start the our webserver
	httpd.start();
	//
	//Listen();
    // Create the UI.
    g_ButtonModal = NewButton("Modal Web View");
    g_ButtonShow  = NewButton("Show Web View");
    g_ButtonHide  = NewButton("Hide Web View");

    // Check for the availability of the Web View functionality.
    // If it is available then enable the button.
    bool avail = (s3eWebViewAvailable() != 0);
    g_ButtonHide->m_Enabled = false;
    g_ButtonModal->m_Enabled = false;
    g_ButtonShow->m_Enabled = avail;
}

void ExampleTerm()
{
    if (g_WebView)
    {
        s3eWebViewDestroy(g_WebView);
        g_WebView = NULL;
    }
}

bool ExampleUpdate()
{
	
    // If the button has been selected, try to execute the OS command
    // and store the result.
    if (GetSelectedButton() == g_ButtonModal)
    {
        g_Result = s3eWebViewCreateModal(g_URL);
    }
    else if (GetSelectedButton() == g_ButtonShow)
    {
        if (!g_WebView)
        {
            g_WebView = s3eWebViewCreate();
           
        }
		 s3eWebViewNavigate(g_WebView, g_URL);

        int y = GetYBelowButtons() + 10;
        s3eWebViewShow(g_WebView, 0, y, s3eSurfaceGetInt(S3E_SURFACE_WIDTH), s3eSurfaceGetInt(S3E_SURFACE_HEIGHT) - y - 10);

        g_ButtonShow->m_Enabled = false;
        g_ButtonHide->m_Enabled = true;
    }
    else if (GetSelectedButton() == g_ButtonHide)
    {
        s3eWebViewHide(g_WebView);
        g_ButtonShow->m_Enabled = true;
        g_ButtonHide->m_Enabled = false;
    }
	
	
    return true;
}

/*
* The following function display the user's prompt and any error messages.
* The s3eDebugPrint() function to print the strings.
*/
void ExampleRender()
{
    // Print results
    if (!s3eWebViewAvailable())
    {
        s3eDebugPrint(50, GetYBelowButtons(), "`xff0000Web view isn't available on this device.", 1);
    }

    if (g_Result == S3E_RESULT_ERROR)
    {
        s3eDebugPrint(50, GetYBelowButtons()    , "`xff0000Failed to open the URL.", 1);
    }
}

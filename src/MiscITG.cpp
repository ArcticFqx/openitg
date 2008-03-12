#include "global.h"
#include "XmlFile.h"
#include "RageUtil.h"
#include "MiscITG.h"
#include "ProfileManager.h"
#include "RageInput.h" /* for g_sInputType */
#include "RageLog.h"
#include "RageTimer.h"
#include "SongManager.h"
#include "ProductInfo.h"
#include "io/ITGIO.h"
#include "io/USBDevice.h"

/* Include dongle support if we're going to use it...
 * Maybe we should set a HAVE_DONGLE directive? */
#ifdef ITG_ARCADE
extern "C" {
#include "ibutton/ownet.h"
#include "ibutton/shaib.h" 
}
#endif

// See where to look for all this stuff
// "Stats" is mounted to "Data" in the VFS for non-arcade
#ifdef ITG_ARCADE
#define STATS_DIR_PATH CString("/rootfs/stats/")
#else
#define STATS_DIR_PATH CString("Data/")
#endif

extern CString g_sInputType;

int GetNumCrashLogs()
{
	// Give ourselves a variable.
	CStringArray aLogs;
	
	// Get them all.
	GetDirListing( STATS_DIR_PATH + "crashinfo-*.txt" , aLogs );
	
	return aLogs.size();
}

int GetNumMachineEdits()
{
	CStringArray aEdits;
	CString sDir = PROFILEMAN->GetProfileDir(PROFILE_SLOT_MACHINE) + EDIT_SUBDIR;
	
	GetDirListing( sDir , aEdits );
	
	return aEdits.size();
}

int GetIP()
{
	return 0;
}

int GetRevision()
{
	CString sPath = STATS_DIR_PATH + "patch/patch.xml";

	// Create the XML Handler, and clear it, for practice.
	XNode *xml = new XNode;
	xml->Clear();
	xml->m_sName = "patch";
	
	// Check for the file existing
	if( !IsAFile(sPath) )
	{
		LOG->Warn( "There is no patch file (patch.xml)" );
		SAFE_DELETE( xml ); 
		return 1;
	}
	
	// Make sure you can read it
	if( !xml->LoadFromFile(sPath) )
	{
		LOG->Warn( "patch.xml unloadable" );
		SAFE_DELETE( xml ); 
		return 1;
	}
	
	// Check the node <Revision>x</Revision>
	if( !xml->GetChild( "Revision" ) )
	{
		LOG->Warn( "Revision node missing! (patch.xml)" );
		SAFE_DELETE( xml ); 
		return 1;
	}
	
	int iRevision = atoi( xml->GetChild("Revision")->m_sValue );

	return iRevision;
}

/* Make sure you delete anything you new!
 * Otherwise, memory leaks may occur -- Vyhd */
int GetNumMachineScores()
{
	CString sXMLPath = STATS_DIR_PATH + "/MachineProfile/Stats.xml";

	// Create the XML Handler and clear it, for practice
	XNode *xml = new XNode;
	xml->Clear();
	
	// Check for the file existing
	if( !IsAFile(sXMLPath) )
	{
		LOG->Warn( "There is no Stats.xml file!" );
		SAFE_DELETE( xml ); 
		return 0;
	}
	
	// Make sure you can read it
	if( !xml->LoadFromFile(sXMLPath) )
	{
		LOG->Trace( "Stats.xml unloadable!" );
		SAFE_DELETE( xml ); 
		return 0;
	}
	
	const XNode *pData = xml->GetChild( "SongScores" );
	
	if( pData == NULL )
	{
		LOG->Warn( "Error loading scores: <SongScores> node missing" );
		SAFE_DELETE( xml ); 
		return 0;
	}
	
	/* Even I'll admit this change is pedantic... -- Vyhd */
	unsigned int iScoreCount = 0;
	
	// Named here, for LoadFromFile() renames it to "Stats"
	xml->m_sName = "SongScores";
	
	// For each pData Child, or the Child in SongScores...
	FOREACH_CONST_Child( pData , p )
		iScoreCount++;

	/* Can't forget about this! Remember to SAFE_DELETE 'new' objects. */
	SAFE_DELETE( xml ); 

	return iScoreCount;
}

CString GetSerialNumber()
{
	if ( !g_SerialNum.empty() )
		return g_SerialNum;

#ifdef ITG_ARCADE
	SHACopr copr;
	CString sNewSerial;
	uchar spBuf[32];

	if ( (copr.portnum = owAcquireEx("/dev/ttyS0")) == -1 )
	{
		LOG->Warn("Failed to get machine serial, unable to acquire port");
		return "????????";
	}
	FindNewSHA(copr.portnum, copr.devAN, true);
	ReadAuthPageSHA18(copr.portnum, 9, spBuf, NULL, false);
	owRelease(copr.portnum);

	sNewSerial = (char*)spBuf;
	TrimLeft(sNewSerial);
	TrimRight(sNewSerial);
	g_SerialNum = sNewSerial;

	return sNewSerial;
#endif

	/* Dummy return */
	return "ITG-C-02242008-529-3";
}

bool HubIsConnected()
{
	vector<USBDevice> vDevices;
	GetUSBDeviceList( vDevices );

	/* Hub can't be connected if there are no devices. */
	if( vDevices.size() == 0 )
		return false;

	for( unsigned i = 0; i < vDevices.size(); i++ )
		if( vDevices[i].IsHub() )
			return true;

	return false;
}

/*
 * [ScreenArcadeDiagnostics]
 *
 * All ITG2AC Functions here
 * Mostly...Implemented
 *
 * Work Log!
 *
 * Work started 2/9/08, after 10 PM - 2:30 AM
 *  ProductName, Revision, Uptime, Crashlogs,
 *  Machine edits, done!
 *
 * Work, 2/10/08 7 PM - 9:30 PM
 *  Did work on GetNumMachineScores() ... That sucked
 *  Somewhat complete...Can't do IO Errors, an ITG-IO
 *  exclusive, it seems.
 *
 * Total Hours: ~6
 * 
 * this doesn't belong in LuaManager.cpp --infamouspat
 */
#include "LuaFunctions.h"
// Added by Matt1360
LuaFunction_NoArgs( GetProductName	, CString( PRODUCT_NAME_VER ) ); // Return the product's name from ProductInfo.h [ScreenArcadeDiagnostics]
LuaFunction_NoArgs( GetRevision	, GetRevision() ); // Return current Revision ( ProductInfo.h ) [ScreenArcadeDiagnostics]
LuaFunction_NoArgs( GetUptime		, SecondsToHHMMSS( RageTimer::GetTimeSinceStart() ) ); // Uptime calling [ScreenArcadeDiagnostics]
LuaFunction_NoArgs( GetIP		, GetIP() ); // Calling the IP [ScreenArcadeDiagnostics]
LuaFunction_NoArgs( GetNumCrashLogs	, GetNumCrashLogs() ); // Count the crashlogs [ScreenArcadeDiagnostics]
LuaFunction_NoArgs( GetNumMachineEdits	, GetNumMachineEdits() ); // Count the machine edits [ScreenArcadeDiagnostics]
LuaFunction_NoArgs( GetNumMachineScores, GetNumMachineScores() ); // Call the machine score count [ScreenArcadeDiagnostics]
// added by infamouspat
LuaFunction_NoArgs( GetSerialNumber, GetSerialNumber() ); // returns serial from page 9 on dongle
// added by Vyhd, if it matters that much :P
LuaFunction_NoArgs( GetNumIOErrors, ITGIO::m_iInputErrorCount ); // Call the number of I/O errors
LuaFunction_NoArgs( GetInputType, CString(g_sInputType) ); // grabs from RageInput's global variable
LuaFunction_NoArgs( HubIsConnected, HubIsConnected() );


/*
 * (c) 2008 BoXoRRoXoRs
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

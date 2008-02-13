/* RageFileDriverDirect - File driver for accessing a regular filesystem. */

#ifndef RAGE_FILE_DRIVER_DIRECT_H
#define RAGE_FILE_DRIVER_DIRECT_H

#include "RageFileDriver.h"

class RageFileDriverDirect: public RageFileDriver
{
public:
	RageFileDriverDirect( CString root );

	RageFileBasic *Open( const CString &path, int mode, int &err );
	bool Remove( const CString &sPath );
	bool Remount( const CString &sPath );

private:
	CString root;
};

/* This driver handles direct file access. */

class RageFileObjDirect: public RageFileObj
{
public:
	RageFileObjDirect( const CString &sPath, int iFD, int iMode );
	virtual ~RageFileObjDirect();
	virtual int ReadInternal( void *pBuffer, size_t iBytes );
	virtual int WriteInternal( const void *pBuffer, size_t iBytes );
	virtual int FlushInternal();
	virtual int SeekInternal( int offset );
	virtual RageFileBasic *Copy() const;
	virtual CString GetDisplayPath() const { return m_sPath; }
	virtual int GetFileSize() const;

private:
	int m_iFD;
	CString m_sPath; /* for Copy */

	CString m_sWriteBuf;
	
	bool FinalFlush();

	int m_iMode;
};

#endif

/*
 * Copyright (c) 2003-2004 Glenn Maynard
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


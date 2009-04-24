#include "global.h"
#include "RageUtil.h"
#include "ScreenManager.h"
#include "DebugTimer.h"
#include "InputHandler_PIUIO_Helper.h"

// keep these in local scope
uint32_t g_iSensors[4];
uint32_t g_iLights;

/* Do we have the r16 kernel hack available to use? */
bool MK6Helper::HasKernelPatch()
{
#ifdef LINUX
	if( IsAFile("/rootfs/stats/patch/modules/usbcore.ko") )
		return true;
#endif

	// never true on a non-Linux system
	return false;
}

void MK6Helper::Import( const uint32_t iSensorsIn[], const uint32_t iLightsIn )
{
	for( int i = 0; i < 4; i++ )
		g_iSensors[i] = iSensorsIn[i];
	g_iLights = iLightsIn;
}

/* Optimization opportunity: remove vector! :< */
CString MK6Helper::GetSensorDescription( const uint32_t iSensors[4], short iBit )
{
	static CString SensorNames[] = { "right", "left", "bottom", "top" };

	// TODO: cache the results of INPUTMAPPER->IsMapped(StyleInput)
	// to intelligently decide which inputs to report sensors for.
	CStringArray sensors;

	for( int i = 0; i < 4; i++ )
		if( IsBitSet(iSensors[i], iBit) )
			sensors.push_back( SensorNames[i] );

	/* HACK: if all sensors are reporting, then don't return anything.
	 * On PIUIO, all buttons always return all sensors except pads. */
	if( sensors.size() == 4 )
		return "";

	return join(", ", sensors);
}

void MK6Helper::DebugOutput( const DebugTimer &timer )
{
	if( !SCREENMAN )
		return;		// no point

	CString sDebugLine = "Input:\n";

	for( int i = 0; i < 4; i++ )
	{
		sDebugLine += "     " + BitsToString( g_iSensors[i] );
		sDebugLine += "\n";
	}

	sDebugLine += "Output:\n     " + BitsToString( g_iLights );
	sDebugLine += "\n" + ssprintf( "%i Hz, %f per update", timer.GetUpdateRate(), timer.GetUpdateTime() );

	SCREENMAN->SystemMessageNoAnimate( sDebugLine );
}

#include "LuaBinding.h"

// TRICKY: we need to build a 2D table and do it quickly,
// so we can't really use any built-in functions.
// To do: static function, namespace or Luna class.
int GetSensors( lua_State *L )
{
	lua_newtable( L );			// create the master sensor table
	
	// create subtables
	for( unsigned i = 0; i < 4; i++ )
	{
		lua_newtable( L );			// create a base table 
		lua_pushnumber( L, i+1 );	// set the index on the table
		lua_pushvalue( L, -2 );		// move the table reference to the top
		lua_settable( L, -4 );		// set the new table to the base table

		// create booleans for each sensor 1-32
		for( short s = 1; s <= 32; s++ )
		{
			// push the boolean value, then associate and pop it
			lua_pushboolean( L, (int)IsBitSet(g_iSensors[i],s) );
			lua_rawseti( L, -2, s );
		}

		lua_pop( L, 1 );	// pop the sub-table
	}

	// leave the subtable at the top of the stack
	return 1;
}

// since the above manually manipulates the LUA table,
// we need to manually register it with a hack. Go figure.
void MK6_Register( lua_State *L )
{
	lua_register( L, "MK6_GetSensors", GetSensors );
}

REGISTER_WITH_LUA_FUNCTION( MK6_Register );

/*
 * (c) 2008-2009 BoXoRRoXoRs
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
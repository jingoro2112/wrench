/*******************************************************************************
Copyright (c) 2023 Curt Hartung -- curt.hartung@gmail.com

MIT License

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*******************************************************************************/

#include "wrench.h"

#ifdef WRENCH_INCLUDE_DEBUG_CODE

//------------------------------------------------------------------------------
WrenchPacket::WrenchPacket( const WrenchDebugComm type, const uint32_t payloadSize )
{
	memset( (char*)this, 0, sizeof(WrenchPacket) );
	_type = type;
	setPayloadSize( payloadSize );
}

//------------------------------------------------------------------------------
WrenchPacket::WrenchPacket( const int32_t type, const uint32_t payloadSize )
{
	memset( (char*)this, 0, sizeof(WrenchPacket) );
	t = type;
	setPayloadSize( payloadSize );
}

//------------------------------------------------------------------------------
WrenchPacket* WrenchPacket::alloc( WrenchPacket const& base )
{
	WrenchPacket* packet = (WrenchPacket*)g_malloc( base.size );

	memcpy( (char*)packet, (char*)&base, sizeof(WrenchPacket) );
	return packet;
}

//------------------------------------------------------------------------------
WrenchPacket* WrenchPacket::alloc( const uint32_t type, const uint32_t payloadSize )
{
	WrenchPacket* packet = (WrenchPacket*)g_malloc( sizeof(WrenchPacket) + payloadSize );

	memset( (char*)packet, 0, sizeof(WrenchPacket) );

	packet->t = type;
	packet->setPayloadSize( payloadSize );

	return packet;
}

//------------------------------------------------------------------------------
uint32_t WrenchPacket::xlate()
{
	uint32_t s = size;

	size = wr_x32( size );
	t = wr_x32( t );
	param1 = wr_x32( param1 );
	param2 = wr_x32( param2 );

	return s;
}

//------------------------------------------------------------------------------
void wr_PacketClearFunc( WrenchPacket*& packet )
{
	g_free( packet );
}

#endif

#ifndef _PACKET_H
#define _PACKET_H
/*******************************************************************************
Copyright (c) 2023 Curt Hartung -- curt.hartung@gmail.com

MIT Licence

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

#ifdef WRENCH_INCLUDE_DEBUG_CODE

//------------------------------------------------------------------------------
enum WrenchDebugComm
{
	WRD_None = 0,
	WRD_Ok,
	WRD_Run,
	WRD_Load,

	WRD_DebugOut,

	WRD_RequestCallstack,
	WRD_RequestStackDump,
	WRD_RequestSymbolBlock,
	WRD_RequestSourceBlock,
	WRD_RequestSourceHash,
	WRD_RequestValue,
	WRD_RequestStepOver,
	WRD_RequestStepInto,
	WRD_RequestSetBreakpoint,
	WRD_RequestClearBreakpoint,

	WRD_ReplyCallstack,
	WRD_ReplyStackDump,
	WRD_ReplySymbolBlock,
	WRD_ReplySource,
	WRD_ReplySourceHash,
	WRD_ReplyValue,

	WRD_ReplyUnavailable,
	WRD_Err,
};

//------------------------------------------------------------------------------
struct WrenchPacket
{
	uint32_t size; // TOTAL size including any additional payload (always first)

	union
	{
		uint32_t t; // type this is
		WrenchDebugComm _type;
	};
	int32_t param1;
	int32_t param2;

	operator WrenchPacket* () { return this; }

	void setPayloadSize( uint32_t newsize ) { size = sizeof(WrenchPacket) + newsize; }
	uint32_t payloadSize() { return (size <= sizeof(WrenchPacket)) ? 0 : (size - sizeof(WrenchPacket)); }
	uint8_t* payload( uint32_t offset =0 ) { return (uint8_t*)((char *)this + sizeof(WrenchPacket)) + offset; }
	uint8_t operator[] ( const int offset ) { return *(uint8_t*)((char*)this + sizeof(WrenchPacket)) + offset; }
	uint8_t* data() { return (uint8_t*)this; }
	
	WrenchPacket() {}
	WrenchPacket( const int32_t type );
	static WrenchPacket* alloc( WrenchPacket const& base );
	static WrenchPacket* alloc( const uint32_t type, const uint32_t payloadSize =0, const uint8_t* payload =0 );
	uint32_t xlate();
};

//------------------------------------------------------------------------------
class WrenchPacketScoped
{
public:
	WrenchPacket* packet;

	operator WrenchPacket* () const { return packet; }
	operator bool() const { return packet != 0; }
	
	WrenchPacketScoped( WrenchPacket* manage =0 ) : packet(manage) {}
	WrenchPacketScoped( const uint32_t type ) : packet( WrenchPacket::alloc(type) ) {}
	WrenchPacketScoped( const uint32_t type, const int bytes, const uint8_t* data ) : packet( WrenchPacket::alloc(type, bytes, data) ) {}
	~WrenchPacketScoped() { g_free( packet ); }
};

#endif

#endif

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
#ifndef _WRENCH_DEBUG_H
#define _WRENCH_DEBUG_H
/*------------------------------------------------------------------------------*/

//------------------------------------------------------------------------------
enum WrenchDebugComm
{
	Run,
	Load,
	
	RequestStatus,
	RequestSymbolBlock,
	RequestSourceBlock,
	RequestSourceHash,

	ReplyStatus,
	ReplySymbolBlock,
	ReplySource,
	ReplySourceHash,
	
	ReplyUnavailable,
	Err,
	Halted,

};

//------------------------------------------------------------------------------
struct WrenchPacketGenericPayload
{
	int32_t hash;

	void xlate()
	{
		hash = wr_x32(hash);
	}
};

//------------------------------------------------------------------------------
struct WrenchPacket
{
	int32_t payloadSize;
	char type;
	char* payload;

	WrenchPacket() { memset(this, 0, sizeof(*this)); }
	~WrenchPacket() { clear(); }

	WrenchPacket( WrenchPacket& other )
	{
		*this = other;
		other.payload = 0;
	}

	WrenchPacket& operator=( WrenchPacket& other )
	{
		if ( this != &other )
		{
			*this = other;
			other.payload = 0;
		}

		return *this;
	}
	

	void clear() { free(payload); payload = 0; }
	
	void xlate() { payloadSize = wr_x32(payloadSize); }

	char* allocate( int size )
	{
		if ( payload )
		{
			free( payload );
		}
		return (payload = (char*)malloc(size));
	}
	
	WrenchPacketGenericPayload* genericPayload()
	{
		return (WrenchPacketGenericPayload *)allocate( sizeof(WrenchPacketGenericPayload) );
	}

private:
	WrenchPacket( const WrenchPacket& other );
	WrenchPacket& operator=( const WrenchPacket& other );
};

//------------------------------------------------------------------------------
enum WrenchDebugInfoType
{
	TypeMask =     0xC000,
	PayloadMask =  0x3FFF,
	
	FunctionCall = 0x0000, // size: 1 the next opcode is a function call, step into or over?
	LineNumber =   0x4000, // 14-bit line number
	Returned =     0x8000, // function has returned
	Reserved2 =    0xC000,
};

//------------------------------------------------------------------------------
struct WrenchSymbol
{
	char label[64];
};

//------------------------------------------------------------------------------
struct WrenchFunction
{
	char label[64];
	SimpleLL<WrenchSymbol> locals;
};

#endif


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
#ifndef _DEBUG_H
#define _DEBUG_H
/*------------------------------------------------------------------------------*/

void wr_stackDump( const WRValue* bottom, const WRValue* top, WRstr& out );

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

	void setPayloadSize( uint32_t newsize ) { size = sizeof(WrenchPacket) + newsize; }
	uint32_t payloadSize() { return (size <= sizeof(WrenchPacket)) ? 0 : (size - sizeof(WrenchPacket)); }
	uint8_t* payload( uint32_t offset =0 ) { return (uint8_t*)((char *)this + sizeof(WrenchPacket)) + offset; }
	uint8_t operator[] ( const int offset ) { return *(uint8_t*)((char*)this + sizeof(WrenchPacket)) + offset; }

	WrenchPacket() {}
	WrenchPacket( const WrenchDebugComm, const uint32_t payloadSize =0 );
	WrenchPacket( const int32_t type, const uint32_t payloadSize =0 );
	static WrenchPacket* alloc( WrenchPacket const& base );
	static WrenchPacket* alloc( const uint32_t type, const uint32_t payloadSize =0 );
	uint32_t xlate();
};

//------------------------------------------------------------------------------
// to allow the compiler to COMPILE debug code even if the VM does not
// execute it
enum WrenchDebugInfoType
{
	WRD_TypeMask =     0xC000,
	WRD_PayloadMask =  0x3FFF,
	WRD_GlobalStopFunction = 0x0F00,
	WRD_ExternalFunction = 0x0E00,

	WRD_FunctionCall = 0x0000, // payload is function index
	WRD_LineNumber =   0x4000, // 14-bit line number
	WRD_Return =       0x8000, // function has returned
	WRD_RESERVED =     0xC000,
};

#ifdef WRENCH_INCLUDE_DEBUG_CODE

void wr_PacketClearFunc( WrenchPacket*& packet );

//------------------------------------------------------------------------------
class WrenchPacketScoped
{
public:

	WrenchPacket* packet;

	operator WrenchPacket* () { return packet; }

	WrenchPacketScoped( WrenchPacket* manage ) : packet(manage) {}
	WrenchPacketScoped( const uint32_t type, const uint32_t size =0 ) : packet( WrenchPacket::alloc(type, size) ) {}
	~WrenchPacketScoped() { g_free( packet ); }
};

//------------------------------------------------------------------------------
class WRDebugClientInterfacePrivate
{
public:
	WRDebugClientInterfacePrivate();
	~WRDebugClientInterfacePrivate();

	void init();

	WrenchPacket* transmit( WrenchPacket* packet );

	WRDebugClientInterface* m_parent;
	
	char* m_sourceBlock;
	uint32_t m_sourceBlockLen;
	uint32_t m_sourceBlockHash;

	SimpleLL<WrenchPacket*>* m_packetQ;

	void getValues( SimpleLL<WrenchDebugValue>& values, const int level );
	void populateSymbols();
	
	bool m_symbolsLoaded;
	SimpleLL<WrenchSymbol>* m_globals;
	SimpleLL<WrenchFunction>* m_functions;

	bool m_callstackDirty;
	SimpleLL<WrenchCallStackEntry>* m_callStack;
	WRP_ProcState m_procState;

	WRDebugServerInterface* m_localServer;
	bool (*m_receiveFunction)( char* data, const int length );
	bool (*m_sendFunction)( const char* data, const int length );
	int (*m_dataAvailableFunction)();

	WRState* m_scratchState;
	WRContext* m_scratchContext;

	void trapRunOutput( WrenchPacket const& packet );
};

//------------------------------------------------------------------------------
class WRDebugServerInterfacePrivate
{
public:
	WRDebugServerInterfacePrivate( WRDebugServerInterface* parent );
	~WRDebugServerInterfacePrivate();

	bool codewordEncountered( const uint8_t* pc, uint16_t codeword, WRValue* stackTop );
	void clearBreakpoint( const int bp );
	WrenchPacket* processPacket( WrenchPacket* packet );

	int m_steppingOverReturnVector;
	int m_lineSteps;
	int m_stepOverDepth;
	bool m_stepOut;

	WrenchPacket m_lastPacket;

	SimpleLL<int>* m_lineBreaks;
	SimpleLL<WrenchPacket*>* m_packetQ;
	SimpleLL<WrenchCallStackEntry>* m_callStack;

	WRState* m_w;
	bool (*m_receiveFunction)( char* data, const int length );
	bool (*m_sendFunction)( const char* data, const int length );
	int (*m_dataAvailableFunction)();

	WRDebugServerInterface* m_parent;
	bool m_firstCall;
	int32_t m_stopOnLine;
	
	uint8_t* m_externalCodeBlock;
	uint32_t m_externalCodeBlockSize;	
	
	const uint8_t* m_embeddedSource;
	uint32_t m_embeddedSourceLen;
	uint32_t m_embeddedSourceHash;
	uint8_t m_compilerFlags;

	const uint8_t* m_symbolBlock;
	uint32_t m_symbolBlockLen;

	// for resuming a function call
	WRContext* m_context;
	const WRValue* m_argv;
	int m_argn;
	const unsigned char* m_pc;
	WRValue* m_frameBase;
	WRValue* m_stackTop;

	bool m_halted;
};

#endif

#endif

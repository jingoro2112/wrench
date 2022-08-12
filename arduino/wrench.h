#define WRENCH_COMBINED
/*******************************************************************************
Copyright (c) 2022 Curt Hartung -- curt.hartung@gmail.com

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
#ifndef _WRENCH_H
#define _WRENCH_H
/*------------------------------------------------------------------------------*/

/************************************************************************
Build wrench without it's compiler. this will minimize
the code size at the cost of being able to only load/run bytecode
*/
//#define WRENCH_WITHOUT_COMPILER
/***********************************************************************/

/************************************************************************
The interpreter normally loads partial blocks, with this defined it will
assume the loader function returns a single time with the entire
program. The returned block should be as large as practical, minimum of
16 bytes is reccomended
*/
#define SINGLE_COMPLETE_BYTECODE_LOAD
/***********************************************************************/

/************************************************************************
how many stack locations the wrench state will pre-allocate. The stack
cannot be grown so this needs to be enough for the life of the process.
this does NOT include global/array space which is allocated separately, just
function calls. Unless you are using piles of local data AND recursing
like crazy a modest size should be more than enough.

On most systems this will consume 8 bytes per stack entry
*/
#define DEFAULT_STACK_SIZE 64
/***********************************************************************/

/************************************************************************
There are a few hash tables here, and they are simple single-level
affairs. in order to do this they grow-on-collide
to the next sensible prime number. there is a limit built in at around
a 4000 entry table. if you have access to that kind of RAM then use
lua or some other mature full-featured scripting engine
if you REALLY want to use wrench then.. yay I guess.. uncomment this
and there will be no [practical] upper bound
*/
//#define UNLIMITED_HASH_SIZE
/***********************************************************************/

/************************************************************************
wr_valueToString() uses sprintf from stdio. If you don't want to
include or use that library then undefine this and that method will no
longer function
*/
#define SPRINTF_OPERATIONS
/***********************************************************************/

#define WRENCH_VERSION 101

#include <stdint.h>

//------------------------------------------------------------------------------
// in order to minimize text segment size, only a minimum of strings
// are embedded in the code. error messages are very verbose enums
enum WRError
{
	WR_ERR_None = 0,

	WR_ERR_compiler_not_loaded,
	WR_ERR_function_hash_signature_not_found,
	WR_ERR_unknown_opcode,
	WR_ERR_unexpected_EOF,
	WR_ERR_unexpected_token,
	WR_ERR_bad_expression,
	WR_ERR_bad_label,
	WR_ERR_statement_expected,
	WR_ERR_unterminated_string_literal,
	WR_ERR_newline_in_string_literal,
	WR_ERR_bad_string_escape_sequence,
	WR_ERR_tried_to_load_non_resolvable,
	WR_ERR_break_keyword_not_in_looping_structure,
	WR_ERR_continue_keyword_not_in_looping_structure,
	WR_ERR_expected_while,
	
	WR_ERR_execute_must_be_called_by_itself_first,
	WR_ERR_hash_table_size_exceeded,
	WR_ERR_wrench_function_not_found,
	WR_ERR_array_must_be_indexed,
	WR_ERR_context_not_found,

	WR_ERR_usr_data_template_already_exists,
	WR_ERR_usr_data_already_exists,
	WR_ERR_usr_data_template_not_found,
	WR_ERR_usr_data_refernce_not_found,

	WR_warning_enums_follow,
	
	WR_WARN_c_function_not_found,

	WR_WARN_run_gc,

};

struct WRState;
struct WRValue;

WRError wr_getLastError( WRState* w );

// the rest of the API is pretty simplified and straightforward

typedef unsigned int (*WR_LOAD_BLOCK_FUNC)( int offset, unsigned char** block, void* usr );

typedef void (*WR_C_CALLBACK)(WRState* s, WRValue* argv, int argn, WRValue& retVal, void* usr );

WRError wr_compile( const char* source, const int size, unsigned char** out, int* outLen );

int wr_run( WRState* w, const unsigned char* block, const int size );
int wr_run( WRState* w, WR_LOAD_BLOCK_FUNC, void* usr );
int wr_callFunction( WRState* w, const int contextId =0, const char* functionName =0, const WRValue* argv =0, const int argn =0 );

void wr_destroyContext( WRState* w, const int contextId );

void wr_makeInt( WRValue* val, int i );
void wr_makeFloat( WRValue* val, float f );
void wr_makeString( WRValue* val, const char* s );

void wr_makeArray( WRValue* val, const int len, const char* registerNameAs =0, WRValue* registerInUserData =0 );
void wr_makeCharArray( WRValue* val, const int len, const unsigned char* preAllocated =0, const char* registerNameAs =0, WRValue* registerInUserData =0 );
void wr_makeIntArray( WRValue* val, const int len, const int* preAllocated =0, const char* registerNameAs =0, WRValue* registerInUserData =0 );
void wr_makeFloatArray(WRValue* val, const int len, const float* preAllocated =0, const char* registerNameAs =0, WRValue* registerInUserData =0 );

void wr_makeUserData( WRValue* val, void* usr =0 );
void wr_destroyValue( WRValue* val );

struct WRUserData;
void wr_registerUserValue( WRValue* userData, const char* key, WRValue* value );
WRValue* wr_getUserValue( WRValue* userData, const char* key );
void* wr_getUserPointer( WRValue* userData );

WRState* wr_newState();
void wr_destroyState( WRState* w );

int wr_registerFunction( WRState* w, const char* name, WR_C_CALLBACK function, void* usr =0 );

char* wr_valueToString( WRValue const& value, char* string );
int wr_valueToInt( WRValue const& value );
float wr_valueToFloat( WRValue const& value );

struct WRUserData;
class WRStaticValueArray;

//------------------------------------------------------------------------------
enum WRValueType
{
	WR_INT =   0x00,
	WR_REF =   0x01,
	WR_FLOAT = 0x02,
	WR_USR =   0x03,
	WR_ARRAY = 0x04,
};

//------------------------------------------------------------------------------
struct WRValue
{
	union
	{
		int32_t i;
		float f;
		void* p;
		WRValue* r;
		WRStaticValueArray* va;
		WRUserData* u;
	};

	union
	{
		uint8_t type;
		uint32_t arrayElement;
	};
			
	void clear() { p = 0; arrayElement = WR_INT; }

	WRValue* asValueArray( int* len =0 );
	unsigned char* asCharArray( int* len =0 );
	int* asIntArray( int* len =0 );
	float* asFloatArray( int* len =0 );

};

//--------------------- STD functions
extern int32_t wr_Seed;
int32_t wr_rnd( int32_t range );


#ifndef WRENCH_COMBINED
#include "utils.h"
#include "vm.h"
#include "opcode.h"
#include "str.h"
#include "cc.h"
#endif

#endif


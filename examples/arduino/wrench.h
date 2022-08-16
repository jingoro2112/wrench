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
For efficiency The interpreter expects a pointer to a single contiguous
block of bytecode, and it will never call the loader again.

IF YOU WANT TO DO PARTIAL LOADS: You must comment out the following!

This will compile in the bounds-checking and call the loader whenever
it detects execution space outside of the current block. The loader
must return a pointer to the requested offset in the bytecode and the
size of the mapped block. return AT LEAST 16 BYTES or "bad things"
happen
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

#define WRENCH_VERSION 102

#include <stdint.h>
struct WRState;
struct WRValue;

/***************************************************************/
/**************************************************************/
//                       State Management

// create/destroy a WRState object that can run multiple contexts/threads
WRState* wr_newState( int stackSize =DEFAULT_STACK_SIZE );
void wr_destroyState( WRState* w );



/***************************************************************/
/**************************************************************/
//                        Running Code

//     function to load, run and call back to the interpreter
// Callback to load bytecode, see note above regarding :
// SINGLE_COMPLETE_BYTECODE_LOAD
// by default this will be called one time and expects the entire
// bytecode to be mapped to the 'block' pointer. wr_run() has a
// built-in simple version for ease of use.
//
// offset: where in the bytecode wrench needs loaded (will be zero
//         unless SINGLE_COMPLETE_BYTECODE_LOAD is defined)
// block:  pointer to pointer where the lock is mapped, this can be ROM
// usr:    opaque pointer passed to each load call (optional)
typedef unsigned int (*WR_LOAD_BLOCK_FUNC)( int offset, const unsigned char** block, void* usr );

// compile a block of code and return a new'ed block of bytecode ready
// for wr_run()
// return value is a WRError
int wr_compile( const char* source, const int size, unsigned char** out, int* outLen );

// run a block of code, either as a single block or with a loader that
// will be called back. PLEASE SEE NOTES ABOVE REGARDING: SINGLE_COMPLETE_BYTECODE_LOAD

// w:      state
// block/size: location of bytecode
// loader/usr: callback for loader and a void* pointer that will be
//             passed to the loader opaquely (optional)
// RETURNS: a contextId which must be passed to callFunction when
//          multiple scripts are loaded into a single state
//          'global' state is NOT SHARED between contexts
int wr_run( WRState* w, const unsigned char* block, const int size );
int wr_run( WRState* w, WR_LOAD_BLOCK_FUNC loader, void* usr =0 );


// after has run, this allows any function contained inside it to be
// called with the given arguments, returning a single value
// contextId:    the context in which the function was loaded
// functionName: plaintext name of the function to be called
// argv:         array of WRValues to call the function with (optional)
// argn:         how many arguments argv contains (optional, but if
//               zero then argv will be ignored)
int wr_callFunction( WRState* w, const int contextId, const char* functionName, const WRValue* argv =0, const int argn =0 );
int wr_callFunction( WRState* w, const int contextId, const int32_t hash, const WRValue* argv =0, const int argn =0 );

// use this to destroy a context you no longer need and free up all the
// memeory it was using
void wr_destroyContext( WRState* w, const int contextId );


/***************************************************************/
/***************************************************************/
//                Calling back to c++ from wrench                         

// register a function inside a state that can be called (by ALL
// contexts)
// callback will contain:
// w:             state it was called back from
// argv:          pointer to number of arguments function was called
//                with (may be null!)
// argn:          how many arguments it was called with
// retVal:        this value will be passed back, default zero
// usr:           opaque pointer function was registered with
typedef void (*WR_C_CALLBACK)(WRState* w, const WRValue* argv, const int argn, WRValue& retVal, void* usr );

// IMPORTANT: The values passed may be references (keepin' it real) so
// always use the getters inside the WRValue class:

// int: WRValue.asInt();
// float: WRValue.asFloat();
// string: WRValue.asString();


// w:        state to register with (will be available to all contexts)
// name:     name of the function
// function: callback (see typdef above)
// usr:      opaque pointer that will be passed to the callback (optional)
int wr_registerFunction( WRState* w, const char* name, WR_C_CALLBACK function, void* usr =0 );



/***************************************************************/
/**************************************************************/
//              wrench types and how to make them!

// The WRValue is the basic data unit inside wrench, it can be a
// float/int/array whatever... think of it as a thneed. Its 8 bytes on
// a 32 bit system and you can create one on the stack no problemo, no
// heap RAM will be harmed

// load a value up and make it ready for calling a function
void wr_makeInt( WRValue* val, int i );
void wr_makeFloat( WRValue* val, float f );

// So you want to access memory directly? This is how you do that. with
// a custom User type. creating a UserData is like creating a struct,
// its a container that can be accessed by wrench with dot notation:
//
// someUserData.value1      <- simple value
// someUserData.value2[20]  <- array

// STEP 1:    create the user data. This will be the 'top level' object
// the other members are loaded into. It can be passed to any function as
// a parameter
void wr_makeUserData( WRValue* value );

// STEP 2:    load it up!
//
// add the given value to the userData, which will be accessible by
// 'name' as in:  theObject.name 

// userData:  User data object created with wr_makeUserData() above
// name:      what the array will be called inside wrench
// data/len:  pointer to a user-managed memory space which will be
//            accessible via .name inside wrench
// value:     pointer to a plain user-mananged WRValue
void wr_addUserValue( WRValue* userData, const char* name, WRValue* value );
void wr_addUserCharArray( WRValue* userData, const char* name, const unsigned char* data, const int len );
void wr_addUserIntArray( WRValue* userData, const char* name, const int* data, const int len );
void wr_addUserFloatArray( WRValue* userData, const char* name, const float* data, const int len );

// STEP 3:    use it!
// the userData object can now be used in any call into wrench and be
// accessible as shown above. All structures will be freed when WRValue
// falls out of scope, alternately they can be manually freed by
// calling the clear() method from WRValue



// NOTE: for efficiency there is no constructor on WRValue despite it
// having pointers! This is because 99.99% of the time one is
// created it's immediately loaded with values, so a constructor would
// be wasted cycles in all cases. But this also means it's your
// responsibility to call "wr_makeXXX or at LEAST .init() after
// creating one!


/***************************************************************/
/***************************************************************/
//                          Errors

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
};

// after wrench executes it may have set an error code, this is how to
// retreive it. This sytems is coarse at the moment. Re-entering the
// interpreter clears the last error
WRError wr_getLastError( WRState* w );


/******************************************************************/
//                    "standard" functions

// just some simple stuff I needed to make my project work

extern int32_t wr_Seed; // make sure you set this to something random on startup if you are going to use rnd

// return a number between 0 and range such that:
// wr_rand( 100 );  -- will return 0 to 99
// wr_rand( 2 );  -- will return 0 or 1
//
// in other words: under the hood it's "rescramble with seed, return (seed % range)"
int32_t wr_rand( int32_t range ); 

// hashing function used inside wrench, it's a stripped down murmer,
// not academically fantastic but very good and very fast
uint32_t wr_hash( const void* dat, const int len );
uint32_t wr_hashStr( const char* dat );



// inside-baseball time. This has to be here so stuff that includes
// "wrench.h" can create a WRValue. nothing to see here.. smile and
// wave...

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
class WRUserData;
class WRStaticValueArray;
struct WRValue
{
	int asInt() const { return type == WR_REF ? r->asInt() : type == WR_FLOAT ? (int)f : i; }
	float asFloat() const { return type == WR_REF ? r->asFloat() : type == WR_FLOAT ? f : (float)i; }

	// string: must point to a buffer long enough to contain the string in
	// the case value is an array of chars. the pointer will be passed back
	char* asString( char* string ) const;


/*
	// argv
	int wr_valueToInt( WRValue const& value );
	float wr_valueToFloat( WRValue const& value );
	// string: must point to a buffer long enough to contain the string in
	// the case value is an array of chars. the pointer will be passed back
	char* wr_valueToString( WRValue const& value, char* string );
*/
	
	
	void init() { p = 0; arrayElement = 0; } // call upon first create or when you're sure no memory is hanging from one
	void free(); // treat this object as if it owns any memory allocated on u or va pointer

	// easy access to the void* dynamically typed
	WRValue* asValueArray( int* len =0 );
	unsigned char* asCharArray( int* len =0 );
	int* asIntArray( int* len =0 );
	float* asFloatArray( int* len =0 );

	~WRValue() { free(); }

	//-----------------
	union // first 4 bytes
	{
		int32_t i;
		float f;
		const void* p;
		WRValue* r;
		WRStaticValueArray* va;
		WRUserData* u;
	};

	union // next 4 bytes
	{
		uint8_t type; // carries the type
		uint32_t arrayElement; // if this is a reference to an array, this carries the currently indexed item in the top 24 bits
	};
};


#ifndef WRENCH_COMBINED
#include "utils.h"
#include "vm.h"
#include "opcode.h"
#include "str.h"
#include "cc.h"
#endif

#endif


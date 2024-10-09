#define WRENCH_COMBINED
#ifndef _WRENCH_H
#define _WRENCH_H
/*******************************************************************************
Copyright (c) 2024 Curt Hartung -- curt.hartung@gmail.com

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

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#define WRENCH_VERSION_MAJOR 6
#define WRENCH_VERSION_MINOR 0
#define WRENCH_VERSION_BUILD 5
struct WRState;

/************************************************************************
The compiler is not particularly memory or space efficient, for
small embedded systems it is strongly reccommeded (nay, required) that
only bytecode be executed. This flag explicitly removes the compiler
(although just not calling it should allow a smart linker to remove it
anyway)
*/
//#define WRENCH_WITHOUT_COMPILER


/***********************************************************************
Cause the interpreter to compile into the smallest program size
possible at the cost of some speed. This loss is from the removal of
unrolled loops and inlined functionality, also making use of some goto.

WRENCH_REALLY_COMPACT reduces size further by removing the jumptable
interpreter in favor of a giant switch(). This savings comes at the cost
of more speed so only use it if you need to.
*/
//#define WRENCH_COMPACT           // saves a lot, costs some speed
//#define WRENCH_REALLY_COMPACT    // saves a little more, costs more speed


/***********************************************************************
Some architectures (mostly embedded) really don't like reading more than
8 bits on an unaligned memory location and will bus fault! If your architecture
allows it (x86/64, Mac, most *nix etc..) this is a decent optimization
*/
//#define WRENCH_UNALIGNED_READS


/***********************************************************************
wrench automatically detects endian-ness and defines these three
macros for reading data from the code stream, if you have special
memory requirements, as some embedded systems do, you may re-define
them here, but they MUST match the endian-ness of the target
architecture, see vm.h for the current definitions
*/
//#define READ_32_FROM_PC( P ) 
//#define READ_16_FROM_PC( P )
//#define READ_8_FROM_PC( P )


/***********************************************************************
 **** EXPERIMENTAL **** only very basic support is in place */
//#define WRENCH_INCLUDE_DEBUG_CODE

// the debugger works over a serial link, define the architecure this
// side is being targetted for
//#define WRENCH_WIN32_SERIAL
//#define WRENCH_ARDUINO_SERIAL
//#define WRENCH_LINUX_SERIAL

/***********************************************************************/


/************************************************************************
The stack is statically allocated but only used for function
calls and locals, NOT structs/arrays/hashes which are malloc'ed. Unless your
applications uses lots of recursion+local variables, a modest size should
be more than enough.
To really reduce RAM footprint this can be lowered considerably
depending on usage. (consumes 8 bytes per stack entry)
*/
#define WRENCH_DEFAULT_STACK_SIZE 64
// this costs a small bit of overhead whenever the stack is used, for
// most applications it is not necessary, but will protect against
// things like infinite recursion
//#define WRENCH_PROTECT_STACK_FROM_OVERFLOW


/************************************************************************
With this defined the VM gives "slice" instructions before forcing a
yield, to prevent infinite loops, this adds a small check to each
instruction
*/
//#define WRENCH_TIME_SLICES

#ifdef WRENCH_TIME_SLICES
// how many instructions each call to the VM executes before yielding,
// set to '0' for unlimited (default)
void wr_setInstructionsPerSlice( WRState* w, const int instructions );
void wr_forceYield( WRState* w );  // for the VM to yield right NOW, (called from a different thread)
#endif

/************************************************************************
if you WANT full sprintf support for floats (%f/%g) this adds it at
the cost of using the standard c library for it (stdlib.h), which can incur a
decent code-size penalty. Without this %f/%g is ignored.
*/
//#define WRENCH_FLOAT_SPRINTF


/************************************************************************
File operations: define ONE of these. If you use "Custom" then you
need to link in a source file with the functions defined in:

  /discrete_src/std_io_defs.h

examples are in

  /discrete_src/std_io_linux.cpp
  /discrete_src/std_io_win32.cpp
  /discrete_src/std_io_spiffs.cpp
  /discrete_src/std_io_littlefs.cpp

*/
//#define WRENCH_WIN32_FILE_IO
//#define WRENCH_LINUX_FILE_IO
//#define WRENCH_SPIFFS_FILE_IO
//#define WRENCH_LITTLEFS_FILE_IO


/************************************************************************
basic TCP/IP socket operations, define the architecture you are
compiling for here, then see wr_loadTCPLib(...)
*/
//#define WRENCH_WIN32_TCP
//#define WRENCH_LINUX_TCP


/************************************************************************
Wrench protects itself against divide-by-zero, but allows the result to
be '0'. Trapping the error requires a tiny bit of overhead(ie- every
result has to be checked to see if it was a divide by zero) so by
default wrench does not bother in favor of speed.
Define this to received the fatal error:  WR_ERR_division_by_zero
*/
//#define WRENCH_TRAP_DIVISION_BY_ZERO

/************************************************************************
for systems that need to know if they have run out of memory.

WARNING: This imposes a small if() check on EVERY INSTRUCTION so the
malloc failure is detected on the instruction it happens and guarantees
graceful exit
*/
//#define WRENCH_HANDLE_MALLOC_FAIL

// by default wrench uses malloc/free but if you want to use your own
// allocator it can be set up here
// NOTE: this becomes global for all wrench code!
typedef void* (*WR_ALLOC)(size_t size);
typedef void (*WR_FREE)(void* ptr);
void wr_setGlobalAllocator( WR_ALLOC wralloc, WR_FREE wrfree );
extern WR_ALLOC g_malloc;
extern WR_FREE g_free;
#ifdef WRENCH_HANDLE_MALLOC_FAIL
extern bool g_mallocFailed; // used as an internal global flag for when a malloc came back null
#endif

//------------------------------------------------------------------------------

struct WRValue;
struct WRContext;
struct WRFunction;

//------------------------------------------------------------------------------
// to minimize text segment size, only a minimum of strings
// are embedded in the code. error messages are very verbose enums
// also provided as an array of strings which are _not_ referenced
// by the VM or Compiler, so normally will not be linked in.
extern const char* c_errStrings[];
enum WRError
{
	WR_ERR_None = 0,

	WR_ERR_compiler_not_loaded,
	WR_ERR_function_not_found,
	WR_ERR_lib_function_not_found,
	WR_ERR_hash_not_found,
	WR_ERR_library_constant_not_loaded,
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
	WR_ERR_compiler_panic,
	WR_ERR_constant_redefined,
	WR_ERR_struct_in_struct,
	WR_ERR_var_not_seen_before_label,
	WR_ERR_unexpected_export_keyword,
	WR_ERR_new_assign_by_label_or_offset_not_both,
	WR_ERR_struct_not_exported,

	WR_ERR_run_must_be_called_by_itself_first,
	WR_ERR_hash_table_size_exceeded,
	WR_ERR_hash_table_invalid_key,
	WR_ERR_wrench_function_not_found,
	WR_ERR_array_must_be_indexed,
	WR_ERR_context_not_found,
	WR_ERR_context_not_yielded,
	WR_ERR_cannot_call_function_context_yielded,

	WR_ERR_hash_declaration_in_array,
	WR_ERR_array_declaration_in_hash,

	WR_ERR_stack_overflow,

	WR_ERR_bad_goto_label,
	WR_ERR_bad_goto_location,
	WR_ERR_goto_target_not_found,

	WR_ERR_switch_with_no_cases,
	WR_ERR_switch_case_or_default_expected,
	WR_ERR_switch_construction_error,
	WR_ERR_switch_bad_case_hash,
	WR_ERR_switch_duplicate_case,

	WR_ERR_bad_bytecode_CRC,

	WR_ERR_execute_function_zero_called_more_than_once,

	WR_ERR_malloc_failed,

	WR_ERR_USER_err_out_of_range,

	WR_ERR_division_by_zero,
	
	WR_warning_enums_follow,

	WR_USER = 100, // user-defined below here

	
	WR_ERR_LAST = 255,
};

/***************************************************************/
/***************************************************************/
//                       State Management

// create/destroy a WRState object that can run multiple contexts/threads
WRState* wr_newState( int stackSize =WRENCH_DEFAULT_STACK_SIZE );
void wr_destroyState( WRState* w );

// allocate/free memory with the same allocator wrench is using, this
// is particularly important for the "takeOwnership" flag below
void* wr_malloc( size_t size );
void wr_free( void* ptr );

// hashing function used inside wrench, it's a stripped down murmer,
// not techcnically the "best" but very good, very fast and very compact
uint32_t wr_hash( const void* dat, const int len, uint32_t serial=0 );
uint32_t wr_hashStr( const char* dat, uint32_t serial=0 );

/***************************************************************/
/**************************************************************/
//                        Running Code

// compile source code and return a wr_malloc'ed block of bytecode ready
// for wr_run(), this memory must be manually wr_free'ed
// return value is a WRError
// optionally an "errMsg" buffer can be passed which will output a
//     human-readable string of what went wrong and where.
// compilerOptionFlags : OR'ed mask of of WrenchCompilerOptionFlags:
enum WrenchCompilerFlags
{
	WR_INCLUDE_GLOBALS   = 1<<0, // include all globals NOTE: wr_getGlobalRef(...)
								 // will not function without this
	
	WR_EMBED_DEBUG_CODE  = 1<<1, // include per-instruction NOTE: WrenchDebugInterface
								 // will not function without this
	
	WR_EMBED_SOURCE_CODE = 1<<2, // include a copy of the source code
	
	WR_NON_STRICT_VAR    = 1<<3, // require 'var' to declare a variable (disabled by default)
};

WRError wr_compile( const char* source,
					const int size,
					unsigned char** out,
					int* outLen,
					char* errMsg =0,
					const uint8_t compilerOptionFlags = WR_INCLUDE_GLOBALS );

// disassemble the bytecode and output humanm readable
void wr_disassemble( const uint8_t* bytecode, const unsigned int len, char** out, unsigned int* outLen =0 );

// w:          state (see wr_newState)
// block:      location of bytecode
// blockSize:  number of bytes in the block
// takeOwnership: assume 'block' was allocated with wr_malloc(), it
//                will be freed with  wr_free()

// RETURNS:    an allocated WRContext
//             NOTE: This Context is automatically destroyed when
//             wr_destroyState() is called, but can be manually destroyed
//             with wr_destroyContext(...) (see below)
WRContext* wr_run( WRState* w, const unsigned char* block, const int blockSize, bool takeOwnership =false );

// macro for automatically freeing the WRContext
#define wr_runOnce( w, block, blockSize ) wr_destroyContext( wr_run((w),(block),(blockSize)) )

// Run the source code as an atomic operation
bool wr_runCommand( WRState* w, const char* sourceCode, const int size =-1 );

// Add more code to the context, the new code cannot access the
// existing block, but all of its functions will become available to
// the parent.
// takeOwnership: assume 'block' was allocated with wr_malloc(), it
//                will be freed with  wr_free()
WRContext* wr_import( WRContext* context, const unsigned char* block, const int blockSize, bool takeOwnership =false );

// After wr_run(...) or wr_callFunction(...) has run, there is always a
// return value (default 0) this function fetches it
WRValue* wr_returnValueFromLastCall( WRContext* context );

// wr_run actually calls these two functions back-to back. If you want
// to separate the process you can call them on your own.
// takeOwnership: assume 'block' was allocated with wr_malloc(), it
//                will be freed with  wr_free()
WRContext* wr_newContext( WRState* w, const unsigned char* block, const int blockSize, bool takeOwnership =false );
WRValue* wr_executeContext( WRContext* context );

// after wr_run() this allows any function in the script to be
// called with the given arguments, returning a single value
//
// contexd:      the context in which the function was loaded
// functionName: plaintext name of the function to be called
// argv:         array of WRValues to call the function with (optional)
// argn:         how many arguments argv contains (optional, but if
//               zero then argv will be ignored)
// RETURNS:      WRValue returned by the function/program, or NULL for
//               error (see w->err for the code)

// If the context is yielded, callFunction(...) will always continue
// where the yielded function left off and ignore any parameters

WRValue* wr_callFunction( WRContext* context, const char* functionName, const WRValue* argv =0, const int argn =0 );

// exactly the same as the above but the hash is supplied directly to
// save compute time, us wr_hashStr(...) to obtain the hash of the
// functionName
WRValue* wr_callFunction( WRContext* context, const int32_t hash, const WRValue* argv =0, const int argn =0 );

// The raw function pointer can be pre-loaded with wr_getFunction() and
// and then called with an absolute minimum of overhead
WRValue* wr_callFunction( WRContext* context, WRFunction* function, const WRValue* argv =0, const int argn =0 );

// Continue a yielded context.
// if context is NOT yielded this will return null will err
// "WR_ERR_context_not_yielded"
WRValue* wr_continue( WRContext* context );

// If the function is yielded, this returns true and provides the
// argument list that was passed to yield(...), if requested
// returns false if this context is not yielded
bool wr_getYieldInfo( WRContext* context, int* args =0, WRValue** firstArg =0, WRValue** returnValue =0 );

// after wrench executes it may have set an error code, this is how to
// retreive it. This sytems is coarse at the moment. Re-entering the
// interpreter clears the last error
WRError wr_getLastError( WRState* w );

// once wr_run() is called the returned context object can be used to
// pre-fetch a function pointer. This reduces the overhead of calling
// that function to almost nothing.
WRFunction* wr_getFunction( WRContext* context, const char* functionName );

// want direct access to a global? okay then, if the code was compiled
// with symbols (see compile(...) suite) then this will give you a
// reference to it
WRValue* wr_getGlobalRef( WRContext* context, const char* label );

// Destroy a context you no longer need and free up all the memory it
// was using
// NOTE: all contexts ARE AUTOMATICALLY FREED when wr_destroyState(...)
//       is called, it it NOT necessary to call this on each context
void wr_destroyContext( WRContext* context );

// how many bytes of memory must be allocated before the gc will run, default
// set here, can be adjusted at runtime with the
// wr_setAllocatedMemoryGCHint()
#define WRENCH_DEFAULT_ALLOCATED_MEMORY_GC_HINT 4000
void wr_setAllocatedMemoryGCHint( WRContext* context, const uint16_t bytes );

/***************************************************************/
/***************************************************************/
//                 Callbacks from wrench                         

// register a function inside a state that can be called (by ALL
// contexts)
// callback will contain:
// c:             context called from
// argv:          pointer to arguments function was called
//                with (may be null!)
// argn:          how many arguments it was called with
// retVal:        this value will be passed back, default: integer zero
// usr:           opaque pointer function was registered with
typedef void (*WR_C_CALLBACK)(WRContext* c, const WRValue* argv, const int argn, WRValue& retVal, void* usr );

// IMPORTANT: The values passed may be references (keepin' it real) so
// always use the getters inside the WRValue class:

// int:    .asInt();
// float:  .asFloat();
// binary: .array( unsigned int* size, char* type );
// string: .c_str( unsigned int* len );

// tests:
// .isFloat() fast check if this is a float
// .isInt() fast check if this is an int
// .isString() fast check if this is a string


// this will do its pest to represent the value as a string to the
// supplied buffer, len is maximum size allowed
// string: .asString(char* string, size_t len )

// w:        state to register with (will be available to all contexts)
// name:     name of the function
// function: callback (see typdef above)
// usr:      opaque pointer that will be passed to the callback (optional)
void wr_registerFunction( WRState* w, const char* name, WR_C_CALLBACK function, void* usr =0 );

// serialize WRValues to and from binary
// NOTE: on success, wr_serialize returns a malloc()'d buffer that must be freed!
bool wr_serialize( char** buf, int* len, const WRValue& value ); 
bool wr_deserialize( WRContext* context, WRValue& value, const char* buf, const int len );

/***************************************************************/
/***************************************************************/
//                   wrench Library calls

// Wrench allows libraries to be loaded in the form of <name>::<func>
// these calls impose no overhead if not used

// library calls do assume a deep knowledge of the internal workings of
// wrench so the callback is an absolute minimum of info:
// stackTop: current top of the stack, arguments have been pushed here,
//           and any return value must be placed here, not pushed! see
//           one of the many library examples
// argn:     how many arguments this function was called with. no
//           guarantees are made about matching a function signature,
//           library calls validate if they care.
// stackTop         : [return value]
// stackTop - 1     : [argN ]
// stackTop - 2     : [argN-1]
// ...
// stackTop - (N-1) : [arg2]
// stackTop - N     : [arg1]
typedef void (*WR_LIB_CALLBACK)( WRValue* stackTop, const int argn, WRContext* context );

// w:         state to register with (will be available to all contexts)
// signature: library signature must be in the form of <lib>::<name>
//            ex: "math::cos"
// function:  callback (see typdef above)
void wr_registerLibraryFunction( WRState* w, const char* signature, WR_LIB_CALLBACK function );
void wr_registerLibraryConstant( WRState* w, const char* signature, const WRValue& value );

// you can write your own libraries (look at std.cpp for all the
// standard ones) but several have been provided:

void wr_loadAllLibs( WRState* w ); // load all libraries in one call

void wr_loadSysLib( WRState* w ); // system/internal functions
void wr_loadMathLib( WRState* w ); // provides most of the calls in math.h
void wr_loadStdLib( WRState* w ); // standard functions like sprintf/rand/
void wr_loadIOLib( WRState* w ); // IO funcs (time/file/io)
void wr_loadStringLib( WRState* w ); // string functions
void wr_loadMessageLib( WRState* w ); // messaging between contexts
void wr_loadSerializeLib( WRState* w ); // serialize WRValues to and from binary
void wr_loadDebugLib( WRState* w ); // debuger-interact functions
void wr_loadTCPLib( WRState* w ); // TCP/IP functions

// arduino-specific functions, be sure to add arduino_lib.cpp to your
// sketch. much thanks to Koepel for contributing
void wr_loadAllArduinoLibs( WRState* w );

void wr_loadArduinoSTDLib( WRState* w ); 
void wr_loadArduinoIOLib( WRState* w ); 
void wr_loadArduinoLCDLib( WRState* w ); 

/***************************************************************/
/***************************************************************/
//             wrench types and how to make them!

// The WRValue is the basic data unit inside wrench, it can be a
// float/int/array whatever... think of it as a thneed. It's 8 bytes on
// a 32 bit system and you can create one on the stack no problemo, no
// heap will be harmed

// load a value up and make it ready for calling a function
WRValue& wr_makeInt( WRValue* val, int i );
WRValue& wr_makeFloat( WRValue* val, float f );

// a string has to exist in a context so it can be worked with
WRValue& wr_makeString( WRContext* context, WRValue* val, const char* data, const int len =0 );

// turning a value into a container allocates a hash table which must
// be released with destroy!
void wr_makeContainer( WRValue* val, const uint16_t sizeHint =0 );
void wr_destroyContainer( WRValue* val );

// NOTE: value memory is managed by called and must remain valid for
// duration of the container!!
void wr_addValueToContainer( WRValue* container, const char* name, WRValue* value );
void wr_addArrayToContainer( WRValue* container, const char* name, char* array, const uint32_t size );

// memory managed by the container, fire and forget:
void wr_addFloatToContainer( WRValue* container, const char* name, const float f );
void wr_addIntToContainer( WRValue* container, const char* name, const int i );

/******************************************************************/
//                    "standard" functions

extern int32_t wr_Seed;

// inside-baseball time. This has to be here so stuff that includes
// "wrench.h" can create a WRValue.

/*

 wrench values have a 32-bit type vector
 
type bits: XXXX  XXXX  ____  ____  ____  ____  __TT
                                        [8-bit type]
           [             32-bit xtype              ]

the bottom two bits 'T' represent the main type:
int       00 -- integer, ui / i are the value
float     01 -- float, f is the value
reference 10 -- reference, *r is a pointer to the real value
extended  11 -- extended type, see upper bits.

This is so operation tables can be constructed 4x4 for very quick
resolution of how to add/subtract/AND/etc... two values without a
time-consuming if-else-tree. single bit shift + OR and we're there:

            (typeA << 2) | typeB

Always results in a value from 0-15 which can index into a function
table:

    FunctionTypedef wr_func[16] = 
    {
      do_I_I,  do_I_F,  do_I_R,  do_I_E,
      do_F_I,  do_F_F,  do_F_R,  do_F_E,
      do_R_I,  do_R_F,  do_R_R,  do_R_E,
      do_E_I,  do_E_F,  do_E_R,  do_E_E,
    };

such that
wr_func[ (typeA << 2) | typeB ]( look ma, no ifs! );

In the case of "extended" types these are the ones that take extra
logic: arrays and hashes.

The "extended" types are:

(x is "don't care")

0x20xxxxxx  raw array: r->c points to a flat array, and
                       r->p2 holds the size of the array in the
                              middle 21 bits:

                              0x1FFFFF00
                              
                              such that shifting them >>8 yields
                              the actual array size
                       This is done so the value is never garbage
                       collected, or matched to the wrong type, but
                       does limit the size of an array to 2megabytes

0x40xxxxxx  debug break: stop execution and do debugging work

0x60xxxxxx  iterator: p2 holds the element we are on
                        va holds the array to be iterated

0x80xxxxxx  reference array: refers to a particular element of an
                             array.

                             r-> points to the array we are holding an
                             element of

                             p2 holds the element in the middle 21
                             bits:

							 0x1FFFFF00
                             
							 such that shifting them >>8 yields
                             the actual element
                             
						     This is done so the value is never garbage
							 collected, or matched to the wrong type, but
							 does limit the size of an array to 2megabytes
                              
0xA0xxxxxx  array/utility hash:
                        va-> holds a pointer to the actual array object (gc_object)
                        which has been allocated and is subject to
                        garbage collection

                        there are 4 kinds of "array" objects:

						SV_VALUE            0x01 array of WRValues
						SV_CHAR             0x02 array of chars (this
                         						 is how strings are represented)
						SV_VOID_HASH_TABLE  0x04 hash table of void*
						                         (do not gc)
						SV_HASH_TABLE       0x03 hash table of WRValues
						                         (DO descend for gc)

0xC0xxxxxx  struct: This value is a constructed "struct" object with a
                   hash table of values
                   
                   va-> refers to the container of initialized objects
                  

0xE0xxxxxx  hash table: hash table of WRValues

                   va-> refers to the hash table object

*/


//------------------------------------------------------------------------------
#if __cplusplus <= 199711L
enum WRValueType
#else
enum WRValueType : uint8_t
#endif
{
	WR_INT =   0x00,
	WR_FLOAT = 0x01,
	WR_REF =   0x02,
	WR_EX =    0x03,
};

//------------------------------------------------------------------------------
#if __cplusplus <= 199711L
enum WRExType
#else
enum WRExType : uint8_t
#endif
{
	WR_EX_INVALID    = 0x00,  // 0000

	WR_EX_RAW_ARRAY  = 0x20,  // 0010

	// EX types have one of the upper two bits set
	WR_EX_DEBUG_BREAK= 0x40,  // 0100
	
	WR_EX_ITERATOR	   = 0x60,  // 0110
	WR_EX_ARRAY_MEMBER = 0x80,  // 1000
	WR_EX_ARRAY        = 0xA0,  // 1010

	// see EXPECTS_HASH_INDEX!!
	WR_EX_STRUCT     = 0xC0,  // 1100
	WR_EX_HASH_TABLE = 0xE0,  // 1110
};

//------------------------------------------------------------------------------
enum WRGCObjectType
{
	SV_VALUE = 0x01,           // 0001
	SV_CHAR = 0x02,            // 0010
	SV_HASH_TABLE = 0x03,      // 0011
	SV_VOID_HASH_TABLE = 0x04, // 0100 // used for wrench internals, not meant for VM to care about
};

#define EX_TYPE_MASK   0xE0
#define IS_DEBUG_BREAK(X) ((X)==WR_EX_DEBUG_BREAK)
#define IS_ARRAY_MEMBER(X) (((X)&EX_TYPE_MASK)==WR_EX_ARRAY_MEMBER)
#define IS_ARRAY(X) ((X)==WR_EX_ARRAY)
#define IS_ITERATOR(X) ((X)==WR_EX_ITERATOR)
#define IS_RAW_ARRAY(X) (((X)&EX_TYPE_MASK)==WR_EX_RAW_ARRAY)
#define IS_HASH_TABLE(X) ((X)==WR_EX_HASH_TABLE)
#define EXPECTS_HASH_INDEX(X) ( ((X)==WR_EX_STRUCT) || ((X)==WR_EX_HASH_TABLE) )

#ifdef ARDUINO
#include <Arduino.h>
#endif

#if (defined(BYTE_ORDER) && BYTE_ORDER == BIG_ENDIAN) || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) || (defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN) || defined(__BIG_ENDIAN__) || defined(__ARMEB__) || defined(__THUMBEB__) || defined(__AARCH64EB__) || defined(_MIBSEB) || defined(__MIBSEB) || defined(__MIBSEB__) || defined(ARDUINO_BIG_ENDIAN)
  #define WRENCH_BIG_ENDIAN
#elif (defined(BYTE_ORDER) && BYTE_ORDER == LITTLE_ENDIAN) || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) || (defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN) || defined(__LITTLE_ENDIAN__) || defined(__ARMEL__) || defined(_M_PPC) || defined(__THUMBEL__) || defined(__AARCH64EL__) || defined(_MSC_VER) || defined(_MIPSEL) || defined(__MIPSEL) || defined(__MIPSEL__) || defined(ARDUINO_LITTLE_ENDIAN)
  #define WRENCH_LITTLE_ENDIAN
#else
  #error "Endian-ness not detected! Please contact curt.hartung@gmail.com so it can be added <01>"
#endif

class WRGCObject;

//------------------------------------------------------------------------------
struct WRIteratorEntry
{
	int type; // SV_VALUE, SV_CHAR or SV_HASH_TABLE

	const WRValue* key; // if this is a hash table the key value
	const WRValue* value; // for SV_HASH_TABLE and SV_VALUE (character will be null)

	int index; // array entry for non-hash tables
	char character; // for SV_CHAR (value will be null)
};

//------------------------------------------------------------------------------
struct WRValue
{
	// never reference the data members directly, they are unions and
	// bad things will happen. Always access them with one of these
	// methods
	int asInt() const;
	void setInt( const int val );
	
	float asFloat() const;
	void setFloat( const float val );

	bool isFloat() const {return type == WR_FLOAT || (type == WR_REF && r->type == WR_FLOAT); }
	bool isInt() const { return type == WR_INT || (type == WR_REF && r->type == WR_INT); }
	bool isString( int* len =0 ) const;
	bool isWrenchArray( int* len =0 ) const;
	bool isRawArray( int* len =0 ) const;
	bool isHashTable( int* members=0 ) const;

	// if this value is an array, return 'index'-th element
	// if create is true and this value is NOT an array, it will be converted into one
	WRValue* indexArray( WRContext* context, const uint32_t index, const bool create );
	
	// if this value is a hash table, return [or create] the 'index' hash item
	// if create is true and this value is NOT a hash, it will be converted into one
	WRValue* indexHash( WRContext* context, const uint32_t hash, const bool create );
	
	// string: must point to a buffer long enough to contain at least maxLen bytes.
	// the "string" pointer will be passed back, if maxLen is 0 (not
	// reccomended) the string is assumed to be unlimited size
	char* asString( char* string, unsigned int maxLen =0, unsigned int* strLen =0 ) const;

	// malloc a string of sufficient size and copy/format the contents
	// of this value into it, the string muyst be g_free'ed
	char* asMallocString( unsigned int* strLen =0 ) const;
	class MallocStrScoped // helper class for asMallocString()
	{
	public:
		operator bool()        const { return m_str != 0; }
		operator const char*() const { return m_str; }
		unsigned int size()    const { return m_size; }
		MallocStrScoped( WRValue const& V ) : m_size(0), m_str(V.asMallocString(&m_size)) {}
		~MallocStrScoped() { g_free((char*)m_str); }
	private:
		unsigned int m_size;
		const char* m_str;
	};

	// same as "asString" but will print it in a more debug-symbol-y
	char* technicalAsString( char* string, unsigned int maxLen, bool valuesInHex =false, unsigned int* strLen =0 ) const;

	// return a raw pointer to the data array if this is one, otherwise null
	void* array( unsigned int* len =0, char arrayType =SV_CHAR ) const;
	int arraySize() const; // returns length of the array or -1 if this value is not an array

//private: // is what this SHOULD be.. but that's impractical since the
	// VM is not an object that can be friended.

	inline WRValue* init() { p = 0; p2 = 0; return this; } // call upon first create or when you're sure no memory is hanging from one

	WRValue& singleValue() const; // return a single value for comparison
	WRValue& deref() const; // if this value is a ARRAY_MEMBER, copy the referred value in

	uint32_t getHash() const { return (type <= WR_FLOAT) ? ui : getHashEx(); } // easy cases
	uint32_t getHashEx() const; // harder

	union // first 4 bytes 
	{
		int32_t i;
		uint32_t ui;
		WRValue* frame;
		float f;
		const void* p;
		void* r1;
		char* c;
		WRValue* r;
		WRGCObject* va;
		WR_C_CALLBACK ccb;
		WR_LIB_CALLBACK lcb;
		WRFunction* wrf;
	};

	union
	{
#ifdef WRENCH_LITTLE_ENDIAN
		struct
		{
#if (__cplusplus <= 199711L)
			uint8_t type; // carries the type
			uint8_t padL; // pad carries an array pointer (up to 2megs using bottom 4 bits of xtype)
			uint8_t padH;
			uint8_t xtype; // carries the extra type (if it exists)
#else
			WRValueType type; // carries the type
			uint8_t padL; // pad carries an array pointer (up to 2megs using bottom 4 bits of xtype)
			uint8_t padH;
			WRExType xtype; // carries the extra type (if it exists)
#endif
		};
#else // WRENCH_BIG_ENDIAN
		struct
		{
#if (__cplusplus <= 199711L)
			uint8_t xtype; // carries the extra type (if it exists)
			uint8_t padH;
			uint8_t padL; // pad carries an array pointer (up to 2megs using bottom 4 bits of xtype)
			uint8_t type; // carries the type
#else
			WRExType xtype; // carries the extra type (if it exists)
			uint8_t padH;
			uint8_t padL; // pad carries an array pointer (up to 2megs using bottom 4 bits of xtype)
			WRValueType type; // carries the type
#endif
		};
#endif
		uint32_t p2;
		union
		{
			int returnOffset;
			WRValue* r2;
			void* usr;
		};
	};

	WRValue() {}
	inline WRValue& operator= (const WRValue& V) { r1 = V.r1; r2 = V.r2; return *this; }
	inline WRValue(const WRValue& V) { r1 = V.r1; r2 = V.r2; }


public:

	//------------------------------------------------------------------------------
	class Iterator
	{
	public:
		Iterator( WRValue const& V );
		
		Iterator() : m_va(0) {}

		bool operator!=( const Iterator& other )
		{
			return m_va != other.m_va;
		}

		WRIteratorEntry const& operator* () const { return m_current; }

		const Iterator operator++();
			
	private:

		WRIteratorEntry m_current;
		WRGCObject const* m_va;
		unsigned int m_element;
	};
	const Iterator begin() const { return Iterator(deref()); }
	const Iterator end() const { return Iterator(); }
};

//------------------------------------------------------------------------------
// Helper class to represent a wrench value, in all cases it does NOT
// manage the memory, but relies on a WRContext to do that
class WrenchValue
{
public:
		
	WrenchValue( WRContext* context, const char* label ) : m_context(context), m_value( wr_getGlobalRef(context, label) ) {}
	WrenchValue( WRContext* context, WRValue* value ) : m_context(context), m_value(value) {}

	bool isValid() const { return m_value ? true : false; }

	// if the value is not the correct type it will be converted to
	// that type, preserving the value as best it can
	operator int32_t* () { return Int(); }
	int32_t* Int();
	
	operator float* () { return Float(); }
	float* Float();

	// this will convert it to an array if it isn't one
	WRValue& operator[] ( const int index ) { return *asArrayMember( index ); }
	WRValue* asArrayMember( const int index );
	int arraySize() const { return m_value ? m_value->arraySize() : -1; } // returns -1 if this is not an array

private:
	WRContext* m_context;
	WRValue* m_value;
};


#ifdef WRENCH_TIME_SLICES
//------------------------------------------------------------------------------
// Helper class to schedule and run tasks
struct WrenchScheduledTask;

class WrenchScheduler
{
public:
	WrenchScheduler( const int stackSizePerThread = WRENCH_DEFAULT_STACK_SIZE );
	~WrenchScheduler();

	WRState* state() const { return m_w; }

	void tick( const int instructionsPerSlice =1000 );

	// returns a task ID
	int addThread( const uint8_t* byteCode, const int size, const int instructionsThisSlice =1000, const bool takeOwnership =false );
	bool removeTask( const int taskId );

private:
	WRState* m_w;
	WrenchScheduledTask* m_tasks;
};
#endif

#define WRENCH_NULL_HASH 0xABABABAB  // -1414812757 / -1.2197928214371934e-12, can't be zero since we use int/floats as their own hash

#ifdef WRENCH_INCLUDE_DEBUG_CODE
#ifndef WRENCH_COMBINED
#include "utils/simple_ll.h"
#endif

struct WrenchPacket;
class WRDebugServerInterface;
class WRDebugServerInterfacePrivate;
class WRDebugClientInterfacePrivate;
class WrenchDebugCommInterface;
template<class> class SimpleLL;

//------------------------------------------------------------------------------
struct WrenchDebugValue
{
	char name[64];
	WRValue value;
};

//------------------------------------------------------------------------------
struct WrenchSymbol
{
	char label[64];
};

//------------------------------------------------------------------------------
struct WrenchFunction
{
	char name[64];
	int arguments;
	SimpleLL<WrenchSymbol>* vars;
};

//------------------------------------------------------------------------------
struct WrenchCallStackEntry
{
	int32_t onLine;

	uint8_t fromUnitIndex;
	uint8_t thisUnitIndex;
	uint16_t locals;

	uint8_t arguments;
	uint8_t stackOffset;
};

//------------------------------------------------------------------------------
enum WRP_ProcState
{
	WRP_Unloaded =0,
	WRP_Loaded,
	WRP_Running,
	WRP_Complete
};

//------------------------------------------------------------------------------
// This object runs as the main interface to a running debug image, it
// can drive it remotely over a serial (or whatever) link or directly
// through a pointer.
class WRDebugClientInterface
{
public:
	// server is remote, define these methods to talk to it
	WRDebugClientInterface( WrenchDebugCommInterface* CommInterface );

	// load and prepare a program for running
	// if byteCode/size is null then previoous code is re-loaded
	void load( const uint8_t* byteCode =0, const int size =0 ); 

	bool getSourceCode( const char** data, int* len ); // get a new'ed pointer to the source code
	uint32_t getSourceCodeHash(); // get the crc of the code that was compiled

	SimpleLL<WrenchFunction>& getFunctions(); // global is function[0] 

	// 0 is global, 1,2,3... etc are frames
	SimpleLL<WrenchCallStackEntry>* getCallstack();
	bool getStackDump( char* out, const unsigned int maxOut );
	
	const char* getFunctionLabel( const int index );
	const char* getValueLabel( const int index, const int depth );
	WRValue* getValue( WRValue& value, const int index, const int depth );

	void run( const int toLine =0 );

	void stepInto( const int steps =1 );
	void stepOver( const int steps =1 );

	void setBreakpoint( const int lineNumber );
	void clearBreakpoint( const int lineNumber );

	WRP_ProcState getProcState();

	void setDebugEmitCallback( void (*print)( const char* debugText ) );

	// "hidden" internals to keep header clean
	WRDebugClientInterfacePrivate* I;
	~WRDebugClientInterface();

	void init();
};

//------------------------------------------------------------------------------
// This object is run on the target machine to manage the running
// image. It has a minimal interface and is meant to be driven entirely
// by WRDebugClientInterface
class WRDebugServerInterface
{
public:

	WRDebugServerInterface( WRState* w, WrenchDebugCommInterface* CommInterface );

	void tick();

	// NOTE: loadBytes() does NOT make a local copy! bytes must remain valid for the life of this object!
	WRContext* loadBytes( const uint8_t* bytes, const int len );

	// "hidden" internals to keep header clean
	WRDebugServerInterfacePrivate *I;
	~WRDebugServerInterface();
};

#endif

#ifdef WRENCH_REALLY_COMPACT
#ifndef WRENCH_COMPACT
#define WRENCH_COMPACT
#endif
#endif

#ifndef WRENCH_COMBINED
#include "utils/utils.h"
#include "vm/gc_object.h"
#include "utils/serializer.h"
#include "utils/simple_args.h"
#include "vm/vm.h"
#include "utils/opcode.h"
#include "cc/str.h"
#include "cc/opcode_stream.h"
#include "debug/packet.h"
#include "debug/debug.h"
#include "debug/debugger.h"
#include "cc/cc.h"
#include "lib/std_tcp.h"
#include "lib/std_serial.h"
#include "debug/debug_comm.h"
#include "lib/std_io_defs.h"
#endif

#endif

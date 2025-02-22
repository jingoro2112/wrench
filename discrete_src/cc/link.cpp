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

#include "wrench.h"
#ifndef WRENCH_WITHOUT_COMPILER
#include <assert.h>

#define WR_DUMP_LINK_OUTPUT(D) //D
#define WR_DUMP_UNIT_OUTPUT(D) //D
#define WR_DUMP_BYTECODE(D) //D

//------------------------------------------------------------------------------
void WRCompilationContext::createLocalHashMap( WRUnitContext& unit, unsigned char** buf, int* size )
{
	if ( unit.bytecode.localSpace.count() == 0 )
	{
		*size = 0;
		*buf = 0;
		return;
	}

	WRHashTable<unsigned char> offsets;
	for( unsigned char i=unit.arguments; i<unit.bytecode.localSpace.count(); ++i )
	{
		offsets.set( unit.bytecode.localSpace[i].hash, i - unit.arguments );
	}

	*buf = (unsigned char *)g_malloc( (offsets.m_mod * 5) + 4 );

	(*buf)[0] = (unsigned char)(unit.bytecode.localSpace.count() - unit.arguments);
	(*buf)[1] = unit.arguments;

	wr_pack16( offsets.m_mod, *buf + 2 );

	*size = 4;

	for( int i=0; i<offsets.m_mod; ++i )
	{
		wr_pack32( offsets.m_list[i].hash, *buf + *size );
		*size += 4;
		(*buf)[(*size)++] = offsets.m_list[i].hash ? offsets.m_list[i].value : (unsigned char)-1;
	}
}

//------------------------------------------------------------------------------
void WRCompilationContext::link( unsigned char** out, int* outLen, const uint8_t compilerOptionFlags )
{
	WROpcodeStream code;

	uint8_t data[4];

	NamespacePush *namespaceLookups = 0;

	unsigned int globals = m_units[0].bytecode.localSpace.count();
	assert( globals < 256 );

	code += (uint8_t)globals; // globals count (for VM allocation)
	code += (uint8_t)(m_units.count() - 1); // function count (for VM allocation)
	code += (uint8_t)compilerOptionFlags;

	// push function signatures
	for( unsigned int u=1; u<m_units.count(); ++u )
	{
		m_units[u].offsetInBytecode = code.size(); // mark these two spots for later

		// WRFunction.namespaceOffset
		data[0] = 0xAA;
		data[1] = 0xBB;
		code.append( data, 2 ); // placeholder

		// WRFunction.functionOffset
		data[0] = 0xCC;
		data[1] = 0xDD;
		code.append( data, 2 ); // placeholder

		// WRFunction.hash
		code.append( wr_pack32(m_units[u].hash, data), 4 );

		// WRFunction.arguments;
		data[0] = m_units[u].arguments;
		code.append( data, 1 );

		// WRFunction.frameSpaceNeeded;
		data[1] = m_units[u].bytecode.localSpace.count() - m_units[u].arguments;
		code.append( data + 1, 1 );

		// WRFunction.frameBaseAdjustment;
		data[2] = 1 + data[0] + data[1];
		code.append( data + 2, 1 );
	}

	m_units[0].name = "::global";

	if ( compilerOptionFlags & WR_INCLUDE_GLOBALS )
	{
		for( unsigned int i=0; i<globals; ++i )
		{
			code.append( wr_pack32(m_units[0].bytecode.localSpace[i].hash, data), 4 );
		}
	}

	if ( (compilerOptionFlags & WR_EMBED_DEBUG_CODE)
		 || (compilerOptionFlags & WR_EMBED_SOURCE_CODE) )
	{
		// hash of source compiled for this
		code.append( wr_pack32(wr_hash(m_source, m_sourceLen), data), 4 ); 

		WRstr symbols;

		uint16_t functions = m_units.count();
		symbols.append( (char*)wr_pack16(functions, data), 2 );

		data[0] = 0; // terminator
		for( unsigned int u=0; u<m_units.count(); ++u ) // all the local labels by unit
		{
			data[1] = m_units[u].bytecode.localSpace.count();
			data[2] = m_units[u].arguments;
			symbols.append( (char *)(data + 1), 2 );

			symbols.append( m_units[u].name );
			symbols.append( (char *)data, 1 ); 

			for( unsigned int s=0; s<m_units[u].bytecode.localSpace.count(); ++s )
			{
				symbols.append( m_units[u].bytecode.localSpace[s].label );
				symbols.append( (char *)data, 1 ); 
			}
		}

		uint16_t symbolSize = symbols.size();
		code.append( wr_pack16(symbolSize, data), 2 );
		code.append( (uint8_t*)symbols.p_str(), symbols.size() );
	}

	if ( compilerOptionFlags & WR_EMBED_SOURCE_CODE )
	{
		code.append( wr_pack32(m_sourceLen, data), 4 );
		code.append( (uint8_t*)m_source, m_sourceLen );
	}

	// export any explicitly marked for export or are referred to by a 'new'
	for( unsigned int ux=0; ux<m_units.count(); ++ux )
	{
		for( unsigned int f=0; f<m_units[ux].bytecode.unitObjectSpace.count(); ++f )
		{
			WRNamespaceLookup& N = m_units[ux].bytecode.unitObjectSpace[f];

			for( unsigned int r=0; r<N.references.count(); ++r )
			{
				for( unsigned int u2 = 1; u2<m_units.count(); ++u2 )
				{
					if ( m_units[u2].hash == N.hash )
					{
						m_units[u2].exportNamespace = true;
						break;
					}
				}
			}
		}
	}

	WR_DUMP_LINK_OUTPUT(printf("header funcs[%d] locals[%d] flags[0x%02X]:\n%s\n",
							   (unsigned char)(m_units.count() - 1),
							   (unsigned char)(m_units[0].bytecode.localSpace.count()),
							   (unsigned char)compilerOptionFlags, wr_asciiDump( code.p_str(), code.size(), str )));

	// append all the unit code
	for( unsigned int u=0; u<m_units.count(); ++u )
	{
		uint16_t base;

		if ( u > 0 ) // for the non-zero unit fill locations into the jump table
		{
			if ( m_units[u].exportNamespace )
			{
				base = code.size();

				int size = 0;
				uint8_t* map = 0;
				createLocalHashMap( m_units[u], &map, &size );
				
				if ( size == 0 )
				{
					base = 0;
				}
				m_units[u].offsetOfLocalHashMap = base;

				wr_pack16( base, code.p_str(m_units[u].offsetInBytecode)); // WRFunction.namespaceOffset

				WR_DUMP_LINK_OUTPUT(printf("<new> namespace\n%s\n", wr_asciiDump(map, size, str)));

				code.append( map, size );
				g_free( map );
			}
			else
			{
				base = 0;
				wr_pack16( base, code.p_str(m_units[u].offsetInBytecode) ); // WRFunction.namespaceOffset
			}

			base = code.size();
			wr_pack16( base, code.p_str(m_units[u].offsetInBytecode + 2) ); // WRFunction.functionOffset
		}

		WR_DUMP_UNIT_OUTPUT(printf("unit %d:\n%s\n", u, wr_asciiDump(code, code.size(), str)));

		base = code.size();

		// fill relative jumps in for the goto's
		for( unsigned int g=0; g<m_units[u].bytecode.gotoSource.count(); ++g )
		{
			unsigned int j=0;
			for( ; j<m_units[u].bytecode.jumpOffsetTargets.count(); j++ )
			{
				if ( m_units[u].bytecode.jumpOffsetTargets[j].gotoHash == m_units[u].bytecode.gotoSource[g].hash )
				{
					int diff = m_units[u].bytecode.jumpOffsetTargets[j].offset - m_units[u].bytecode.gotoSource[g].offset;
					diff -= 2;
					if ( (diff < 128) && (diff > -129) )
					{
						*m_units[u].bytecode.all.p_str( m_units[u].bytecode.gotoSource[g].offset ) = (unsigned char)O_RelativeJump8;
						*m_units[u].bytecode.all.p_str( m_units[u].bytecode.gotoSource[g].offset + 1 ) = diff;
					}
					else
					{
						*m_units[u].bytecode.all.p_str( m_units[u].bytecode.gotoSource[g].offset ) = (unsigned char)O_RelativeJump;
						wr_pack16( diff, m_units[u].bytecode.all.p_str(m_units[u].bytecode.gotoSource[g].offset + 1) );
					}

					break;
				}
			}

			if ( j >= m_units[u].bytecode.jumpOffsetTargets.count() )
			{
				m_err = WR_ERR_goto_target_not_found;
				return;
			}
		}

		WR_DUMP_LINK_OUTPUT(printf("Adding Unit %d [%d]\n%s", u, m_units[u].bytecode.all.size(), wr_asciiDump(m_units[u].bytecode.all, m_units[u].bytecode.all.size(), str)));

		code.append( m_units[u].bytecode.all, m_units[u].bytecode.all.size() );

		WR_DUMP_LINK_OUTPUT(printf("->\n%s\n", wr_asciiDump( code.p_str(), code.size(), str )));

		// populate 'new' vectors
		for( unsigned int f=0; f<m_units[u].bytecode.unitObjectSpace.count(); ++f )
		{
			WRNamespaceLookup& N = m_units[u].bytecode.unitObjectSpace[f];

			for( unsigned int r=0; r<N.references.count(); ++r )
			{
				for( unsigned int u2 = 1; u2<m_units.count(); ++u2 )
				{
					if ( m_units[u2].hash == N.hash )
					{
						NamespacePush *n = (NamespacePush *)g_malloc(sizeof(NamespacePush));
						n->next = namespaceLookups;
						namespaceLookups = n;
						n->unit = u2;
						n->location = base + N.references[r];

						break;
					}
				}
			}
		}

		// load function table
		for( unsigned int x=0; x<m_units[u].bytecode.functionSpace.count(); ++x )
		{
			WRNamespaceLookup& N = m_units[u].bytecode.functionSpace[x];

			WR_DUMP_LINK_OUTPUT(printf("function[%d] fixup before:\n%s\n", x, wr_asciiDump(code, code.size(), str)));

			for( unsigned int r=0; r<N.references.count(); ++r )
			{
				unsigned int u2 = 1;
				int index = base + N.references[r];

				for( ; u2<m_units.count(); ++u2 )
				{
					if ( m_units[u2].hash == N.hash ) // local function! call it, manage preserving retval
					{
						if ( m_addDebugSymbols )
						{
							// fill in the codeword with internal function number
							uint16_t codeword = (uint16_t)WRD_FunctionCall | (uint16_t)u2;
							wr_pack16( codeword, (unsigned char *)code.p_str(index - 2) );
						}

						code[index] = O_CallFunctionByIndex;

						code[index+2] = (char)(u2 - 1);

						// r+1 = args
						// r+2345 = hash
						// r+6

						if ( code[index+5] == O_PopOne || code[index + 6] == O_NewObjectTable)
						{
							code[index+3] = 3; // skip past the pop, or TO the NewObjectTable
						}
						else 
						{
							code[index+3] = 2; // skip by this push is seen
							code[index+5] = O_PushIndexFunctionReturnValue;
						}

						break;
					}
				}

				if ( u2 >= m_units.count() ) // no local function found, rely on it being found run-time
				{
					if ( N.hash == wr_hashStr("yield") )
					{
						code[index] = O_Yield;
					}
					else
					{
						if ( code[index+5] == O_PopOne )
						{
							code[index] = O_CallFunctionByHashAndPop;
						}
						else
						{
							code[index] = O_CallFunctionByHash;
						}

						wr_pack32( N.hash, code.p_str(index+2) );
					}
				}
			}

			WR_DUMP_LINK_OUTPUT(printf("function[%d] fixup after:\n%s\n", x, wr_asciiDump(code, code.size(), str)));
		}
	}

	// plug in namespace lookups
	while( namespaceLookups )
	{
		wr_pack16( m_units[namespaceLookups->unit].offsetOfLocalHashMap, code.p_str(namespaceLookups->location) );
		NamespacePush* next = namespaceLookups->next;
		g_free( namespaceLookups );
		namespaceLookups = next;
	}

	// append a CRC
	uint32_t salted = wr_hash( code, code.size() );
	salted += WRENCH_VERSION_MAJOR;
	code.append( wr_pack32(salted, data), 4 );

	WR_DUMP_BYTECODE(printf("bytecode [%d]:\n%s\n", code.size(), wr_asciiDump(code, code.size(), str)));

	if ( !m_err )
	{
		*outLen = code.size();
		code.release( out );
	}
}

#endif

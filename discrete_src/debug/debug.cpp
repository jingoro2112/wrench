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

//------------------------------------------------------------------------------
void wr_formatGCObject( WRGCObject const& obj, WRstr& out )
{
	switch( obj.m_type )
	{
		case SV_VALUE:
		{
			out.appendFormat( "SV_VALUE : size[%d]", obj.m_size ); 
			break;
		}
		
		case SV_CHAR:
		{
			out.appendFormat( "SV_CHAR : size[%d]", obj.m_size ); 
			break;
		}
		
		case SV_HASH_TABLE:
		{
			out.appendFormat( "SV_HASH_TABLE : mod[%d] size[%d] ", obj.m_mod, obj.m_size ); 
			break;
		}
		
		case SV_VOID_HASH_TABLE:
		{
			out.appendFormat( "SV_VOID_HASH_TABLE : @[%p] mod[%d] ", obj.m_ROMHashTable, obj.m_mod ); 
			break;
		}
	}
}

//------------------------------------------------------------------------------
void wr_formatStackEntry( const WRValue* v, WRstr& out )
{
	out.appendFormat( "%p: ", v );
	
	switch( v->type )
	{
		default:
		{
			out.appendFormat( "frame [0x%08X]:[0x%08X]\n", v->p, v->p2 );
			break;
		}
		
		case WR_INT:
		{
			out.appendFormat( "Int [%d]\n", v->i );
			break;
		}

		case WR_FLOAT:
		{
			out.appendFormat( "Float [%g]\n", v->f );
			break;
		}

		case WR_REF:
		{
			out.appendFormat( "r [%p] ->\n", v->r );
			wr_formatStackEntry( v->r, out );
			break;
		}

		case WR_EX:
		{
			switch( v->xtype )
			{
				case WR_EX_RAW_ARRAY:
				{
					out.appendFormat( "EX:RAW_ARRAY [%p] size[%d]\n", v->r->c, EX_RAW_ARRAY_SIZE_FROM_P2(v->r->p2));
					break;
				}

				case WR_EX_DEBUG_BREAK:
				{
					out.appendFormat( "EX:DEBUG_BREAK\n" );
					break;
				}

				case WR_EX_ITERATOR:
				{
					// todo- break out VA
					out.appendFormat( "EX:ITERATOR of[%p] @[%d]\n", v->va, DECODE_ARRAY_ELEMENT_FROM_P2(v->p2) );
					break;
				}

				case WR_EX_ARRAY_MEMBER:
				{
					// todo- decode r
					out.appendFormat( "EX:ARRAY_MEMBER element[%d] of[%p]\n", DECODE_ARRAY_ELEMENT_FROM_P2(v->p2), v->r );
					break;
				}
				
				case WR_EX_ARRAY:
				{
					// todo- decode va
					out.appendFormat( "EX_ARRAY :" );
					wr_formatGCObject( *v->va, out );
					out += "\n";
					break;
				}

				case WR_EX_STRUCT:
				{
					out.appendFormat( "EX:STRUCT\n" );
					break;
				}
				
				case WR_EX_HASH_TABLE:
				{
					out.appendFormat( "EX:HASH_TABLE :" );
					wr_formatGCObject( *v->va, out );
					out += "\n";

					break;
				}
			}
		}
	}
}

//------------------------------------------------------------------------------
void wr_stackDump( const WRValue* bottom, const WRValue* top, WRstr& out )
{
	out.clear();
	
	for( const WRValue* v=bottom; v<top; ++v)
	{
		out.appendFormat( "[%d] ", (int)(top - v) );
		wr_formatStackEntry( v, out );
	}
}

#endif

/*******************************************************************************
Copyright (c) 2022 Curt Hartung -- curt.hartung@gmail.com

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

//------------------------------------------------------------------------------
bool serializeEx( WRValueSerializer& serializer, const WRValue& val )
{
	char temp;
	uint16_t temp16;
	
	const WRValue& value = val.deref();

	serializer.write( &(temp = value.type), 1 );

	switch( (uint8_t)value.type )
	{
		case WR_INT:
		case WR_FLOAT:
		{
			serializer.write( (char *)&value.ui, 4 );
			return true;
		}

		case WR_EX:
		{
			serializer.write( &(temp = value.xtype), 1 );

			switch( (uint8_t)value.xtype )
			{
				case WR_EX_ARRAY:
				{
					serializer.write( &(temp = value.va->m_type), 1 );
					serializer.write( (char *)&(temp16 = value.va->m_size), 2 );

					if ( value.va->m_type == SV_CHAR )
					{
						serializer.write( value.va->m_SCdata, value.va->m_size );
						return true;
					}
					else if ( value.va->m_type == SV_VALUE )
					{
						for( uint32_t i=0; i<value.va->m_size; ++i )
						{
							if ( !serializeEx(serializer, value.va->m_Vdata[i]) )
							{
								return false;
							}
						}
					}

					return true;
				}

				case WR_EX_HASH_TABLE:
				{
					serializer.write( (char *)&(temp16 = value.va->m_mod), 2 );

					for( uint32_t i=0; i<value.va->m_mod; ++i )
					{
						if ( !value.va->m_hashTable[i] )
						{
							serializer.write( &(temp = 0), 1 );
						}

						serializer.write( &(temp = 1), 1 );

						serializeEx( serializer, value.va->m_Vdata[i<<1] );
						serializeEx( serializer, value.va->m_Vdata[(i<<1) + 1] );
					}

					return true;
				}
				
				default: break;
			}
		}
	}

	return false;
}

//------------------------------------------------------------------------------
bool deserializeEx( WRValue& value, WRValueSerializer& serializer, WRContext* context )
{
	char temp;
	uint16_t temp16;

	if ( !serializer.read(&temp , 1) )
	{
		return false;
	}

	value.p2 = (int)temp;

	switch( (uint8_t)temp )
	{
		case WR_INT:
		case WR_FLOAT:
		{
			return serializer.read( (char *)&value.ui, 4 ); // bitpattern
		}

		case WR_EX:
		{
			if ( !serializer.read(&temp, 1) )
			{
				return false;
			}

			switch( (uint8_t)temp )
			{
				case WR_EX_ARRAY:
				{
					if ( !serializer.read(&temp, 1)  // type
						 || !serializer.read((char *)&temp16, 2) ) // size
					{
						return false;
					}

					value.p2 = INIT_AS_ARRAY;
					
					switch( (uint8_t)temp )
					{
						case SV_CHAR:
						{
							value.va = context->getSVA( temp16, SV_CHAR, false );
							if ( !serializer.read(value.va->m_SCdata, temp16) )
							{
								return false;
							}
							return true;
						}

						case SV_VALUE:
						{
							value.va = context->getSVA( temp16, SV_VALUE, false );
							for( uint16_t i=0; i<temp16; ++i )
							{
								if ( !deserializeEx(value.va->m_Vdata[i], serializer, context) )
								{
									return false;
								}
							}

							return true;
						}
					}

					break;
				}

				case WR_EX_HASH_TABLE:
				{
					if ( !serializer.read((char *)&temp16, 2) ) // size
					{
						return false;
					}

					value.p2 = INIT_AS_HASH_TABLE;
					value.va = context->getSVA( temp16 - 1, SV_HASH_TABLE, false );

					for( uint16_t i=0; i<temp16; ++i )
					{
						if ( !serializer.read(&temp, 1) )
						{
							return false;
						}

						if ( !temp )
						{
							value.va->m_Vdata[i<<1].init();
							value.va->m_Vdata[(i<<1) + 1].init();
							value.va->m_hashTable[i] = 0;
						}
						else
						{
							if ( !deserializeEx(value.va->m_Vdata[i<<1], serializer, context)
								 || !deserializeEx(value.va->m_Vdata[(i<<1)+1], serializer, context) )
							{
								return false;
							}
							value.va->m_hashTable[i] = value.va->m_Vdata[(i<<1)+1].getHash() ^ HASH_SCRAMBLER;
						}
					}
					
					return true;
				}

				default: break;
			}
		}
	}

	return false;
}

//------------------------------------------------------------------------------
bool wr_serialize( char** buf, int* len, const WRValue& value )
{
	WRValueSerializer S;
	if ( !serializeEx(S, value) )
	{
		return false;
	}

	S.getOwnership( buf, len );
	return true;
}

//------------------------------------------------------------------------------
bool wr_deserialize( WRContext* context, WRValue& value, const char* buf, const int len )
{
	WRValueSerializer S( buf, len );
	return deserializeEx( value, S, context );
}

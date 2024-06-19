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

#ifdef WRENCH_TIME_SLICES

//------------------------------------------------------------------------------
struct WrenchScheduledTask
{
	WRContext* context;
	WrenchScheduledTask* next;
	int id;
};

//------------------------------------------------------------------------------
WrenchScheduler::WrenchScheduler( const int stackSizePerThread )
{
	m_w = wr_newState( stackSizePerThread );
	m_tasks = 0;
}

//------------------------------------------------------------------------------
WrenchScheduler::~WrenchScheduler()
{
	while( m_tasks )
	{
		WrenchScheduledTask* next = m_tasks->next;
		g_free( m_tasks );
		m_tasks = next;
	}

	wr_destroyState( m_w );
}

//------------------------------------------------------------------------------
void WrenchScheduler::tick( int instructionsPerSlice )
{
	wr_setInstructionsPerSlice( instructionsPerSlice );
	WrenchScheduledTask* task = m_tasks;

	while( task )
	{
		if ( !task->context->yield_pc )
		{
			const int id = task->id;
			task = task->next;
			removeTask( id );
		}
		else
		{
			wr_callFunction( task->context, (WRFunction*)0, task->context->yield_argv, task->context->yield_argn );
			task = task->next;
		}
	}
}

static int wr_idGenerator = 0;
//------------------------------------------------------------------------------
int WrenchScheduler::addThread( const uint8_t* byteCode, const int size, const int instructionsThisSlice, const bool takeOwnership )
{
	wr_setInstructionsPerSlice( instructionsThisSlice );

	WRContext* context = wr_run( m_w, byteCode, size, takeOwnership );
	if ( !context )
	{
		return -1; // error running task
	}
	else if ( !wr_getYieldInfo(context) )
	{
		return 0; // task ran to completion there was no need to yield
	}

	WrenchScheduledTask* task = (WrenchScheduledTask*)g_malloc( sizeof(WrenchScheduledTask) );
	task->context = context;
	task->next = m_tasks;
	task->id = ++wr_idGenerator;
	m_tasks = task;
			  	
	return task->id;
}

//------------------------------------------------------------------------------
bool WrenchScheduler::removeTask( const int taskId )
{
	WrenchScheduledTask* last = 0;
	WrenchScheduledTask* task = m_tasks;
	while( task )
	{
		if ( task->id == taskId )
		{
			wr_destroyContext( m_tasks->context );

			if ( last )
			{
				last->next = task->next;
			}
			else
			{
				m_tasks = task->next;
			}
			
			g_free( task );
			return true;
		}

		last = task;
		task = task->next;
	}
	
	return false;
}

#endif

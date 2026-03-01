# Wrench nmake Makefile for Windows (MSVC)
# Usage: nmake
# Run from a Visual Studio Developer Command Prompt

OBJDIR = objs_win32

OPT = /O2
#OPT = /Od /Zi

CXXFLAGS = $(OPT) /nologo /EHsc /std:c++14 /I. /Idiscrete_src /DWRENCH_WIN32_FILE_IO /DWRENCH_WIN32_SERIAL

CC = cl /nologo $(CXXFLAGS) /c

OBJS = \
	$(OBJDIR)\cc.obj \
	$(OBJDIR)\expression.obj \
	$(OBJDIR)\link.obj \
	$(OBJDIR)\optimizer.obj \
	$(OBJDIR)\token.obj \
	$(OBJDIR)\operations.obj \
	$(OBJDIR)\operations_compact.obj \
	$(OBJDIR)\operations_non_compact.obj \
	$(OBJDIR)\index.obj \
	$(OBJDIR)\gc.obj \
	$(OBJDIR)\gc_object.obj \
	$(OBJDIR)\vm.obj \
	$(OBJDIR)\utils.obj \
	$(OBJDIR)\serializer.obj \
	$(OBJDIR)\debug.obj \
	$(OBJDIR)\disassemble.obj \
	$(OBJDIR)\server_debug.obj \
	$(OBJDIR)\packet.obj \
	$(OBJDIR)\client_debug.obj \
	$(OBJDIR)\server_interface_private.obj \
	$(OBJDIR)\client_interface_private.obj \
	$(OBJDIR)\scheduler.obj \
	$(OBJDIR)\std.obj \
	$(OBJDIR)\std_tcp.obj \
	$(OBJDIR)\tcp_debug_comm.obj \
	$(OBJDIR)\serial_debug_comm.obj \
	$(OBJDIR)\std_io.obj \
	$(OBJDIR)\std_string.obj \
	$(OBJDIR)\std_math.obj \
	$(OBJDIR)\std_msg.obj \
	$(OBJDIR)\std_sys.obj \
	$(OBJDIR)\std_serialize.obj \
	$(OBJDIR)\std_container.obj \
	$(OBJDIR)\debug_lib.obj \
	$(OBJDIR)\win32_comm.obj \
	$(OBJDIR)\fastled_lib.obj

all: wrench.exe

tests: test
tested: test

test: $(OBJDIR) $(OBJS)
	cl /nologo $(CXXFLAGS) /Isrc /DDOCTEST_CONFIG_IMPLEMENT_WITH_MAIN /c /Fo$(OBJDIR)\test_cross.obj discrete_src\utils\test_wrench_cross_module_globals.cpp
	cl /nologo /Fe:test_wrench_cross_module_globals.exe $(OBJS) $(OBJDIR)\test_cross.obj
	cl /nologo $(CXXFLAGS) /DWRENCH_ENABLE_CROSS_MODULE_EXTERNAL_TEST /c /Fo$(OBJDIR)\wrench_cli_test.obj discrete_src\utils\wrench_cli.cpp
	cl /nologo $(CXXFLAGS) /c /Fo$(OBJDIR)\debugger_test.obj discrete_src\debug\debugger.cpp
	cl /nologo /Fe:wrench_dev.exe $(OBJS) $(OBJDIR)\wrench_cli_test.obj $(OBJDIR)\debugger_test.obj
	.\test_wrench_cross_module_globals.exe
	.\wrench_dev.exe t

# Phase 3: compile amalgamated source into final binary
wrench.exe: src\wrench.cpp discrete_src\utils\wrench_cli.cpp discrete_src\debug\debugger.cpp
	cl /nologo $(CXXFLAGS) /Isrc /c /Fo$(OBJDIR)\ src\wrench.cpp discrete_src\utils\wrench_cli.cpp discrete_src\debug\debugger.cpp
	cl /nologo /Fe:wrench.exe $(OBJDIR)\wrench.obj $(OBJDIR)\wrench_cli.obj $(OBJDIR)\debugger.obj

# Phase 2: run intermediate tool to amalgamate discrete_src -> src/
src\wrench.cpp: wrench_dev.exe
	.\wrench_dev.exe release discrete_src src/.

# Phase 1: build intermediate tool from discrete sources
wrench_dev.exe: $(OBJDIR) $(OBJS)
	cl /nologo $(CXXFLAGS) /c /Fo$(OBJDIR)\ discrete_src\utils\wrench_cli.cpp discrete_src\debug\debugger.cpp
	cl /nologo /Fe:wrench_dev.exe $(OBJS) $(OBJDIR)\wrench_cli.obj $(OBJDIR)\debugger.obj

$(OBJDIR):
	if not exist $(OBJDIR) mkdir $(OBJDIR)

# -- compiler --
$(OBJDIR)\cc.obj: discrete_src\cc\cc.cpp
	$(CC) /Fo$@ $**

$(OBJDIR)\optimizer.obj: discrete_src\cc\optimizer.cpp
	$(CC) /Fo$@ $**

$(OBJDIR)\token.obj: discrete_src\cc\token.cpp
	$(CC) /Fo$@ $**

$(OBJDIR)\expression.obj: discrete_src\cc\expression.cpp
	$(CC) /Fo$@ $**

$(OBJDIR)\link.obj: discrete_src\cc\link.cpp
	$(CC) /Fo$@ $**

# -- vm --
$(OBJDIR)\operations.obj: discrete_src\vm\operations.cpp
	$(CC) /Fo$@ $**

$(OBJDIR)\operations_compact.obj: discrete_src\vm\operations_compact.cpp
	$(CC) /Fo$@ $**

$(OBJDIR)\operations_non_compact.obj: discrete_src\vm\operations_non_compact.cpp
	$(CC) /Fo$@ $**

$(OBJDIR)\index.obj: discrete_src\vm\index.cpp
	$(CC) /Fo$@ $**

$(OBJDIR)\gc.obj: discrete_src\vm\gc.cpp
	$(CC) /Fo$@ $**

$(OBJDIR)\gc_object.obj: discrete_src\vm\gc_object.cpp
	$(CC) /Fo$@ $**

$(OBJDIR)\vm.obj: discrete_src\vm\vm.cpp
	$(CC) /Fo$@ $**

$(OBJDIR)\scheduler.obj: discrete_src\vm\scheduler.cpp
	$(CC) /Fo$@ $**

# -- utils --
$(OBJDIR)\utils.obj: discrete_src\utils\utils.cpp
	$(CC) /Fo$@ $**

$(OBJDIR)\serializer.obj: discrete_src\utils\serializer.cpp
	$(CC) /Fo$@ $**

$(OBJDIR)\win32_comm.obj: discrete_src\utils\win32_comm.cpp
	$(CC) /Fo$@ $**

# -- lib --
$(OBJDIR)\std.obj: discrete_src\lib\std.cpp
	$(CC) /Fo$@ $**

$(OBJDIR)\std_tcp.obj: discrete_src\lib\std_tcp.cpp
	$(CC) /Fo$@ $**

$(OBJDIR)\std_io.obj: discrete_src\lib\std_io.cpp
	$(CC) /Fo$@ $**

$(OBJDIR)\std_string.obj: discrete_src\lib\std_string.cpp
	$(CC) /Fo$@ $**

$(OBJDIR)\std_math.obj: discrete_src\lib\std_math.cpp
	$(CC) /Fo$@ $**

$(OBJDIR)\std_msg.obj: discrete_src\lib\std_msg.cpp
	$(CC) /Fo$@ $**

$(OBJDIR)\std_sys.obj: discrete_src\lib\std_sys.cpp
	$(CC) /Fo$@ $**

$(OBJDIR)\std_serialize.obj: discrete_src\lib\std_serialize.cpp
	$(CC) /Fo$@ $**

$(OBJDIR)\std_container.obj: discrete_src\lib\std_container.cpp
	$(CC) /Fo$@ $**

$(OBJDIR)\debug_lib.obj: discrete_src\lib\debug_lib.cpp
	$(CC) /Fo$@ $**

$(OBJDIR)\fastled_lib.obj: discrete_src\lib\fastled_lib.cpp
	$(CC) /Fo$@ $**

# -- debug --
$(OBJDIR)\disassemble.obj: discrete_src\debug\disassemble.cpp
	$(CC) /Fo$@ $**

$(OBJDIR)\debug.obj: discrete_src\debug\debug.cpp
	$(CC) /Fo$@ $**

$(OBJDIR)\server_debug.obj: discrete_src\debug\server_debug.cpp
	$(CC) /Fo$@ $**

$(OBJDIR)\packet.obj: discrete_src\debug\packet.cpp
	$(CC) /Fo$@ $**

$(OBJDIR)\server_interface_private.obj: discrete_src\debug\server_interface_private.cpp
	$(CC) /Fo$@ $**

$(OBJDIR)\client_interface_private.obj: discrete_src\debug\client_interface_private.cpp
	$(CC) /Fo$@ $**

$(OBJDIR)\client_debug.obj: discrete_src\debug\client_debug.cpp
	$(CC) /Fo$@ $**

$(OBJDIR)\tcp_debug_comm.obj: discrete_src\debug\tcp_debug_comm.cpp
	$(CC) /Fo$@ $**

$(OBJDIR)\serial_debug_comm.obj: discrete_src\debug\serial_debug_comm.cpp
	$(CC) /Fo$@ $**

clean:
	-if exist $(OBJDIR) rmdir /s /q $(OBJDIR)
	-if exist wrench_dev.exe del /q wrench_dev.exe
	-if exist wrench.exe del /q wrench.exe

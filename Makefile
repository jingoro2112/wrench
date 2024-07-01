OPT ?= -O3
#OPT ?= -O0 -ggdb

PERF ?=

#C_VER ?= -std=c++11
C_VER ?= -std=c++98

#FLAGS ?= $(OPT) $(PERF) $(C_VER)
FLAGS ?= $(OPT) $(PERF) -Idiscrete_src -I. $(COMPACT) $(C_VER) -MD -DWRENCH_LINUX_FILE_IO -DWRENCH_LINUX_SERIAL

#FLAGS = $(OPT) -pg
#FLAGS = $(OPT) $(PERF)

all: wrench

OBJDIR = objs_linux

-include $(OBJDIR)/*.d

CC = g++ -MD -Wall -Werror $(FLAGS) -c -o
#CC = g++ -MD $(FLAGS) -c -o

OBJS = \
	$(OBJDIR)/cc.o \
	$(OBJDIR)/expression.o \
	$(OBJDIR)/link.o \
	$(OBJDIR)/optimizer.o \
	$(OBJDIR)/token.o \
	$(OBJDIR)/operations.o \
	$(OBJDIR)/index.o \
	$(OBJDIR)/gc.o \
	$(OBJDIR)/vm.o \
	$(OBJDIR)/utils.o \
	$(OBJDIR)/serializer.o \
	$(OBJDIR)/debug.o \
	$(OBJDIR)/disassemble.o \
	$(OBJDIR)/server_debug.o \
	$(OBJDIR)/packet.o \
	$(OBJDIR)/client_debug.o \
	$(OBJDIR)/server_interface_private.o \
	$(OBJDIR)/scheduler.o \
	$(OBJDIR)/std.o \
	$(OBJDIR)/std_io.o \
	$(OBJDIR)/std_io_linux.o \
	$(OBJDIR)/std_string.o \
	$(OBJDIR)/std_math.o \
	$(OBJDIR)/std_msg.o \
	$(OBJDIR)/std_sys.o \
	$(OBJDIR)/std_serialize.o \
	$(OBJDIR)/debug_lib.o \
	$(OBJDIR)/linux_comm.o \

clean:
	-@rm -rf $(OBJDIR)
	-@mkdir $(OBJDIR)
	-@rm -f wrench
	-@rm -f wrench_dev
	-@rm -f wrench_v
	-@rm -rf src
	-@mkdir src

valgrind: $(OBJS) discrete_src/utils/wrench_cli.cpp discrete_src/utils/debugger.cpp
	g++ -o wrench_v $(FLAGS) -Wall -Werror -I. -Idiscrete_src -O3 -ggdb $(OBJS) discrete_src/utils/wrench_cli.cpp discrete_src/utils/debugger.cpp

test: $(OBJS) discrete_src/utils/wrench_cli.cpp discrete_src/utils/debugger.cpp
	g++ $(OBJS) -Wall -Werror discrete_src/utils/debugger.cpp discrete_src/utils/wrench_cli.cpp $(FLAGS) -Idiscrete_src -Isrc -o wrench_dev
	./wrench_dev t

wrench_dev: dev_wrench

dev_wrench: $(OBJS) discrete_src/utils/wrench_cli.cpp discrete_src/utils/debugger.cpp
	g++ $(OBJS) -Wall -Werror discrete_src/utils/wrench_cli.cpp discrete_src/utils/debugger.cpp $(FLAGS) -Idiscrete_src -Isrc -o wrench_dev

wrench: $(OBJS) discrete_src/utils/wrench_cli.cpp
	g++ $(OBJS) -Wall -Werror discrete_src/utils/debugger.cpp discrete_src/utils/wrench_cli.cpp $(FLAGS) -Idiscrete_src -Isrc -o wrench_dev
	./wrench_dev release discrete_src src/.
	-@rm wrench_dev
	g++ -o wrench -Wall -Werror $(FLAGS) -Isrc src/wrench.cpp discrete_src/utils/wrench_cli.cpp discrete_src/utils/debugger.cpp

$(OBJDIR)/cc.o: discrete_src/cc/cc.cpp
	$(CC) $@ $<

$(OBJDIR)/optimizer.o: discrete_src/cc/optimizer.cpp
	$(CC) $@ $<

$(OBJDIR)/token.o: discrete_src/cc/token.cpp
	$(CC) $@ $<

$(OBJDIR)/expression.o: discrete_src/cc/expression.cpp
	$(CC) $@ $<

$(OBJDIR)/link.o: discrete_src/cc/link.cpp
	$(CC) $@ $<

$(OBJDIR)/operations.o: discrete_src/vm/operations.cpp
	$(CC) $@ $<

$(OBJDIR)/index.o: discrete_src/vm/index.cpp
	$(CC) $@ $<

$(OBJDIR)/gc.o: discrete_src/vm/gc.cpp
	$(CC) $@ $<

$(OBJDIR)/vm.o: discrete_src/vm/vm.cpp
	$(CC) $@ $<

$(OBJDIR)/utils.o: discrete_src/utils/utils.cpp
	$(CC) $@ $<

$(OBJDIR)/serializer.o: discrete_src/utils/serializer.cpp
	$(CC) $@ $<

$(OBJDIR)/std_serialize.o: discrete_src/lib/std_serialize.cpp
	$(CC) $@ $<

$(OBJDIR)/debug_lib.o: discrete_src/lib/debug_lib.cpp
	$(CC) $@ $<

$(OBJDIR)/disassemble.o: discrete_src/debug/disassemble.cpp
	$(CC) $@ $<

$(OBJDIR)/debug.o: discrete_src/debug/debug.cpp
	$(CC) $@ $<

$(OBJDIR)/server_debug.o: discrete_src/debug/server_debug.cpp
	$(CC) $@ $<

$(OBJDIR)/packet.o: discrete_src/debug/packet.cpp
	$(CC) $@ $<

$(OBJDIR)/server_interface_private.o: discrete_src/debug/server_interface_private.cpp
	$(CC) $@ $<

$(OBJDIR)/client_debug.o: discrete_src/debug/client_debug.cpp
	$(CC) $@ $<

$(OBJDIR)/scheduler.o: discrete_src/vm/scheduler.cpp
	$(CC) $@ $<

$(OBJDIR)/std.o: discrete_src/lib/std.cpp
	$(CC) $@ $<

$(OBJDIR)/std_io.o: discrete_src/lib/std_io.cpp
	$(CC) $@ $<

$(OBJDIR)/std_io_linux.o: discrete_src/lib/std_io_linux.cpp
	$(CC) $@ $<

$(OBJDIR)/std_string.o: discrete_src/lib/std_string.cpp
	$(CC) $@ $<

$(OBJDIR)/std_math.o: discrete_src/lib/std_math.cpp
	$(CC) $@ $<

$(OBJDIR)/std_msg.o: discrete_src/lib/std_msg.cpp
	$(CC) $@ $<

$(OBJDIR)/std_sys.o: discrete_src/lib/std_sys.cpp
	$(CC) $@ $<

$(OBJDIR)/linux_comm.o: discrete_src/utils/linux_comm.cpp
	$(CC) $@ $<

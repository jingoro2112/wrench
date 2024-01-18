OPT ?= -O3
#OPT ?= -O0 -ggdb

PERF ?=

#FLAGS ?= $(OPT) $(PERF) -std=c++11
FLAGS ?= $(OPT) $(PERF) -I. $(COMPACT) -std=c++98 -MD -DWRENCH_LINUX_FILE_IO

#FLAGS = $(OPT) -pg
#FLAGS = $(OPT) $(PERF)

all: wrench valgrind

OBJDIR = objs_linux

-include $(OBJDIR)/*.d

CC = g++ -MD -Wall -Werror $(FLAGS) -c -o
#CC = g++ -MD $(FLAGS) -c -o

OBJS = \
	$(OBJDIR)/cc.o \
	$(OBJDIR)/operations.o \
	$(OBJDIR)/index.o \
	$(OBJDIR)/vm.o \
	$(OBJDIR)/utils.o \
	$(OBJDIR)/serializer.o \
	$(OBJDIR)/wrench_server_debug.o \
	$(OBJDIR)/wrench_client_debug.o \
	$(OBJDIR)/std.o \
	$(OBJDIR)/std_io.o \
	$(OBJDIR)/std_io_linux.o \
	$(OBJDIR)/std_string.o \
	$(OBJDIR)/std_math.o \
	$(OBJDIR)/std_msg.o \
	$(OBJDIR)/std_sys.o \
	$(OBJDIR)/std_serialize.o \

clean:
	-@rm -rf $(OBJDIR)
	-@mkdir $(OBJDIR)
	-@rm -f wrench
	-@rm -f wrench_v
	-@rm -rf src
	-@mkdir src

valgrind: $(OBJS) wrench_cli.cpp
	g++ -o wrench_v $(FLAGS) -Wall -Werror -I. -Idiscrete_src -O3 -ggdb $(OBJS) wrench_cli.cpp 

test: $(OBJS) wrench_cli.cpp
	g++ $(OBJS) -Wall -Werror wrench_cli.cpp $(FLAGS) -Idiscrete_src -Isrc -o wrench_cli
	./wrench_cli t

wrench_dev: dev_wrench

dev_wrench: $(OBJS) wrench_cli.cpp
	g++ $(OBJS) -Wall -Werror wrench_cli.cpp $(FLAGS) -Idiscrete_src -Isrc -o wrench_cli

wrench: $(OBJS) wrench_cli.cpp
	g++ $(OBJS) -Wall -Werror wrench_cli.cpp $(FLAGS) -Idiscrete_src -Isrc -o wrench_cli
	./wrench_cli release discrete_src src/.
	-@rm wrench_cli
	g++ -o wrench -Wall -Werror $(FLAGS) -Isrc src/wrench.cpp wrench_cli.cpp

$(OBJDIR)/cc.o: discrete_src/cc.cpp
	$(CC) $@ $<

$(OBJDIR)/operations.o: discrete_src/operations.cpp
	$(CC) $@ $<

$(OBJDIR)/index.o: discrete_src/index.cpp
	$(CC) $@ $<

$(OBJDIR)/vm.o: discrete_src/vm.cpp
	$(CC) $@ $<

$(OBJDIR)/utils.o: discrete_src/utils.cpp
	$(CC) $@ $<

$(OBJDIR)/serializer.o: discrete_src/serializer.cpp
	$(CC) $@ $<

$(OBJDIR)/std_serialize.o: discrete_src/std_serialize.cpp
	$(CC) $@ $<

$(OBJDIR)/wrench_server_debug.o: discrete_src/wrench_server_debug.cpp
	$(CC) $@ $<

$(OBJDIR)/wrench_client_debug.o: discrete_src/wrench_client_debug.cpp
	$(CC) $@ $<

$(OBJDIR)/std.o: discrete_src/std.cpp
	$(CC) $@ $<

$(OBJDIR)/std_io.o: discrete_src/std_io.cpp
	$(CC) $@ $<

$(OBJDIR)/std_io_linux.o: discrete_src/std_io_linux.cpp
	$(CC) $@ $<

$(OBJDIR)/std_string.o: discrete_src/std_string.cpp
	$(CC) $@ $<

$(OBJDIR)/std_math.o: discrete_src/std_math.cpp
	$(CC) $@ $<

$(OBJDIR)/std_msg.o: discrete_src/std_msg.cpp
	$(CC) $@ $<

$(OBJDIR)/std_sys.o: discrete_src/std_sys.cpp
	$(CC) $@ $<

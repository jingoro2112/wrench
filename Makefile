OPT ?= -O3
#OPT ?= -O0 -ggdb

PERF ?=

#FLAGS ?= $(OPT) $(PERF) -std=c++11
FLAGS ?= $(OPT) $(PERF) -I. $(COMPACT)

#FLAGS = $(OPT) -pg
#FLAGS = $(OPT) $(PERF)

all: wrench valgrind

OBJDIR = objs_linux

-include $(OBJDIR)/*.d

CC = g++  -MD -Wall -Werror $(FLAGS) -c -o
#CC = g++ -MD $(FLAGS) -c -o

OBJS = \
	$(OBJDIR)/cc.o \
	$(OBJDIR)/operations.o \
	$(OBJDIR)/vm.o \
	$(OBJDIR)/std.o \
	$(OBJDIR)/std_io.o \
	$(OBJDIR)/std_string.o \
	$(OBJDIR)/std_math.o \

clean:
	-@rm -rf $(OBJDIR)
	-@mkdir $(OBJDIR)
	-@rm -f wrench
	-@rm -f wrench_v
	-@rm -rf src
	-@mkdir src

valgrind: $(OBJS) wrench_cli.cpp
	g++ -o wrench_v $(FLAGS) -Wall -Werror -I. -Idiscrete_src -O3 -ggdb $(OBJS) wrench_cli.cpp 

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

$(OBJDIR)/vm.o: discrete_src/vm.cpp
	$(CC) $@ $<

$(OBJDIR)/std.o: discrete_src/std.cpp
	$(CC) $@ $<

$(OBJDIR)/std_io.o: discrete_src/std_io.cpp
	$(CC) $@ $<

$(OBJDIR)/std_string.o: discrete_src/std_string.cpp
	$(CC) $@ $<

$(OBJDIR)/std_math.o: discrete_src/std_math.cpp
	$(CC) $@ $<

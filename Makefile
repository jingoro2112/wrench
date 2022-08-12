OPT ?= -O0 -ggdb
PERF ?=

#FLAGS ?= $(OPT) $(PERF) -std=c++11
FLAGS ?= $(OPT) $(PERF) -I.

#FLAGS = -O3 -pg
#FLAGS = -O0 -ggdb $(PERF)

all: wrench

OBJDIR = objs_linux

-include $(OBJDIR)/*.d

CC = g++  -MD -Wall -Werror $(FLAGS) -c -o
#CC = g++ -MD $(FLAGS) -c -o

OBJS = \
	$(OBJDIR)/cc.o \
	$(OBJDIR)/operations.o \
	$(OBJDIR)/vm.o \

clean:
	-@rm -rf $(OBJDIR)
	-@mkdir $(OBJDIR)
	-@rm -f wrench
	-@rm -f wrench_v
	-@rm -rf src
	-@mkdir src

valgrind: $(OBJS) wrench_cli.cpp
	g++ $(OBJS) -Wall -Werror wrench_cli.cpp $(FLAGS) -ggdb -Idiscrete_src -o wrench_v

wrench: $(OBJS) wrench_cli.cpp
	g++ $(OBJS) -Wall -Werror wrench_cli.cpp $(FLAGS) -Idiscrete_src -Isrc -o wrench_cli
	./wrench_cli release discrete_src src/.
	-@rm wrench_cli
	g++ -o wrench -Wall -Werror $(FLAGS) -Isrc src/wrench.cpp wrench_cli.cpp
	g++ -o wrench_min -Wall -Werror -DWRENCH_WITHOUT_COMPILER $(FLAGS) -Isrc src/wrench.cpp wrench_min.cpp
	g++ -o wrench_v -Wall -Werror -I. -Idiscrete_src -O0 -ggdb $(OBJS) wrench_cli.cpp 

$(OBJDIR)/cc.o: discrete_src/cc.cpp
	$(CC) $@ $<

$(OBJDIR)/operations.o: discrete_src/operations.cpp
	$(CC) $@ $<

$(OBJDIR)/vm.o: discrete_src/vm.cpp
	$(CC) $@ $<



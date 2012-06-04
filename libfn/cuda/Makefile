NVCC:=/usr/local/cuda/bin/nvcc
CC:=/usr/bin/g++
LIBTOOL:=/usr/bin/libtool
CXXFLAGS:=-I../libfn/include \
	-I./include \
	-I/opt/local/include \
	-I/usr/local/cuda/include
LDFLAGS:=-L/usr/local/cuda/lib
LIBS:=-lcudart
OBJDIR:=./bin
VPATH:=../libfn/src/hmm:./src:./test
LIBFILES:=\
	../libfn/src/hmm/deterministic_node.cpp \
	../libfn/src/hmm/graphviz.cpp \
	../libfn/src/hmm/hmm_network.cpp \
	../libfn/src/hmm/hmm_node.cpp \
	../libfn/src/hmm/hmm.cpp \
	../libfn/src/hmm/probabilistic_node.cpp \
	src/gpu_network.cpp

CUDAFILES:=\
	src/gpu_update.cu

OBJS:=$(addprefix $(OBJDIR)/, \
	$(patsubst %.cpp,%.o,$(notdir $(LIBFILES))) \
	$(patsubst %.cu,%.o,$(notdir $(CUDAFILES))))

DEPS:=$(wildcard ./libfn/include/**.h)

LIBNAME:=$(OBJDIR)/libfncuda.a

all: $(LIBNAME) test

$(LIBNAME): $(OBJS)
	$(LIBTOOL) -o $@ -static $^ 

$(OBJDIR)/%.o : %.cpp $(DEPS)
	$(CC) -c $< -o $@ $(CXXFLAGS)

$(OBJDIR)/%.o : %.cu $(DEPS)
	$(NVCC) -c $< -o $@ $(CXXFLAGS)

$(OBJS): | $(OBJDIR)

$(OBJDIR):
	mkdir $(OBJDIR)

testexec: $(LIBNAME) $(OBJDIR)/test.o
	$(NVCC) -o $@ $(CXXFLAGS) $^ 

test: testexec
	$(if $(shell ./testexec), $(info Tests passed.), $(error Failed))

.PHONY: clean

clean:
	- rm -rf $(LIBNAME) $(OBJDIR) testexec core 

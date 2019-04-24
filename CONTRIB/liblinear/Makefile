CXX ?= g++
CC ?= gcc
CFLAGS = -Wall -Wconversion -O3 -fPIC
LIBS = blas/blas.a
SHVER = 3
OS = $(shell uname)
AR = ar rcs
RANLIB = ranlib 
#LIBS = -lblas

all: train predict

lib: linear.o tron.o blas/blas.a 
	if [ "$(OS)" = "Darwin" ]; then \
		SHARED_LIB_FLAG="-dynamiclib -Wl,-install_name,liblinear.so.$(SHVER)"; \
	else \
		SHARED_LIB_FLAG="-shared -Wl,-soname,liblinear.so.$(SHVER)"; \
	fi; \
	$(CXX) $${SHARED_LIB_FLAG} linear.o tron.o blas/blas.a -o liblinear.so.$(SHVER)
	$(AR) liblinear.a linear.o tron.o blas/daxpy.o blas/ddot.o blas/dnrm2.o blas/dscal.o
	$(RANLIB) liblinear.a 
	ln -fs liblinear.so.$(SHVER) liblinear.so

train: tron.o linear.o train.c blas/blas.a
	$(CXX) -static $(CFLAGS) -o train train.c tron.o linear.o $(LIBS)

predict: tron.o linear.o predict.c blas/blas.a
	$(CXX) -static $(CFLAGS) -o predict predict.c tron.o linear.o $(LIBS)

tron.o: tron.cpp tron.h
	$(CXX) $(CFLAGS) -c -o tron.o tron.cpp

linear.o: linear.cpp linear.h
	$(CXX) $(CFLAGS) -c -o linear.o linear.cpp

blas/blas.a: blas/*.c blas/*.h
	make -C blas OPTFLAGS='$(CFLAGS)' CC='$(CC)';

clean:
	make -C blas clean
	rm -f *~ tron.o linear.o train predict liblinear.so.$(SHVER) liblinear.so liblinear.a

CC = gcc
CXXFLAGS = -g -fPIC -Wall
CPPFLAGS = -D_GNU_SOURCE
INCLUDE = -I.
FILES = process_judger.cpp
OUT_EXE = process_judger

process_judger: $(FILES)
	$(CC) $(CXXFLAGS) $(CPPFLAGS) $(INCLUDE) $(FILES) -o $(OUT_EXE)

clean:
	rm -rf  $(OUT_EXE)

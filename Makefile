CC = gcc
CXXFLAGS = -g -fPIC -Wall
CPPFLAGS = -D_GNU_SOURCE
FILES = process_judger.cpp
OUT_EXE = process_judger

process_judger: $(FILES)
	$(CC) $(CXXFLAGS) $(CPPFLAGS) $(FILES) -o $(OUT_EXE)

clean:
	rm -rf  $(OUT_EXE)

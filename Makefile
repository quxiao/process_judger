CC = gcc
FILES = process_judger.cpp
OUT_EXE = process_judger

process_judger: $(FILES)
	$(CC) -DMY_DEBUG $(FILES) -o $(OUT_EXE)

clean:
	rm -rf  $(OUT_EXE)

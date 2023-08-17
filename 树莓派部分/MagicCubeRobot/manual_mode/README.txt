这个文件中的文件是用于手动调试的.
如果不需要用到此功能, 那就不必编译它, 所以把这个功能设计成了跟主功能分开的形式.
这是一个独立的可执行文件, 它需要独立编译, 因为这个比较简单, 所以没有提供Makefile, 在命令行编译就行.
编译命令为: gcc manual.c ../tucomm/tucomm.c -o manual -lwiringPi -I.. -I../tucomm

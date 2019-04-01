call egcc.bat
windres resource.rc -O coff -o resource.o
gcc %CCFLAGS2% *.cc resource.o -lwin32hlp -lexshit -lstdshit -lcomctl32 -lcomdlg32 -lgdi32 -mwindows -o image-measure -s

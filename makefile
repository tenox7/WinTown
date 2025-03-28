# Simple makefile for MicropolisNT

all: MicropolisNT.exe

main.obj: main.c
	cl.exe /O2 /nologo /c main.c

MicropolisNT.exe: main.obj
	link.exe /NOLOGO /SUBSYSTEM:WINDOWS main.obj user32.lib gdi32.lib comdlg32.lib /OUT:MicropolisNT.exe

clean:
	del *.obj
	del MicropolisNT.exe
	del err.out
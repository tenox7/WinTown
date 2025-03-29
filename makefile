# Simple makefile for MicropolisNT

all: main.obj simulation.obj zone.obj power.obj traffic.obj scanner.obj evaluation.obj budget.obj scenarios.obj disasters.obj
	link.exe /NOLOGO /SUBSYSTEM:WINDOWS main.obj simulation.obj zone.obj power.obj traffic.obj scanner.obj evaluation.obj budget.obj scenarios.obj disasters.obj user32.lib gdi32.lib comdlg32.lib /OUT:MicropolisNT.exe

main.obj: main.c simulation.h
	cl.exe /O2 /nologo /c main.c

simulation.obj: simulation.c simulation.h
	cl.exe /O2 /nologo /c simulation.c

zone.obj: zone.c simulation.h
	cl.exe /O2 /nologo /c zone.c
	
power.obj: power.c simulation.h
	cl.exe /O2 /nologo /c power.c
	
traffic.obj: traffic.c simulation.h
	cl.exe /O2 /nologo /c traffic.c
	
scanner.obj: scanner.c simulation.h
	cl.exe /O2 /nologo /c scanner.c
	
evaluation.obj: evaluation.c simulation.h
	cl.exe /O2 /nologo /c evaluation.c
	
budget.obj: budget.c simulation.h
	cl.exe /O2 /nologo /c budget.c

scenarios.obj: scenarios.c simulation.h
	cl.exe /O2 /nologo /c scenarios.c
	
disasters.obj: disasters.c simulation.h
	cl.exe /O2 /nologo /c disasters.c

clean:
	del *.obj
	del MicropolisNT.exe
	del err.out
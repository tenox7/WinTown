OBJS = src\animatin.obj src\budget.obj src\disaster.obj src\evaluate.obj src\main.obj \
	src\power.obj src\scanner.obj src\scenario.obj src\sim.obj src\tools.obj \
	src\traffic.obj src\zone.obj src\gdifix.obj


CC = cl
CFLAGS = /nologo /G3 /W3 /Zi /YX /D "_X86_" /D "_DEBUG" /D "_WINDOWS" /FR /ML /Fd"SS.PDB" /Fp"SS.PCH"
# CFLAGS = /nologo /W3 /YX /O2 /D "_X86_" /D "NDEBUG" /D "_WINDOWS" /FR /ML /Fp"SS.PCH"
LIBS = gdi32.lib user32.lib kernel32.lib COMDLG32.lib

.c.obj:
        $(CC) $(CFLAGS) /c $*.c /Fo$*.obj

ss.exe: $(OBJS)
	link -out:ss.exe $(OBJS) $(LIBS)



clean:
	del /q $(OBJS)
	del /q ss.exe
	del /q *.sbr
	del /q ss.pch ss.pdb
OBJS = src\anim.obj src\budget.obj src\disastr.obj src\eval.obj src\main.obj \
	src\power.obj src\scanner.obj src\scenario.obj src\sim.obj src\sprite.obj \
	src\tools.obj src\traffic.obj src\zone.obj src\gdifix.obj


CC = cl
CFLAGS = /nologo /G3 /W3 /Zi /YX /D "_X86_" /D "_DEBUG" /D "_WINDOWS" /FR /ML /Fd"ntpolis.PDB" /Fp"ntpolis.PCH"
# CFLAGS = /nologo /W3 /YX /O2 /D "_X86_" /D "NDEBUG" /D "_WINDOWS" /FR /ML /Fp"ntpolis.PCH"
LIBS = gdi32.lib user32.lib kernel32.lib COMDLG32.lib

.c.obj:
        $(CC) $(CFLAGS) /c $*.c /Fo$*.obj

ntpolis.exe: $(OBJS)
	link -out:ntpolis.exe $(OBJS) $(LIBS)



clean:
	del /q $(OBJS)
	del /q ntpolis.exe
	del /q *.sbr
	del /q ntpolis.PCH ntpolis.PDB

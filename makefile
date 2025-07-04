OBJS = src\anim.obj src\budget.obj src\charts.obj src\disastr.obj src\eval.obj src\main.obj \
	src\power.obj src\scanner.obj src\scenario.obj src\sim.obj src\sprite.obj \
	src\tiles.obj src\tools.obj src\traffic.obj src\zone.obj src\gdifix.obj budget.res


CC = cl
RC = rc
CFLAGS = /nologo /W3 /O2 /D "_X86_" /D "NDEBUG" /D "_WINDOWS" /ML
DEBUGFLAGS = /nologo /G3 /W3 /Zi /YX /D "_X86_" /D "_DEBUG" /D "_WINDOWS" /FR /ML /Fd"ntpolis.PDB" /Fp"ntpolis.PCH"
LIBS = gdi32.lib user32.lib kernel32.lib COMDLG32.lib

.c.obj:
        $(CC) $(CFLAGS) /c $*.c /Fo$*.obj

budget.res: budget.rc
	$(RC) budget.rc

all: ntpolis.exe

ntpolis.exe: $(OBJS)
	link /NOLOGO /OUT:ntpolis.exe $(OBJS) $(LIBS)

debug: clean
	nmake CFLAGS="$(DEBUGFLAGS)" ntpolis.exe



clean:
	del /q $(OBJS)
	del /q ntpolis.exe
	del /q *.sbr
	del /q ntpolis.PCH ntpolis.PDB
	del /q budget.res

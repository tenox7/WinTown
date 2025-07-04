CC = cl
RC = rc
CFLAGS = /nologo /W3 /O2 /D "_X86_" /D "NDEBUG" /D "_WINDOWS" /ML
LIBS = gdi32.lib user32.lib kernel32.lib COMDLG32.lib

all: ntpolis.exe

# Compile all source files
src\anim.obj: src\anim.c
	$(CC) $(CFLAGS) /c src\anim.c /Fosrc\anim.obj

src\budget.obj: src\budget.c
	$(CC) $(CFLAGS) /c src\budget.c /Fosrc\budget.obj

src\charts.obj: src\charts.c
	$(CC) $(CFLAGS) /c src\charts.c /Fosrc\charts.obj

src\disastr.obj: src\disastr.c
	$(CC) $(CFLAGS) /c src\disastr.c /Fosrc\disastr.obj

src\eval.obj: src\eval.c
	$(CC) $(CFLAGS) /c src\eval.c /Fosrc\eval.obj

src\main.obj: src\main.c
	$(CC) $(CFLAGS) /c src\main.c /Fosrc\main.obj

src\power.obj: src\power.c
	$(CC) $(CFLAGS) /c src\power.c /Fosrc\power.obj

src\scanner.obj: src\scanner.c
	$(CC) $(CFLAGS) /c src\scanner.c /Fosrc\scanner.obj

src\scenario.obj: src\scenario.c
	$(CC) $(CFLAGS) /c src\scenario.c /Fosrc\scenario.obj

src\sim.obj: src\sim.c
	$(CC) $(CFLAGS) /c src\sim.c /Fosrc\sim.obj

src\sprite.obj: src\sprite.c
	$(CC) $(CFLAGS) /c src\sprite.c /Fosrc\sprite.obj

src\tiles.obj: src\tiles.c
	$(CC) $(CFLAGS) /c src\tiles.c /Fosrc\tiles.obj

src\tools.obj: src\tools.c
	$(CC) $(CFLAGS) /c src\tools.c /Fosrc\tools.obj

src\traffic.obj: src\traffic.c
	$(CC) $(CFLAGS) /c src\traffic.c /Fosrc\traffic.obj

src\zone.obj: src\zone.c
	$(CC) $(CFLAGS) /c src\zone.c /Fosrc\zone.obj

src\gdifix.obj: src\gdifix.c
	$(CC) $(CFLAGS) /c src\gdifix.c /Fosrc\gdifix.obj

budget.res: budget.rc
	$(RC) budget.rc

ntpolis.exe: src\anim.obj src\budget.obj src\charts.obj src\disastr.obj src\eval.obj src\main.obj src\power.obj src\scanner.obj src\scenario.obj src\sim.obj src\sprite.obj src\tiles.obj src\tools.obj src\traffic.obj src\zone.obj src\gdifix.obj budget.res
	link /NOLOGO /OUT:ntpolis.exe src\anim.obj src\budget.obj src\charts.obj src\disastr.obj src\eval.obj src\main.obj src\power.obj src\scanner.obj src\scenario.obj src\sim.obj src\sprite.obj src\tiles.obj src\tools.obj src\traffic.obj src\zone.obj src\gdifix.obj budget.res $(LIBS)

clean:
	del /q src\*.obj
	del /q ntpolis.exe
	del /q *.res

debug: clean
	nmake CFLAGS="$(DEBUGFLAGS)" ntpolis.exe

CC = cl
RC = rc
CFLAGS = /nologo /W3 /Ox /Ot /Oi /Ob2 /D "_WINDOWS" /ML /I.
#CFLAGS = /nologo /W3 /Ox /Ot /Oi /Ob2 /D "_WINDOWS" /D "_CRT_SECURE_NO_WARNINGS" /I.
#CFLAGS = /nologo -D_AXP64_=1 -D_ALPHA64_=1 -DALPHA=1 -DWIN64 -D_WIN64 -DWIN32 -D_WIN32  -Wp64 -W4 -Ap64
LIBS = gdi32.lib user32.lib kernel32.lib COMDLG32.lib

all: wintown.exe

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

src\notify.obj: src\notify.c
	$(CC) $(CFLAGS) /c src\notify.c /Fosrc\notify.obj

src\animtab.obj: src\animtab.c
	$(CC) $(CFLAGS) /c src\animtab.c /Fosrc\animtab.obj

src\newgame.obj: src\newgame.c
	$(CC) $(CFLAGS) /c src\newgame.c /Fosrc\newgame.obj

src\mapgen.obj: src\mapgen.c
	$(CC) $(CFLAGS) /c src\mapgen.c /Fosrc\mapgen.obj

src\assets.obj: src\assets.c
	$(CC) $(CFLAGS) /c src\assets.c /Fosrc\assets.obj

wintown.res: wintown.rc
	$(RC) /i. wintown.rc

wintown.exe: src\anim.obj src\budget.obj src\charts.obj src\disastr.obj src\eval.obj src\main.obj src\power.obj src\scanner.obj src\scenario.obj src\sim.obj src\sprite.obj src\tiles.obj src\tools.obj src\traffic.obj src\zone.obj src\gdifix.obj src\notify.obj src\animtab.obj src\newgame.obj src\mapgen.obj src\assets.obj wintown.res
	link /NOLOGO /OUT:wintown.exe src\anim.obj src\budget.obj src\charts.obj src\disastr.obj src\eval.obj src\main.obj src\power.obj src\scanner.obj src\scenario.obj src\sim.obj src\sprite.obj src\tiles.obj src\tools.obj src\traffic.obj src\zone.obj src\gdifix.obj src\notify.obj src\animtab.obj src\newgame.obj src\mapgen.obj src\assets.obj wintown.res $(LIBS)

clean:
	del /q src\*.obj
	del /q wintown.exe
	del /q *.res

debug: clean
	nmake CFLAGS="$(DEBUGFLAGS)" wintown.exe

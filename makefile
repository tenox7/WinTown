CC = cl
RC = rc
CFLAGS = /nologo /O2 /Ot /Oi
LIBS = gdi32.lib user32.lib kernel32.lib COMDLG32.lib

OBJS = anim.obj animtab.obj assets.obj budget.obj charts.obj disastr.obj eval.obj gdifix.obj main.obj mapgen.obj newgame.obj notify.obj power.obj scanner.obj scenario.obj sim.obj sprite.obj tiles.obj tools.obj traffic.obj zone.obj

all: wintown.exe

.c.obj:
	$(CC) $(CFLAGS) /c $<

wintown.res: wintown.rc
	$(RC) wintown.rc

wintown.exe: $(OBJS) wintown.res
	link /NOLOGO /OUT:wintown.exe $(OBJS) wintown.res $(LIBS)
        del /q *.obj *.res

clean:
	del /q *.obj *.res wintown.exe

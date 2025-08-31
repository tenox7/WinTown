CC = cl
RC = rc
CFLAGS = /nologo /O2 /Ot /Oi /Z7 /W3
#CFLAGS = -D_AXP64_=1 -D_ALPHA64_=1 -DALPHA=1 -DWIN64 -D_WIN64 -DWIN32 -D_WIN32  -Wp64 -W4 -Ap64
LIBS = gdi32.lib user32.lib kernel32.lib COMDLG32.lib

OBJS = anim.obj animtab.obj assets.obj budget.obj charts.obj disastr.obj eval.obj gdifix.obj main.obj mapgen.obj newgame.obj notify.obj power.obj scanner.obj scenario.obj sim.obj sprite.obj tiles.obj tools.obj traffic.obj zone.obj

all: wintown.exe

.c.obj:
	$(CC) $(CFLAGS) /c $<

wintown.res: wintown.rc
	$(RC) wintown.rc

wintown.exe: $(OBJS) wintown.res
	link /NOLOGO /DEBUG /OUT:wintown.exe $(OBJS) wintown.res $(LIBS)
        del /q *.obj *.res

clean:
	del /q *.obj *.res *.pdb *.ilk wintown.exe

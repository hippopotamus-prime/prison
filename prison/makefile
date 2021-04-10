ofiles = prison.o game.o boni.o sound.o bricks.o guys.o highscores.o handlers.o prefs.o

all: opts = -Wall -Werror -O2
all: prcname = prison
all: prison.prc clean
	ls -l *.prc

debug: opts = -Wall -Werror -g -mdebug-labels
debug: prcname = prison
debug: prison.prc clean
	ls -l *.prc

cheat: opts = -Wall -Werror -O2 -DCHEAT
cheat: prcname = prison
cheat: prison.prc clean
	ls -l *.prc

prison.prc: grc bin
	build-prc $(prcname).prc "Prison" Prsn *.bin *.grc $(buildopts)

bin: rc.rcp rc.h gfx/*.bmp
	pilrc $(pilrcopts) rc.rcp

grc: $(ofiles)
	m68k-palmos-gcc $(opts) -o prison $(ofiles)
	m68k-palmos-obj-res prison

prison.o: prison.c rc.h guys.h boni.h sound.h bricks.h game.h highscores.h handlers.h prefs.h
	m68k-palmos-gcc $(opts) -c prison.c

game.o: game.c rc.h boni.h sound.h bricks.h guys.h highscores.h game.h prefs.h
	m68k-palmos-gcc $(opts) -c game.c

boni.o: boni.c rc.h boni.h sound.h guys.h game.h bricks.h
	m68k-palmos-gcc $(opts) -c boni.c

sound.o: sound.c game.h sound.h rc.h
	m68k-palmos-gcc $(opts) -c sound.c

bricks.o: bricks.c bricks.h game.h rc.h boni.h sound.h guys.h
	m68k-palmos-gcc $(opts) -c bricks.c

guys.o: guys.c game.h guys.h rc.h sound.h
	m68k-palmos-gcc $(opts) -c guys.c

highscores.o: highscores.c highscores.h game.h
	m68k-palmos-gcc $(opts) -c highscores.c

handlers.o: handlers.c handlers.h game.h rc.h sound.h prefs.h guys.h
	m68k-palmos-gcc $(opts) -c handlers.c

prefs.o: prefs.c prefs.h game.h sound.h
	m68k-palmos-gcc $(opts) -c prefs.c

clean:
	rm -f *.bin *.grc prison

veryclean:
	rm -f *.bin *.grc prison *.o


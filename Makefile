TARGET = 2player_games
OBJS = main.o

BUILD_PRX = 1
PSP_FW_VERSION = 500

CFLAGS = -O2 -G0 -Wall
CXXFLAGS = -O2 -G0 -Wall -std=c++11 -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

INCDIR = $(PSPDEV)/psp/include/SDL2

LIBDIR =
LDFLAGS =
LIBS = -lSDL2main -lSDL2 -lGL -lpspvram -lpspaudio -lpspvfpu -lpspgu -lpspge -lpsphprm -lpspdisplay -lpspctrl -lpsppower -lpspdebug -lstdc++ -lm

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = 2 Player Games: The Challenge
PSP_EBOOT_ICON = ICON0.PNG
PSP_EBOOT_PIC1 = PIC1.PNG

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

package: EBOOT.PBP
	mkdir -p PSP/GAME/2PlayerGames
	cp EBOOT.PBP PSP/GAME/2PlayerGames/
	zip -r 2PlayerGames_PSP.zip PSP/

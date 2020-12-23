EXE      := pavu
CC       := g++ -std=c++11
CPPFLAGS := -I -MMD -MP
CFLAGS   := -Wall -g
LINKER   := -lpulse -lcurses
SRC      := $(wildcard *.cpp)
OBJ      := $(SRC:.cpp=.o)

$(EXE): $(OBJ)
	$(CC) $^ -o $@ $(LINKER)

%.o: %.cpp config.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@$(RM) -rv $(EXE) *.o

-include $(OBJ:.o=.d)

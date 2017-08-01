INCLUDES=-Iinclude/
CFLAGS=$(INCLUDES) -Wall -Werror -pedantic

OBJ=objs/vm.o
OUTPUT=aurora-soft

.PHONY: debug
default: $(OUTPUT)
debug: CFLAGS += -g -O0 -D__DEBUG
debug: $(OUTPUT)

################################################################################
#                                SOURCES STUFF                                 #
################################################################################

objs/%.o: src/%.c
	@mkdir -p objs/
	$(CC) -c -o $@ $< $(CFLAGS) $(EXTRA)

$(OUTPUT): $(OBJ)
	$(CC) $< -o $@ $(CFLAGS)

.PHONY: clean
clean:
	-rm -f $(OBJ)
	-rm -f $(OUTPUT)

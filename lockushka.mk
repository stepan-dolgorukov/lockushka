COMPILER_C := gcc

lockushka: lockushka.o
	"${COMPILER_C}" "${^}" -o "${@}"

lockushka.o: lockushka.c
	"${COMPILER_C}" -g3 -c "${<}"

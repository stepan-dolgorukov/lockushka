COMPILER_C := gcc
FLAGS_COMPILER_C := -Og -g3

lockushka: lockushka.o
	"${COMPILER_C}" "${^}" -o "${@}"

lockushka.o: lockushka.c
	"${COMPILER_C}" ${FLAGS_COMPILER_C} -c "${<}"

clang src/main.c -o obj/main.o -c -g0 -Wall -Wextra -pedantic -Weverything
clang src/label.c -o obj/label.o -c -g0 -Wall -Wextra -pedantic -Weverything
clang src/state.c -o obj/state.o -c -g0 -Wall -Wextra -pedantic -Weverything
clang src/token.c -o obj/token.o -c -g0 -Wall -Wextra -pedantic -Weverything
clang src/parser.c -o obj/parser.o -c -g0 -Wall -Wextra -pedantic -Weverything

clang obj/main.o obj/label.o obj/state.o obj/token.o obj/parser.o -o bin/ar

./bin/ar test.arbor test.s

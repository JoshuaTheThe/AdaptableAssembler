clang src/main.c -o obj/main.o -c -Wall -Wextra -pedantic
clang src/label.c -o obj/label.o -c -Wall -Wextra -pedantic
clang src/state.c -o obj/state.o -c -Wall -Wextra -pedantic
clang src/token.c -o obj/token.o -c -Wall -Wextra -pedantic
clang src/parser.c -o obj/parser.o -c -Wall -Wextra -pedantic
clang src/codegen.c -o obj/codegen.o -c -Wall -Wextra -pedantic
#zig cc -c src/codegen.zig -o obj/codegen.o -I./src/

clang obj/main.o obj/label.o obj/state.o obj/token.o obj/parser.o obj/codegen.o -o bin/ar
clang stdlib/stdlib.c -o stdlib/stdlib.o -c -Wall -Wextra -pedantic -m32

./bin/ar test.arbor test.s
nasm test.s -o test.o -felf32
cc test.o stdlib/stdlib.o -o test -m32
./test
echo "Result was: $?"
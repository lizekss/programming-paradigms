## Compile
```sh
make
```

## Testing
The code is tested with a total of 7 tests (t1 through t7).
When running each test, the `-w` tag can be used to control how many threads are used in the testing.
For example, to run the first test with an environment of 4 threads, use:
```sh
./bankdriver -t1 -w4
```
Use the `-b` tag to turn on balance checking:
```sh
./bankdriver -t7 -w4 -b
```
Use the `-y` tag to control the probability with which the threads die, e.g. `-y50` means a 50% chance:
```sh
./bankdriver -t3 -w2 -y50
```
You can use `valgrind --tool=helgrind` to check your work for race conditions and other concurrency related issues
```sh
valgrind --tool=helgrind ./bankdriver -r -t2 -w4
```

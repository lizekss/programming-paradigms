## Install valgrind 
### Ubuntu
```sh
sudo apt-get install valgrind
```
### Arch
```sh
yay -S valgrind
```

## Compile
```sh
make
```

## Run single test
```sh
./rsg data/bionic.g
```

## Memory checks
```sh
./rsgChecker32 ./rsg data/bionic.g
./rsgChecker64 ./rsg data/bionic.g
```

## Run all tests
```sh
make test_all
```

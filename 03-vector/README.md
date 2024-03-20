## დავალების ატვირთვა
დავალება უნდა ატვირთოთ თქვენს პერსონალურ Github Classroom-ის რეპოზიტორიაში.
## Compile
```sh
make
```

## Data files required for testing
Running `make` for the first time automatically creates the `data` directory with required test files.
If necessary, the files can be re-donwloaded the following way:
```sh
rm -rf data/
make data
```

## Testing
```sh
./vector-test
./hashset-test
```

`vector-test` and `hashset-test` applications output how your vector and hashset implementations behave,
which can be compared to `sample-output-vector.txt` და `sample-output-hashset.txt` files to ensure correctness of your implementation.

```sh
./vector-test | diff sample-output-vector.txt -
./hashset-test | diff sample-output-hashset.txt -
```

## Thesaurus
The thesaurus-lookup.c and streaktokenizer.c files, when
compiled against fully operational versions of vector and hashset,
contribute to an application called thesaurus-lookup, which is
this C-string intense application that loads a thesaurus into
a hashset (where synonym sets are stored in vectors).  You don't
need to do anything for this extra thesaurus-lookup application.
I just decided to include it with the starter files so you have
a much more sophisticated test application to exercise your implementations.
It also provides a good amount of sample code for you to read over
while working on next week's Assignment 4, which requires you
use the hashset and the vector to build an index of hundreds of online
news articles (with real networking!)

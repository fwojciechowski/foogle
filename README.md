foogle - wyszukiwarka
=====================

Krótki opis
-----------

Działanie programu polega na wykorzystaniu gotowej biblioteki plików tekstowych i wyszukaniu w niej podanej frazy.

Kompilacja
----------

Program kompilowany był z pomocą CMake w wersji 3.5.1.

```bash
$ cmake .
$ make
```

Przygotowanie biblioteki (opcjonalnie)
--------------------------------------

Do projektu dołączony jest skrypt prepare_biblioteka.sh. Jego działanie polega na ściągnięciu z polskiej Wikipedii ok. 1000 artykułów, których tytuły podane są w pliku mock_data.txt. Artykuły są pozbawiane tagów HTML. Wystarczy uruchomić podany skrypt.

```bash
$ ./prepare_biblioteka.sh
```

Jego wykonanie może zająć parę minut. Po ukończeniu pojawi się ponad 20MB przykładowych informacji do przeszukania.

Uruchamianie
------------

Do działania program potrzebuje ścieżki do biblioteki z plikami tekstowymi oraz frazy do poszukiwania. Przykładowo może wyglądać to tak:

```bash
$ ./foogle ./biblioteka pomidor
```
foogle - wyszukiwarka
=====================

Krótki opis
-----------

Działanie programu polega na wykorzystaniu gotowej biblioteki plików tekstowych i wyszukaniu w niej podanej frazy.

Kompilacja
----------

Program kompilowany był z pomocą CMake w wersji 3.5.1 na Linux Mint 18.

```bash
$ cmake .
$ make
```

Przygotowanie biblioteki (opcjonalnie)
--------------------------------------

Do projektu dołączony jest skrypt prepare_biblioteka.sh. Jego działanie polega na ściągnięciu z polskiej Wikipedii ok. 3000 artykułów, których tytuły podane są w pliku mock_data.txt. Artykuły są pozbawiane tagów HTML. Wystarczy uruchomić podany skrypt.

Do działania potrzebne jest utility html2text. Przykładowa instalacja w systemie Ubuntu:

```bash
$ sudo apt install html2text
```

Skrypt wystarczy odpalić poleceniem:

```bash
$ ./prepare_biblioteka.sh
```

Jego wykonanie może zająć parę minut. Po ukończeniu pojawi się w kataloku biblioteka ponad 35MB przykładowych informacji do przeszukania.

Uruchamianie
------------

Do działania program potrzebuje ścieżki do biblioteki z plikami tekstowymi oraz frazy do poszukiwania. Przykładowo może wyglądać to tak:

```bash
$ ./foogle ./biblioteka pomidor
```

Część WWW
------------

Do uruchomienia serwera potrzebny jest node.js. Niniejszy program był testowany z wersją v10.5.0.

W katalogu www należy uruchomić polecenie

```bash
$ npm install
```

Po ściągnięciu wymaganych komponentów można uruchomić serwer poleceniem
```bash
$ node app
```

Interfejs powinien byc dostepny pod adresem http://localhost:3000/ .

UWAGA: Do poprawnego działania interfejsu należy sprawdzić skrypt foogle_phrase.sh . Powinien on zawierać poprawną ścieżkę do biblioteki oraz samego programu foogle. 
Repozytorium zakłada kompilację przy użyciu cmake, stąd w innych warunkach ścieżka może być niepoprawna.
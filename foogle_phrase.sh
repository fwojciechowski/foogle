#!/bin/bash

# Nalezy sprawdzic sciezke. Uruchamianie w formacie:
# mpiexec -n <liczba procesow> <sciezka do foogle> <sciezka do biblioteki> <fraza - tutaj argument podawany przez serwer www>
mpiexec -n 4 ../cmake-build-debug/foogle ../biblioteka $1


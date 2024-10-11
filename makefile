all: utilitario1 utilitario2 utilitario3 simulador

utilitario1: utilitario1.c
	gcc utilitario1.c -o utilitario1 -luuid

utilitario2: utilitario2.c
	gcc utilitario2.c -o utilitario2 -pthread

utilitario3: utilitario3.c
	gcc utilitario3.c -o utilitario3 -pthread

simulador: simulador.c
	gcc simulador.c -o simulador -pthread

.PHONY: all
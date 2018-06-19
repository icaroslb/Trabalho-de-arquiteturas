//Esquema funcionando
#include <stdio.h>
int main(int argc, char const *argv[]) {
	int vetor[100];
	for (int i = 0; i < 100; i++) {
		vetor[i] = i;
	}

	FILE *memoria;
	memoria = fopen(argv[1], "wb");
	fwrite(vetor, sizeof(int), sizeof(vetor), memoria);
	fclose(memoria);
	return 0;
}
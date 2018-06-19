#include <stdio.h>
int main(int argc, char const *argv[]) {
	int vetor[100];

	FILE *memoria;
	memoria = fopen(argv[1], "rb");
	fread(vetor, sizeof(int), sizeof(vetor), memoria);
	fclose(memoria);

	for (int i = 0; i < 100; i++) {
		printf("%d\n", vetor[i]);
	}
	return 0;
}
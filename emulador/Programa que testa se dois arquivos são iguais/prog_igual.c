#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef unsigned int palavra; 				//32 bits
typedef unsigned char byte;					//8 bits
typedef unsigned long int microinstrucao;	//64 bits

byte memoria1[10000], memoria2[10000], memoria3[10000];
palavra tamanho1, tamanho2, tamanho3;

FILE *prog;
byte tam_arquivo[4];

int main(int argc, char const *argv[]) {
	int iguais = 1;
	switch (argc){
		case 3: {
			prog = fopen(argv[1], "rb");
			if (prog != NULL) {
				fread(memoria1, sizeof(byte), 4, prog);
				memcpy(&tamanho1, memoria1, 4);
				fread(&memoria1[4], sizeof(byte), tamanho1, prog);
				fclose(prog);

			}

			prog = fopen(argv[2], "rb");
			if (prog != NULL) {
				fread(memoria2, sizeof(byte), 4, prog);
				memcpy(&tamanho2, memoria2, 4);
				fread(&memoria2[4], sizeof(byte), tamanho2, prog);
				fclose(prog);

			}

			if (tamanho2 == tamanho1) {
				for (int i = 0; i < tamanho1; i++) {
					if (memoria2[i] != memoria1[i]) {
						iguais = 0;
						printf(" Difere no Byte %d\n", i+1);
						printf("    É   0x%02X   em   %s\n", memoria1[i], argv[1]);
						printf("    É   0x%02X   em   %s\n\n", memoria2[i], argv[2]);
					}
				
				}
				if (iguais) printf("Os arquivos são iguais\n");

			} else printf("Os arquivos são diferentes no tamanho\n");
		} break;

		case 4: {
			prog = fopen(argv[1], "rb");
			if (prog != NULL) {
				fread(memoria1, sizeof(byte), 4, prog);
				memcpy(&tamanho1, memoria1, 4);
				fread(&memoria1[4], sizeof(byte), tamanho1, prog);
				fclose(prog);

			}

			prog = fopen(argv[2], "rb");
			if (prog != NULL) {
				fread(memoria2, sizeof(byte), 4, prog);
				memcpy(&tamanho2, memoria2, 4);
				fread(&memoria2[4], sizeof(byte), tamanho2, prog);
				fclose(prog);

			}			

			prog = fopen(argv[3], "rb");
			if (prog != NULL) {
				fread(memoria3, sizeof(byte), 4, prog);
				memcpy(&tamanho3, memoria3, 4);
				fread(&memoria3[4], sizeof(byte), tamanho3, prog);
				fclose(prog);

			}


			if (tamanho2 == tamanho1 && tamanho1 == tamanho3) {
				for (int i = 0; i < tamanho1; i++) {
					if (memoria2[i] != memoria1[i] || memoria1[i] != memoria3[i]) {
						iguais = 0;
						printf(" Difere no Byte %d\n", i+1);
						printf("    É   0x%02X   em   %s\n", memoria1[i], argv[1]);
						printf("    É   0x%02X   em   %s\n", memoria2[i], argv[2]);
						printf("    É   0x%02X   em   %s\n\n", memoria3[i], argv[3]);
					}
				
				}
				if (iguais) printf("Os arquivos são iguais\n");
				
			} else printf("Os arquivos são diferentes no tamanho\n");
		} break;
	}
	return 0;
}
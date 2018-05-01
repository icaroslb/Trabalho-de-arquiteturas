#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Tipos

typedef unsigned char byte;                 //08 bits
typedef unsigned int word;                  //32 bits
typedef unsigned long int microinstrucao;   //64 bits

// Registradores

word MAR = 0;   //Lê uma word da memória.
word MDR = 0;   //Armazena uma word lida na memória.
word PC =  0;   //Lê um byte da memória.
byte MBR = 0;   //Armazena um byte lido na memória.

word SP = 0, LV = 0, CPP = 0, TOS = 0, OPC = 0, H = 0;  //Realizam operações na ALU.

// Barramentos

word barramentoB, barramentoC;

// Flip-flops de 1 bit para o deslocador.

byte N = 0, Z = 0;

// Microinstrução

microinstrucao MIR; //Microinstrução lida no armazenamento de controle.

byte MIR_B;         //Define qual dos registradores será usado no barramento B.
byte MIR_Mem;       //Define qual operação será feita com a memória principal.
word MIR_C;         //Define quais registradores serão gravados a partir do barramento C.
byte MIR_ALU;       //Define qual operação será feita na ALU e no deslocador.
byte MIR_JAM;       //Define se haverá pulo ou não.

word MPC = 0;       //Define a próxima microinstrução a ser buscada no armazenamento de controle.

// Memórias

byte memoria[1000000];              //Memória principal.

microinstrucao armazenamento[512];  //Armazenamento de controle.

// Declaração das funções.

void carregar_microprograma();  //Carrega o microprog.rom no armazenamento de controle.
void carregar_programa();       //Carrega o programa na memória principal.
void separar_microinstrucao();  //Separa MIR em suas subdivisões.
void atribuir_barramentoB();    //Atribui um valor ao barramento B a partir de um registrador.
void realizar_operacao_ALU();   //Realiza a operação da ALU.
void atribuir_registradores();  //Atribui o valor do barramento C para um ou mais registradores.
void operar_memoria();          //Realiza a operação de memória.
void pular();                   //Realiza o pulo, se necessário.

void exibir_processo();         //Exibe o processo no terminal.

// Emulador

int main(int argc, char *argv[]) {
	carregar_microprograma();
	carregar_programa();

	while(1) {
		MIR = armazenamento[MPC];

		exibir_processo();
		separar_microinstrucao();
		atribuir_barramentoB();
		realizar_operacao_ALU();
		atribuir_registradores();
		operar_memoria();
		pular();
	}

	return 0;
}

// Escopo das funções.

void carregar_microprograma() {
	FILE *microprograma;
	microprograma = fopen("microprog.rom", "rb");

	if (microprograma != NULL) {
		fread(&armazenamento, sizeof(microinstrucao), 512, microprograma);
		fclose(microprograma);
	}
}

void carregar_programa() {
	FILE *programa;
	programa = fopen(argv[1], "rb");

	char tamanho_programa[1];					// Registra o tamanho do programa.
    fread(tamanho_programa, 1, 1, programa);

	if (programa != NULL) {
		fread(memoria, 1, tamanho_programa[0], programa);
		fclose(programa);
	}
}

void separar_microinstrucao() {
    MIR_B   = (MIR)       & 0b1111;
	MIR_Mem = (MIR >> 4)  & 0b111;
	MIR_C   = (MIR >> 7)  & 0b111111111;
	MIR_ALU = (MIR >> 16) & 0b11111111;
	MIR_JAM = (MIR >> 24) & 0b111;
	MPC     = (MIR >> 27) & 0b111111111;
}

void atribuir_barramentoB() {
    switch(MIR_B) {
        case 0: barramentoB = MDR; break;
        case 1: barramentoB = PC;  break;
        case 2: barramentoB = MBR; break;
        case 3: barramentoB = MBR; if(MBR & (0b1 << 7)) barramentoB = (barramentoB | (0b11111111 << 8)); break; 
        case 4: barramentoB = SP;  break;
        case 5: barramentoB = LV;  break;
        case 6: barramentoB = CPP; break;
        case 7: barramentoB = TOS; break;
        case 8: barramentoB = OPC; break;

        default: barramentoB = -1; break;
    }
}

void realizar_operacao_ALU() {
    switch((MIR_ALU << 2) >> 2 ) {
        case 12: barramentoC = H & barramentoB;		    break;
		case 17: barramentoC = 1;			            break;
		case 18: barramentoC = -1;			            break;
		case 20: barramentoC = barramentoB;			    break;
		case 24: barramentoC = H;			            break;
		case 26: barramentoC = ~H;			            break;
		case 28: barramentoC = H | barramentoB;		    break;
		case 44: barramentoC = ~barramentoB;            break;
		case 53: barramentoC = barramentoB + 1;         break;
		case 54: barramentoC = barramentoB - 1;		    break;
		case 57: barramentoC = H + 1;		            break;
		case 59: barramentoC = -H;			            break;
		case 60: barramentoC = H + barramentoB;		    break;
		case 61: barramentoC = H + barramentoB + 1;     break;
		case 63: barramentoC = barramentoB - H;		    break;

		default: break;
    }

    //Verifica se o resultado da ALU é 0.

    if(barramentoC) N = 1;
    else Z = 1;

    //Faz o deslocamento.
    
    switch(MIR_ALU >> 6) {
        case 1: barramentoC = barramentoC >> 1; break;
        case 2: barramentoC = barramentoC << 8; break;
    }
}

void atribuir_registradores() {
    if (MIR_C & 0b000000001) MAR = barramentoC;
    if (MIR_C & 0b000000010) MDR = barramentoC;
    if (MIR_C & 0b000000100) PC  = barramentoC;
    if (MIR_C & 0b000001000) SP  = barramentoC;
    if (MIR_C & 0b000010000) LV  = barramentoC;
    if (MIR_C & 0b000100000) CPP = barramentoC;
    if (MIR_C & 0b001000000) TOS = barramentoC;
    if (MIR_C & 0b010000000) OPC = barramentoC;
    if (MIR_C & 0b100000000) H   = barramentoC;
}

void operar_memoria() {
	switch(MIR_Mem) {
		case 1: MBR = memoria[PC]; 					break;	//FETCH
		case 2: memcpy(&MDR, &memoria[MAR*4], 4);	break;	//READ
		case 4: memcpy(&memoria[MAR*4], &MDR, 4); 	break;	//WRITE

		default: break;
	}
}

void pular() {
	if(MIR_JAM & 0b001) MPC = MPC | (Z << 8);
	if(MIR_JAM & 0b010) MPC = MPC | (N << 8);
	if(MIR_JAM & 0b100) MPC = MPC | MBR;
}

void exibir_processo() {
	system("clear");

	printf("\nMAR: %d", MAR);
	printf("\nMDR: %d", MDR);
	printf("\nPC : %d", PC);
	printf("\nMBR: %d", (word) MBR);
	printf("\nSP : %d", SP);
	printf("\nLV : %d", LV);
	printf("\nCPP: %d", CPP);
	printf("\nTOS: %d", TOS);
	printf("\nOPC: %d", OPC);
	printf("\nH  : %d", H);
	printf("\n");
	printf("\nMPC: %d", MPC);
	printf("\n");

	int y = 0;

    for (int i = 0; i < 58; i++) {
        if (y < 4) {
            printf("Mem[%02d] : 0x0%x ", i, memoria[i]);
            y++;
        }
        else {
            y = 1;
            printf("\n");
            printf("Mem[%02d] : 0x0%x ", i, memoria[i]);
        }
	}
	
	getchar();
}

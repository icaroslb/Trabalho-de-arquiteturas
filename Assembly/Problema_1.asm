org 0x7c00  
bits 16     

mov ax, 0
mov ds, ax

inicio:	
	cli

	mov ah, 0	 ;Muda o modo de vídeo
	mov al, 0x13 ;modo gráfico 320x200x8bits (256 cores)
	int 0x10

	int 0x13
	mov ah, 0x02;ler setores
	mov al, 32  ;Hariamy: Lendo 32 setores que é o tamanho da imagem ;quantos setores devem ser lidos 32
	mov cl, 2  ;ler a partir do setor 2
	mov ch, 0  ;do cilindro 0
	mov dh, 0  ;do cabeçote 0
	mov bx, 0x7e00 ;coloque os dados lidos no end. 0x7e00
	int 0x13

	;move imagem para memória de vídeo
	mov ax, 0xA000 ;Endereço da memória de vídeo
	mov es, ax	   ;Coloca 0xA000 no registrador para se o setor
	mov di, 0	   ;Coloca o índice 0 no índice do setor da imagem
	mov si, 0x7e00 ;Onde a imagem tá salva
	
.copiar:	
	mov al, [ds:si] ;Move o conteúdo do setor 0 índice si para o registrador al
	mov [es:di], al ;Move o registrador al para o conteúdo do setor A000 índice di

;	Se o valor de di for igual ao tamanho da ímagem
	cmp di, 16000
	 
;	Então pula para o final
	je .final 

;	Senão, incrementa o si e o di
	inc si 
	inc di
	jmp .copiar

.final:
	jmp .final

times 510 - ($-$$) db 0
dw 0xaa55
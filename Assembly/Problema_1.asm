org 0x7c00  
bits 16     

mov ax, 0
mov ds, ax

inicio:	
	cli

	mov ah, 0
	mov al, 0x13 ;modo gráfico 320x200x8bits (256 cores)
	int 0x10

	int 0x13
	mov ah, 0x02 ;ler setores
	mov al, 32  ;Hariamy: Lendo 32 setores que é o tamanho da imagem ;quantos setores devem ser lidos 32
	mov cl, 2  ;ler a partir do setor 2
	mov ch, 0  ;do cilindro 0
	mov dh, 0  ;do cabeçote 0
	mov bx, 0x7e00 ;coloque os dados lidos no end. 0x7e00
	int 0x13

	;move imagem para memória de vídeo
	mov ax, 0xA000
	mov es, ax
	mov di, 0
	mov si, 0x7e00;Onde a imagem tá salva
	
.loop2:	
	mov al, [ds:si]
	mov [es:di], al

;	EXIBIR LENTAMENTE
	;mov ah, 0x0E
	;mov al, 't'
	;int 0x10
;	FIM EXIBIR LENTAMENTE

	cmp di, 16000
	je .final
	inc si
	inc di
	jmp .loop2

.final:
	jmp .final

times 510 - ($-$$) db 0
dw 0xaa55
org 0x7c00
bits 16

cli
inicio:

mov ah, 0x0e
mov al, '1'
int 0x10
mov al, ' '
int 0x10

mov cx, 21
mov ax, 1
mov bx, 0

.laco:
mov dx, ax
add ax, bx
mov bx, dx

push ax
push bx
push cx
push dx

jmp .printar

.volta:

pop dx
pop cx
pop bx
pop ax

dec cx
jz .fim

jmp .laco

.printar:
mov cx, 0
mov bx, 10

.calcular:
or ax, ax
jz .mostrar

mov dx, 0
idiv bx
mov dh, 0x0e
push dx
inc cx
jmp .calcular

.mostrar:
pop ax
add al, 48
int 0x10
dec cx
jz .finalizar

jmp .mostrar

.finalizar:
mov al, ' '
int 0x10
jmp .volta

.fim:
jmp .fim

times 510 - ($ - $$) db 0
dw 0xaa55

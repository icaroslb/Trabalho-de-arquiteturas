import sys

""" Dicionário que contém as instruções
    Formato: { mnemônico: [hex, [operandos]]}
    Tipos de operandos: byte, varnum. new_varnum, index, const, offset, disp
"""

dicionario_instrucoes = {
                        'nop':           [0x01, []],
                        'iadd':          [0x02, []],
                        'isub':          [0x05, []],
                        'iand':          [0x08, []],
                        'ior':           [0x0b, []],
                        'dup':           [0x0e, []],
                        'pop':           [0x10, []],
                        'swap':          [0x13, []],
                        'bipush':        [0x19, ['byte']],
                        'iload':         [0x1c, ['varnum']],
                        'istore':        [0x22, ['new_varnum']],
                        'wide':          [0x28, []],
                        'ldc_w':         [0x32, ['index']], 
                        'iinc':          [0x36, ['varnum', 'const']],
                        'goto':          [0x3c, ['offset']],
                        'iflt':          [0x43, ['offset']],
                        'ifeq':          [0x47, ['offset']],
                        'if_icmpeq':     [0x4b, ['offset']],
                        'invokevirtual': [0x55, ['disp']], 
                        'ireturn':       [0x6b, []]
                    }

# Estruturas e constantes globais #

dicionario_labels    = {} # Dicionário que armazena as labels e o byte que cada uma indica
dicionario_variaveis = {} # Dicionário que armazena as variáveis e suas posições de memória

lista_byte = [] # Lista que armazena os bytes que serão adicionados ao arquivo
log_erro   = '' # String que armazena o log de erros

contador_byte  = 0 # Conta o programa byte a byte
contador_linha = 0 # Conta o programa linha por linha

proxima_variavel = 1 # Indica a posição de memória da pŕoxima variável a ser adicionada

def main():
    global contador_linha

    try: # Tenta ler o programa escrito em assembly
        programa = open(sys.argv[1], 'r')
    except:
        print('Arquivo não encontrado')
        raise IndexError

    for linha in programa: # Percorre as linhas contidas no programa
        contador_linha += 1

        s_linha = linha.lower().split() # Separa a linha em strings em coloca em uma lista


        if label(s_linha[0]): # Verifica se a linha contém uma label
            adicionar_label(s_linha[0]) # Se sim, cria a label
            del s_linha[0] #Remove a label do vetor de strings


        if s_linha != [] and not comentario(s_linha[0]): # Verifica se linha vazia ou comentário
            instrucao = s_linha[0]

            if intrucao_valida(instrucao): # Se for uma instrução válida
                tipo_operando = dicionario_instrucoes[instrucao][1] 
                numero_operandos = len(tipo_operando)
                operando = s_linha[1 : numero_operandos + 1]

                if operando_valido(instrucao, operando, tipo_operando, numero_operandos):
                    adicionar_instrucao(instrucao)
                    adicionar_operando(instrucao, operando, tipo_operando, numero_operandos)

                else:
                    adicionar_erro('Operando inválido', contador_linha)

            else:
                adicionar_erro('Instrução Inválida', contador_linha)
                
    programa.close()
    
    if log_erro == '':    
        gerar_arquivo()

    else:
        print(log_erro)

# Escopo das funções #

"""Informa se uma string é um comentário"""
def comentario(string):
    return string.startswith('//')

"""Informa se uma string é uma label"""
def label(string):
    global contador_linha, dicionario_instrucoes, dicionario_variaveis
    
    if string in dicionario_variaveis.keys():
        adicionar_erro('Label possui o mesmo nome de uma variável', contador_linha)
        return False

    else:
        return (string not in dicionario_instrucoes.keys()) and string.replace("_","").isalnum()

"""Adiciona uma label ao dicionário dicionario_labels"""
def adicionar_label(label):
    global contador_byte, dicionario_labels
    dicionario_labels[label] = contador_byte + 1

"""Informa se uma string é uma instrução é válida"""
def intrucao_valida(string):
    global dicionario_instrucoes
    return string in dicionario_instrucoes.keys()

"""Adiciona o endereço de uma instrução ao lista_byte"""
def adicionar_instrucao(instrucao):
    global contador_byte, dicionario_instrucoes
    contador_byte += 1
    lista_byte.append(dicionario_instrucoes[instrucao][0])

"""Informa se uma string é uma variável válida"""
def variavel_valida(string):
    global contador_linha, dicionario_labels

    if string in dicionario_labels.keys():
        adicionar_erro('Variável possui o mesmo nome de uma label', contador_linha)
        return False
    else:
        return string[0].isalpha() and string.replace("_","").isalnum()

"""Informa se uma linha possui operando(s) válido(s)."""
def operando_valido(instrucao, operando, tipo_operando, numero_operandos):
    global dicionario_variaveis

    if numero_operandos == len(operando): 
        if numero_operandos == 0:
            return True

        check = True

        for i in range (0, numero_operandos, 1):
            
            if tipo_operando[i] == 'varnum':   
                if operando[i] not in dicionario_variaveis:
                	check = False

            elif tipo_operando[i] == 'new_varnum':
                if not variavel_valida(operando[i]):
                	check = False

            elif (tipo_operando[i] == 'byte') or (tipo_operando[i] == 'const') or (tipo_operando[i] == 'disp') or (tipo_operando[i] == 'index'):
                if not operando[i].isnumeric():
                	check = False

            else:  # tipo_operando[i] == 'offset':
                if not label(operando[i]):
                	check = False
        return check
    else:
        return False

"""Adiciona o endereço de um operando ao lista_byte"""
def adicionar_operando(instrucao, operando, tipo_operando, numero_operandos):
    global contador_byte, proxima_variavel, dicionario_variaveis

    for i in range (0, numero_operandos, 1):
        if tipo_operando[i] == 'varnum':
            lista_byte.append(dicionario_variaveis[operando[i]])
            contador_byte += 1

        elif tipo_operando[i] == 'new_varnum':
            if operando[i] not in dicionario_variaveis.keys():
                dicionario_variaveis[operando[i]] = proxima_variavel
                proxima_variavel += 1

            lista_byte.append(dicionario_variaveis[operando[i]])
            contador_byte += 1

        elif (tipo_operando[i] == 'byte') or (tipo_operando[i] == 'const'):
            lista_byte.append(int(operando[i]))
            contador_byte += 1

        elif (tipo_operando[i] == 'disp') or (tipo_operando[i] == 'index'):
            lista_byte.append(int(operando[i]) & 0xff)
            lista_byte.append(int(operando[i]) >> 8)
            contador_byte += 2

        else:  # tipo_operando[i] == 'offset':
            """Cria uma lista [label, endereço] que será substituída na função gerar_arquivo()"""
            lista_byte.append([operando[i], contador_byte])
            contador_byte += 2

"""Adiciona erros ao log"""
def adicionar_erro(tipo_erro, linha_erro):
    global log_erro

    if (str(linha_erro) not in log_erro):
        erro_para_adicionar = 'Linha: ' + format(linha_erro, '#02d') + ' erro: ' + str(tipo_erro) + '\n'
        log_erro = log_erro + erro_para_adicionar

"""Gera o arquivo final"""
def gerar_arquivo():
    global lista_byte, contador_byte, contador_linha

    byte_array_final = bytearray()      # Bytearray que será gravado no arquivo

    # Quantidade de bytes do programaa
    programa_size = (contador_byte + 20).to_bytes(4, byteorder = 'little', signed = True)
    byte_array_final += programa_size

    # Inicialização da memória
    bytes_inicializacao = [   
                            0x7300,
                            0x0006,
                            0x1001,
                            0x0400,
                            0x1001 + len(dicionario_variaveis.keys())
                        ]
    try: #Tenta gerar o arquivo
        
        for byte in bytes_inicializacao:
            byte_array_final += byte.to_bytes(4, byteorder = 'little', signed = True)
    
        # Bytes das instruções, operandos e labels
        for byte in lista_byte:
            if type(byte) is list: # Verifica se é label
                if byte[0] not in dicionario_labels.keys(): # Verifica se a label existe
                        adicionar_erro('Label não encontrada', contador_linha)
                    
                else:
                    label_byte = dicionario_labels[byte[0]] - byte[1] 
                    byte_array_final += label_byte.to_bytes(2, byteorder = 'big', signed = True)

            else:
                byte_array_final.append(byte)
   
    except Exception:
        print("Erro ao gerar o arquivo")
        print(log_erro)

    programa_final = open(sys.argv[1][:-4] + '.exe', 'wb')
    programa_final.write(byte_array_final)
    programa_final.close()

"""Executa a função main"""
if __name__ == '__main__':
    main()
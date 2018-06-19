import sys      # Biblioteca usada para receber argumentos da linha de comando.

# Dicionário que contém as instruções sem operando. #

no_operand_dict = { 'nop':    0x01,     'iadd':          0x02,    'isub':   0x05,     'iand':      0x08,
                    'ior':    0x0b,     'dup':           0x0e,    'pop':    0x10,     'swap':      0x13,
                    'wide':   0x28,     'ireturn':       0x6b }

# Dicionário que contém as instruções com operando(s). #

operand_dict = {    'goto':   0x3c,     'iftl':          0x43,    'ifeq':   0x47,     'if_icmpeq': 0x4b,   # Desvio.
                    'ldc_w':  0x32,     'invokevirtual': 0x55,                                             # Operando 2 bytes.
                    'bipush': 0x19,     'iload':         0x1c,    'istore': 0x22,     'iinc':      0x36 }  # Operando 1 byte.

# Declaração das estruturas de dados utilizadas. #

labels_list = {}        # Dicionário que armazena as labels e suas respectivas posições (em bytes).
constants_list = {}     # Dicionário que armazena as constantes e suas posições de memória (em bytes).
byte_list = []          # Lista que armazena os bytes que serão adicionados ao arquivo.
error_log = ''          # String que armazena o log de erros do programa.

# Declaração dos contadores globais. #

byte_counter = 0    # Contador global de bytes.
next_constant = 0   # Contador global de constantes.

# Função main. #

def main():
    global byte_counter

    with open(sys.argv[1]) as program:      # Abre o arquivo de acordo com o argumento passado pela linha de comandos.
        line_counter = 0

        for line in program:
            line_counter += 1

            sline = line.lower().split()        # Separa os strings da linha que não são espaços em braco e os armazena em sline.
            
            if sline != [] and not is_comment(sline[0]):        # Se linha vazia ou comentário, o programa segue para a próxima linha.
                
                if is_label(sline[0]):      # Verifica se a linha contém uma label.
                    add_label(sline[0])
                    del sline[0]

                if is_valid_instruction(sline[0]):      # Verifica se a instrução é válida.
                    add_instruction(sline[0])

                else:
                    add_error('invalid_instruction', line_counter)
                    
                if is_valid_operand(sline):     # Verifica se o(s) operando(s) é(são) válido(s).
                    if sline[0] in operand_dict:
                        add_operand(sline)

                else:
                    if ('Line: %02d' %line_counter) not in error_log:
                        add_error('invalid_operand', line_counter)

    if error_log == '':    
        generate_file()     # Gera o arquivo final.
    
    else:
        print (error_log)

# Escopo das funções. #

def is_comment(string):              # Informa se uma string é um comentário.
    return string.startswith('//')

def is_label(string):                # Informa se uma string é uma label.
    return (string.startswith('l')) and (string[1:].isnumeric())

def add_label(label):                # Adiciona uma label ao vetor byte_list.
    global byte_counter

    labels_list[label] = byte_counter + 1

def is_valid_instruction(string):    # Informa se uma instrução é válida.
    return (string in no_operand_dict.keys()) or (string in operand_dict.keys())

def add_instruction(instruction):    # Adiciona o endereço de uma instrução ao byte_list.
    global byte_counter
    byte_counter += 1

    if instruction in no_operand_dict.keys():  # Instruções sem operando.
        byte_list.append(no_operand_dict[instruction])

    else:                            # Instruções com operando(s).
        byte_list.append(operand_dict[instruction])

def is_valid_constant(string):       # Informa se uma string é uma constante válida.
    return (len(string) > 0) and (string[0].isalpha())

def is_valid_operand(sline):         # Informa se uma linha possui operando(s) válido(s).

    if sline[0] in no_operand_dict.keys():     # Instruções sem operando, espera-se ausência de operandos.
        return (len(sline) == 1) or (is_comment(sline[1]))
    
    elif sline[0] in operand_dict.keys():   # Instruções com operando.
        if (len(sline) > 1) and (not is_comment(sline[1])):
            
            if (sline[0] == 'goto') or (sline[0] == 'iflt') or (sline[0] == 'ifeq') or (sline[0] == 'if_icmpeq'):
                return ((len(sline) == 2) or (is_comment(sline[2]))) and (is_label(sline[1]))
            
            elif (sline[0] == 'bipush') or (sline[0] == 'ldc_w') or (sline[0] == 'invokevirtual'):
                return ((len(sline) == 2) or (is_comment(sline[2]))) and (sline[1].isnumeric())
            
            elif (sline[0] == 'iload'):
                return ((len(sline) == 2) or (is_comment(sline[2]))) and (sline[1] in constants_list)
            
            elif (sline[0] == 'istore'):
                return ((len(sline) == 2) or (is_comment(sline[2]))) and (is_valid_constant(sline[1]))
            
            else: # (sline == 'iinc')
                if (len(sline) > 2) and (not is_comment(sline[2])):
                    return ((len(sline) == 3) or (is_comment(sline[3]))) and (sline[1].isnumeric()) and (is_valid_constant(sline[2]))

    else:
        return False

def add_operand(sline):              # Adiciona o endereço de um operando ao byte_list.
    global byte_counter
    global next_constant

    if (sline[0] == 'goto') or (sline[0] == 'iflt') or (sline[0] == 'ifeq') or (sline[0] == 'if_icmpeq'):
        byte_list.append([sline[1], byte_counter])
        byte_counter += 2
    
    elif (sline[0] == 'bipush'):
        byte_list.append(int(sline[1]))
        byte_counter += 1
    
    elif (sline[0] == 'iload'):
        byte_list.append(constants_list[sline[1]])
        byte_counter += 1
    
    elif (sline[0] == 'istore'):
        if sline[1] not in constants_list.keys():
            constants_list[sline[1]] = next_constant
            next_constant += 1
        byte_list.append(constants_list[sline[1]])
        byte_counter += 1

    elif (sline[0] == 'ldc_w') or (sline[0] == 'invokevirtual'):
        byte_list.append(int(sline[1]) & 0xFF)
        byte_list.append(int(sline[1]) >> 8)
        byte_counter += 2
    
    else: # (sline == 'iinc')
        byte_list.append(int(sline[1]))
        byte_list.append(constants_list[sline[2]])
        byte_counter += 2

def add_error(error_type, error_line): # Adiciona um erro ao log de erros.
    global error_log

    error_to_be_added = 'Line: ' + format(error_line, '#02d') + ' error: ' + str(error_type) + '\n'
    error_log = error_log + error_to_be_added


def generate_file(): # Gera o arquivo final.
    global byte_list
    global byte_counter

    final_byte_array = bytearray()      # Bytearray que será gravado no arquivo.
   
    # Escrevendo os 4 bytes de tamanho do programa. 
    final_byte_array.append((byte_counter + 20) & 0xff)
    final_byte_array.append(((byte_counter + 20) >> 8) & 0xff)
    final_byte_array.append(((byte_counter + 20) >> 16) & 0xff)
    final_byte_array.append(((byte_counter + 20) >> 24) & 0xff)
    
    # Inicialização da memória.
    bytes_to_be_added = [0x00, 0x73, 0x00, 0x00, # memory[1]  = 0x0073
                         0x06, 0x00, 0x00, 0x00, # memory[4]  = 0x0006
                         0x01, 0x10, 0x00, 0x00, # memory[8]  = 0x1001
                         0x00, 0x04, 0x00, 0x00, # memory[12] = 0x0400
                         0x01, 0x10, 0x00, 0x00] # memory[16] = 0x1001

    for byte in bytes_to_be_added:
        final_byte_array.append(byte)          # Escreve a inicialização da memória.
   
    for byte in byte_list:                                         # Escreve os bytes das instruções, labels e operandos.

        if type(byte) is list:                                     # Verifica se é label e adiciona 2 bytes relativos.
            label_byte = labels_list[byte[0]] - byte[1]            # Valor da label será a posição relativa da label à sua linha destino.
            final_byte_array.append(label_byte & 0xFF)
            final_byte_array.append(label_byte >> 8)

        else:
            final_byte_array.append(byte)                          # Se não for label, apenas adiciona o byte ao bytearray.

    # print(final_byte_array)    # Mostra o bytearray antes de finalizar.
    
    final_program = open(sys.argv[1][:-4] + '.exe', 'wb')   # Abre o arquivo para escrita.
    final_program.write(final_byte_array)                   # Escreve.
    final_program.close()

if __name__ == "__main__":
    main()

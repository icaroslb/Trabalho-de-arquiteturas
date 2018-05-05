import sys

""" Dicionário que contém as instruções
    Formato: { mnemônico: [hex, [operandos]]}
    Tipos de operandos: const1, const2, var, new_var, label """

instructions_dict = {
                        'nop':                  [0x01, []],
                        'iadd':                 [0x02, []],
                        'isub':                 [0x05, []],
                        'iand':                 [0x08, []],
                        'ior':                  [0x0b, []],
                        'dup':                  [0x0e, []],
                        'pop':                  [0x10, []],
                        'swap':                 [0x13, []],
                        'bipush':               [0x19, ['byte']],
                        'iload':                [0x1c, ['varnum']],
                        'istore':               [0x22, ['new_varnum']],
                        'wide':                 [0x28, []],
                        'ldc_w':                [0x32, ['index']],          # Não entendo como funciona
                        'iinc':                 [0x36, ['varnum', 'const']],
                        'goto':                 [0x3c, ['offset']],
                        'iflt':                 [0x43, ['offset']],
                        'ifeq':                 [0x47, ['offset']],
                        'if_icmpeq':            [0x4b, ['offset']],
                        'invokevirtual':        [0x55, ['disp']],           # Não entendo como funciona
                        'ireturn':              [0x6b, []]
                    }

# Estruturas e constantes globais #

labels_dict = {}        # Dicionário que armazena as labels e o byte que cada uma indica
variables_dict = {}     # Dicionário que armazena as variáveis e suas posições de memória
byte_list = []          # Lista que armazena os bytes que serão adicionados ao arquivo
error_log = ''          # String que armazena o log de erros

byte_counter = 0        # Conta o programa byte a byte
line_counter = 0        # Conta o programa linha por linha

next_variable = 1       # Indica a posição de memória da pŕoxima variável a ser adicionada

def main():
    global line_counter

    try:
        program = open(sys.argv[1], 'r')
    except:
        print('no argument')
        raise IndexError

    for line in program:
        line_counter += 1

        s_line = line.lower().split()                                           # Separa os strings em uma lista

        try:
            if is_label(s_line[0]):                                         # Verifica se a linha contém uma label
                add_label(s_line[0])                                        # Se sim, cria a label
                del s_line[0]
        
        except IndexError:
            pass
        

        if s_line != [] and not is_comment(s_line[0]):                          # Verifica se linha vazia ou comentário
            instruction = s_line[0]

            if is_valid_instruction(instruction):
                operand_types = instructions_dict[instruction][1]
                number_of_operands = len(operand_types)
                operand = s_line[1 : number_of_operands + 1]

                if is_valid_operand(instruction, operand, operand_types, number_of_operands):
                    add_instruction(instruction)
                    add_operand(instruction, operand, operand_types, number_of_operands)

                else:
                    add_error('invalid_operand', line_counter)

            else:
                add_error('invalid_instruction', line_counter)
                
    program.close()
    
    if error_log == '':    
        generate_file()

    else:
        print (error_log)

# Escopo das funções #

def is_comment(string):
    """Informa se uma string é um comentário"""
    try:
        return string.startswith('//')
    except TypeError:
        raise TypeError

def is_label(string):
    """Informa se uma string é uma label"""
    global line_counter, instructions_dict, variables_dict
    
    if string in variables_dict.keys():
        add_error('label_same_name_as_variable', line_counter)
        return False

    else:
        return (string not in instructions_dict.keys()) and string.replace("_","").isalnum()

def add_label(label):
    """Adiciona uma label ao dicionário labels_dict"""
    global byte_counter, labels_dict

    labels_dict[label] = byte_counter + 1

def is_valid_instruction(string):
    """Informa se uma string é uma instrução é válida"""
    global instructions_dict

    return string in instructions_dict.keys()

def add_instruction(instruction):
    """Adiciona o endereço de uma instrução ao byte_list"""
    global byte_counter, instructions_dict

    byte_counter += 1
    byte_list.append(instructions_dict[instruction][0])

def is_valid_variable(string):
    """Informa se uma string é uma variável válida"""
    global line_counter, labels_dict

    if string in labels_dict.keys():
        add_error('varialbe_same_name_as_label', line_counter)
        return False

    else:
        return string[0].isalpha() and string.replace("_","").isalnum()

def is_valid_operand(instruction, operand, operand_types, number_of_operands):
    """Informa se uma linha possui operando(s) válido(s)."""
    global variables_dict

    if number_of_operands == len(operand):
        if number_of_operands == 0:
            return True
        
        for i in range (0, number_of_operands, 1):
            
            if operand_types[i] == 'varnum':
                
                return operand[i] in variables_dict.keys()

            elif operand_types[i] == 'new_varnum':
                
                return is_valid_variable(operand[i])

            elif (operand_types[i] == 'byte') or (operand_types[i] == 'const') or (operand_types[i] == 'disp') or (operand_types[i] == 'index'):
                return operand[i].isnumeric()

            else:  # operand_types[i] == 'offset':
                
                return is_label(operand[i])
    
    else:
        return False

def add_operand(instruction, operand, operand_types, number_of_operands):
    """Adiciona o endereço de um operando ao byte_list"""
    global byte_counter, next_variable, variables_dict

    for i in range (0, number_of_operands, 1):
        if operand_types[i] == 'varnum':
            byte_list.append(variables_dict[operand[i]])
            byte_counter += 1

        elif operand_types[i] == 'new_varnum':
            if operand[i] not in variables_dict.keys():
                variables_dict[operand[i]] = next_variable
                next_variable += 1

            byte_list.append(variables_dict[operand[i]])
            byte_counter += 1

        elif (operand_types[i] == 'byte') or (operand_types[i] == 'const'):
            byte_list.append(int(operand[i]))
            byte_counter += 1

        elif (operand_types[i] == 'disp') or (operand_types[i] == 'index'):
            byte_list.append(int(operand[i]) & 0xff)
            byte_list.append(int(operand[i]) >> 8)
            byte_counter += 2

        else:  # operand_types[i] == 'offset':
            """Cria uma lista [label, endereço] que será substituída na função generate_file()"""
            byte_list.append([operand[i], byte_counter])
            byte_counter += 2

def add_error(error_type, error_line):
    """Adiciona erros ao log"""
    global error_log

    if (str(error_line) not in error_log):
        error_to_be_added = 'Line: ' + format(error_line, '#02d') + ' error: ' + str(error_type) + '\n'
        error_log = error_log + error_to_be_added

def generate_file():
    """Gera o arquivo final"""
    global byte_list, byte_counter, line_counter

    final_byte_array = bytearray()      # Bytearray que será gravado no arquivo

    # Quantidade de bytes do programa
    program_size = (byte_counter + 20).to_bytes(4, byteorder = 'little', signed = True)
    final_byte_array += program_size

    # Inicialização da memória
    bytes_to_be_added = [   
                            0x7300,
                            0x0006,
                            0x1001,
                            0x0400,
                            0x1001 + len(variables_dict.keys())
                        ]

    for byte in bytes_to_be_added:
        final_byte_array += byte.to_bytes(4, byteorder = 'little', signed = True)
   
    # Bytes das instruções, operandos e labels
    for byte in byte_list:
        if type(byte) is list:                                     # Verifica se é label
            if byte[0] not in labels_dict.keys():                  # Verifica se a label existe
                    add_error('label_not_found', line_counter)
                
            else:
                label_byte = labels_dict[byte[0]] - byte[1] 
                final_byte_array += label_byte.to_bytes(2, byteorder = 'big', signed = True)

        else:
            final_byte_array.append(byte)

    final_program = open(sys.argv[1][:-4] + '.exe', 'wb')
    final_program.write(final_byte_array)
    final_program.close()

if __name__ == '__main__':
    main()
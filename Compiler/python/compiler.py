import sys
import getopt
import re

CLI_ERROR_CODE = 2

# Opcodes de 2 bits
isa = {
    'WRITE_MEM': {'opcode': '00'},
    'READ_MEM': {'opcode': '01'},
    'BROADCAST_INVALIDATE': {'opcode': '10'}
}

ADDR_SIZE = 16
CACHE_LINE_SIZE = 8
QOS_SIZE = 4
SRC_SIZE = 5

MAX_ADDR = 4096 * 4
MAX_CACHE_LINE = 512
MAX_QOS = 15
MAX_SRC = 31


def get_args(argv):
    input_file = ''
    output_file = ''
    try:
        opts, _ = getopt.getopt(argv, "hi:o:", ["ifile=", "ofile="])
    except getopt.GetoptError:
        print('Usage: python compiler.py -i <inputfile> -o <outputfile>')
        sys.exit(CLI_ERROR_CODE)
    for opt, arg in opts:
        if opt == '-h':
            print('python compiler.py -i <inputfile> -o <outputfile>')
            sys.exit()
        elif opt in ("-i", "--ifile"):
            input_file = arg
        elif opt in ("-o", "--ofile"):
            output_file = arg
    return input_file, output_file


def clean_instructions(file):
    instructions = []
    for line in file:
        line = line.split(';')[0].strip()
        if line:
            instructions.append(re.split(r'[\s,]+', line))
    return instructions


def get_binary(instructions):
    binary_instr = []
    for instr in instructions:
        mnemonic = instr[0]
        opcode = isa[mnemonic]['opcode']

        if mnemonic == 'WRITE_MEM':
            src = validate_src(instr[1])
            addr = validate_addr(instr[2])
            num_cl = validate_cache_line(instr[3])
            start_cl = validate_cache_line(instr[4])
            qos = validate_qos(instr[5])
            bin_str = opcode + src + addr + num_cl + start_cl + qos
            binary_instr.append(bin_str)

        elif mnemonic == 'READ_MEM':
            src = validate_src(instr[1])
            addr = validate_addr(instr[2])
            size = validate_cache_line(instr[3])
            qos = validate_qos(instr[4])
            relleno = '0' * 8
            bin_str = opcode + src + addr + size + relleno + qos
            binary_instr.append(bin_str)

        elif mnemonic == 'BROADCAST_INVALIDATE':
            src = validate_src(instr[1])
            cache_line = validate_cache_line(instr[2])
            qos = validate_qos(instr[3])
            relleno1 = '0' * 8
            relleno2 = '0' * 16
            bin_str = opcode + src + relleno1 + cache_line + relleno2 + qos
            binary_instr.append(bin_str)

        else:
            raise Exception(f"Instrucción no válida: {mnemonic}")
    return binary_instr


def to_bin(value, bits):
    num = int(value, 0)
    return bin(num & (2**bits - 1))[2:].zfill(bits)


def validate_src(value):
    num = int(value, 0)
    if num < 0 or num > MAX_SRC:
        raise Exception(f"SRC inválido: {value} (debe estar entre 0 y {MAX_SRC})")
    return to_bin(value, SRC_SIZE)


def validate_addr(value):
    num = int(value, 0)
    if num % 4 != 0:
        raise Exception(f"Dirección {value} no está alineada a 4 bytes")
    if num < 0 or num >= MAX_ADDR:
        raise Exception(f"Dirección {value} fuera del rango de memoria compartida")
    return to_bin(value, ADDR_SIZE)


def validate_cache_line(value):
    num = int(value, 0)
    if num < 0 or num >= MAX_CACHE_LINE:
        raise Exception(f"Línea de caché {value} fuera de rango [0-{MAX_CACHE_LINE - 1}]")
    return to_bin(value, CACHE_LINE_SIZE)


def validate_qos(value):
    num = int(value, 0)
    if num < 0 or num > MAX_QOS:
        raise Exception(f"Valor de QoS {value} fuera de rango [0-{MAX_QOS}]")
    return to_bin(value, QOS_SIZE)


def write_binary(instructions, output_file):
    with open(output_file, 'w') as f:
        for line in instructions:
            f.write(line + '\n')


if __name__ == "__main__":
    try:
        input_file, output_file = get_args(sys.argv[1:])
        if not input_file or not output_file:
            raise Exception("Missing input or output file.")
        with open(input_file, 'r') as f:
            instr = clean_instructions(f)
        bin_instr = get_binary(instr)
        write_binary(bin_instr, output_file)
    except Exception as e:
        print(e)
        sys.exit(CLI_ERROR_CODE)

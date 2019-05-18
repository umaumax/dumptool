#!/usr/bin/env python3

import struct
import argparse
import sys


def parse_binary(input_file, binary_type, offset):
    with open(input_file, "rb") as f:
        data = f.read()
        if binary_type == 'png':
            fmt = '>I4sIIBB'
            ret = struct.unpack_from(fmt, data, offset + 8)
            print(ret)
            return 0
        if binary_type == 'raw':
            fmt = 'fffi'
            n = 16  # TODO: don't use const value
            for i in range(n):
                ret = struct.unpack_from(fmt, data, offset)
                print('(x,y,z,v)', ret)
                offset += struct.calcsize(fmt)
            return 0
        print('"{}" is not implemented yet'.format(binary_type))
        return 1


def main():
    binary_types = ['png', 'raw']
    parser = argparse.ArgumentParser()
    parser.add_argument('-t', '--type', default='', type=str, help='type e.g. ' + ', '.join(binary_types))
    parser.add_argument('-o', '--offset', default=0, type=int, help='file head offset')
    parser.add_argument('input_file', nargs=1, help='input filepath')

    args, extra_args = parser.parse_known_args()
    offset = args.offset
    binary_type = args.type
    input_file = args.input_file[0]
    if binary_type not in binary_types:
        print('"{}" is not supported'.format(binary_type))
        print('supported types: [{}]'.format(', '.join(binary_types)))
        sys.exit(1)

    ret = parse_binary(input_file, binary_type, offset)
    sys.exit(ret)


if __name__ == '__main__':
    main()

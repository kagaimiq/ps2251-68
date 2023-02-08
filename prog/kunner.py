from scsiio import SCSIDev
import argparse

ap = argparse.ArgumentParser(description='Phison enxecutive')
ap.add_argument('-d', '--device', help='Path to device', required=True)
args = ap.parse_args()

def hexdump(data, width=16):
    for i in range(0, len(data), width):
        s = '%8X: ' % i

        for j in range(width):
            if (i+j) < len(data):
                s += '%02X ' % data[i+j]
            else:
                s += '-- '

        s += ' |'

        for j in range(width):
            if (i+j) < len(data):
                c = data[i+j]
                if c < 0x20 or c >= 0x7f: c = ord('.')
                s += chr(c)
            else:
                s += ' '

        s += '|'

        print(s)
    print()

with SCSIDev(args.device) as dev:
    #------- NAND IDs -------#
    data = bytearray(2560)
    dev.execute(b'\x06\xe6\xdc\x00\x00\x00\x00\x00\x05\x00\x00', None, data)

    hexdump(data)


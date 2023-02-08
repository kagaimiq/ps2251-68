from scsiio import SCSIDev
import argparse

ap = argparse.ArgumentParser(description='Phison executive')
ap.add_argument('-d', '--device', help='Path to device', required=True)
ap.add_argument('file', help='File to run')
args = ap.parse_args()



with SCSIDev(args.device) as dev:
    #------- NAND IDs -------#
    data = bytearray(512)
    dev.execute(b'\x06\x04\x00\x00\x00\x00', None, data)

    for i in range(8):
        print('%d: %s' % (i, memoryview(data[(i*0x10):(i*0x10)+0x6]).hex('-', 1)))

    print()

    #exit()

    #--------- Send ---------#
    with open(args.file, 'rb') as f:
        hdr = f.read(512)
        dat = f.read()

        if len(dat) > 0x8000: dat = dat[:0x8000]
        else:                 dat = dat + bytes(0x8000 - len(dat))

        dev.execute(b'\x06\x11\x03\x00\x00\x00\x00\x00\x01', hdr, None)
        dev.execute(b'\x06\x11\x02\x00\x00\x00\x00\x00\x40', dat, None)

    #-------- Readout --------#
    data = bytearray(512)
    dev.execute(b'\x06\x0b\x00\x00\x00\x00\x00\x00\x01', None, data)

    for i in range(0, len(data), 16):
        print(memoryview(data[i:i+0x10]).hex(' ', 1))

    #---------- Run ----------#
    dev.execute(b'\x06\x13\x00\x00\x00\x00', None, None)

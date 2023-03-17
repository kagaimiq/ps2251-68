from scsiio import SCSIDev
from hexdump import hexdump
import argparse

ap = argparse.ArgumentParser(description='Phison executive')

ap.add_argument('--device', required=True,
                help='Path to the Phison PRAM device')

ap.add_argument('-p', '--pramhdr', action='store_true',
                help='Generate a BtPramCd header instead of using the one from the file')

ap.add_argument('-v', '--vecred', action='store_true',
                help='Generate the vector table to redirect the vectors from 0x4000+ to 0x4080+')

ap.add_argument('file',
                help='File to run')

args = ap.parse_args()

with SCSIDev(args.device) as dev:
    ###################################################
    # Get the NAND IDs (why not)                      #
    ###################################################

    data = bytearray(512)
    dev.execute(b'\x06\x04\x00\x00\x00\x00', None, data)

    for i in range(8):
        print('CS%d NAND: %s' % (i, data[(i*0x10):(i*0x10)+0x6].hex('-', 1)))

    print()

    ###################################################
    # Send PRAM code                                  #
    ###################################################

    with open(args.file, 'rb') as f:
        # Get the BtPramCd header
        if args.pramhdr:
            # Make the BtPramCd header
            header = bytearray(512)
            header[0:8] = b'BtPramCd'
            header[16] = 0x20
            header = bytes(header)
        else:
            # Grab the first 512 bytes of the file as a header
            header = f.read(512)

        # Send the header
        #  06 11 03 -- -- -- -- ss:SS
        dev.execute(b'\x06\x11\x03\x00\x00\x00\x00\x00\x01', header, None)

        ### ### ### ### ### ### ### ### ### ### ### ### ###

        data = f.read()

        # Make the jump vectors that jump 128 bytes ahead
        # (as the code is loaded to 0x4000 but called from 0x4080)
        if args.vecred:
            vecs = bytearray(0x80)

            for i in range(5):
                off = 3 + i * 8
                vecs[off+0] = 0x80 # SJMP
                vecs[off+1] = len(vecs) - 2

            data = bytes(vecs) + data

        # Align the data length
        n = 0x8000
        if len(data) < n:
            data = data + (b'Mizu' * (n // 4))[len(data):]
        else:
            data = data[:n]

        # Send the data
        #  06 11 02 -- -- -- -- ss:SS
        dev.execute(b'\x06\x11\x02\x00\x00\x00\x00' + int.to_bytes(len(data) // 2, 2, 'little'), data, None)

    ###################################################
    # Read the data back                              #
    ###################################################

    """
    for i in range(0, 0x4000, 0x1000):
        data = bytearray(512)

        # Read the data!
        #  06 0B -- aa:AA -- -- ss:SS
        dev.execute(b'\x06\x0b\x00' + i.to_bytes(4, 'little') + b'\x00\x01', None, data)

        hexdump(data)
    """

    ###################################################
    # Execute!                                        #
    ###################################################

    dev.execute(b'\x06\x13\x00\x00\x00\x00', None, None)

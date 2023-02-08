TARGET = phison2268

OBJS = \
	test.rel

CFLAGS  = --model-small
ASFLAGS = 
LDFLAGS = --model-small --code-loc 0x4080 --code-size 0x5f80 --xram-loc 0x0000 --xram-size 0x2000

all: $(TARGET).ihx

%.rel: %.c
	sdcc $(CFLAGS) -c -o $@ $<

%.rel: %.s
	sdas8051 $(ASFLAGS) -o $<

$(TARGET).ihx: $(OBJS)
	sdcc $(LDFLAGS) --out-fmt-ihx -o $@ $(OBJS)

$(TARGET).bin: $(TARGET).ihx
	sdobjcopy -Obinary -Iihex $< $@

clean:
	rm -f $(OBJS)
	rm -f $(addsuffix .asm, $(basename $(OBJS)))
	rm -f $(addsuffix .lst, $(basename $(OBJS)))
	rm -f $(addsuffix .sym, $(basename $(OBJS)))
	rm -f $(addsuffix .rst, $(basename $(OBJS)))
	rm -f $(TARGET).ihx
	rm -f $(TARGET).lk
	rm -f $(TARGET).map
	rm -f $(TARGET).mem
	rm -f $(TARGET).bin

run: $(TARGET).bin
	cat ../BtPramCd $(TARGET).bin > test.bin
	python3 ../runner.py --device /dev/sg3 test.bin

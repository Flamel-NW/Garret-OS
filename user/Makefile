CC := riscv64-unknown-elf-gcc

CFLAGS := -g -Wall -Wno-unused -Werror -std=c11
CFLAGS += -fno-builtin -nostdinc # 不使用C语言内建函数 不搜索默认路径头文件
CFLAGS += -fno-stack-protector #禁用堆栈保护
CFLAGS += -ffunction-sections -fdata-sections # 将每个函数或符号创建为一个sections, 其中每个sections名与function或data名保持一致
CFLAGS += -mcmodel=medany # https://blog.csdn.net/zoomdy/article/details/100699108
CFLAGS += -march=rv64imac_zicsr -mabi=lp64 # 指定RISC-V架构和ABI
CFLAGS += -fno-pic -fno-pie # 禁用位置无关代码 禁用位置无关可执行文件

LD := riscv64-unknown-elf-ld

LD_SCRIPT := -T user.ld

LD_FLAGS := -m elf64lriscv
LD_FLAGS += -nostdlib 
LD_FLAGS += --gc-sections

LCSRCS := $(wildcard lib/*.c ../lib/*.c)
LSSRCS := $(wildcard lib/*.S ../lib/*.S)

LOBJS := $(LCSRCS:.c=.o) $(LSSRCS:.S=.o)

INCLUDE := -I lib \
		   -I ../lib 

.PHONY: clean

%.out : %.o $(LOBJS)
	$(LD) $(LD_FLAGS) $(LD_SCRIPT) -o $@ $^

clean :
	rm -f *.out *.o $(LOBJS)

%.o : %.c
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<

%.o : %.S
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<

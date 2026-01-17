CC := riscv64-unknown-elf-gcc

CFLAGS := -g -Wall -Wno-unused -Werror -std=c11
CFLAGS += -fno-builtin -nostdinc # 不使用C语言内建函数 不搜索默认路径头文件
CFLAGS += -fno-stack-protector #禁用堆栈保护
CFLAGS += -ffunction-sections -fdata-sections # 将每个函数或符号创建为一个sections, 其中每个sections名与function或data名保持一致
CFLAGS += -mcmodel=medany # https://blog.csdn.net/zoomdy/article/details/100699108
CFLAGS += -march=rv64imac_zicsr -mabi=lp64 # 指定RISC-V架构和ABI
CFLAGS += -fno-pic -fno-pie # 禁用位置无关代码 禁用位置无关可执行文件
CFLAGS += $(DEFS)

LD := riscv64-unknown-elf-ld

LD_SCRIPT := -T kernel.ld

LD_FLAGS := -m elf64lriscv
LD_FLAGS += -nostdlib 
LD_FLAGS += --gc-sections  # 配合-ffunction-sections -fdata-sections, 不连接未使用的函数和符号sections, 从而减小可执行文件大小

QEMU := qemu-system-riscv64

QEMU_FLAGS := -machine virt -nographic 
QEMU_DEBUG_FLAGS := -s -S -monitor telnet:127.0.0.1:1235,server,nowait

TARGET := Garret-OS

INCLUDE := -I lib \
		   -I kernel/lib \
		   -I kernel/debug \
		   -I kernel/proc \
		   -I kernel/mm \
		   -I kernel/driver \
		   -I kernel/sync \
		   -I kernel/trap \
		   -I kernel/syscall

CSRCS := $(filter-out user/%, $(wildcard */*.c */*/*.c))
SSRCS := $(filter-out user/%, $(wildcard */*.S */*/*.S))

OBJS := $(CSRCS:.c=.o) $(SSRCS:.S=.o)

UBINS := $(wildcard user/*.out)

.PHONY: clean qemu debug lib touch

build-%: 
	$(V)$(MAKE) clean
	$(V)$(MAKE) -C user $*.out
	$(V)$(MAKE) "DEFS+=-DTEST=$*"

$(TARGET) : $(OBJS)
	$(LD) $(LD_FLAGS) $(LD_SCRIPT) -o $@ $^ --format=binary $(UBINS) --format=default

qemu : $(TARGET)
	$(QEMU) $(QEMU_FLAGS) \
		-kernel $(TARGET)

debug : $(TARGET)
	$(QEMU) $(QEMU_FLAGS) $(QEMU_DEBUG_FLAGS) \
		-kernel $(TARGET)

clean :
	$(V)$(MAKE) -C user clean
	rm -f $(OBJS) $(TARGET)

%.o : %.c
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<

%.o : %.S
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<


# IMG_TARGET := garret.img

# 下面的是南开的uCore-OS-riscv64的运行方法, 实测不好使
# 讨论之后猜测可能是我的这个版本的OpenSBI无法通过addr参数指定起始地址
# elf格式可以运行因为可以读取起始地址, 而bin文件读不出来

# OBJCOPY := riscv64-unknown-elf-objcopy

# OBJCOPY_FLAGS := -S # 不从源文件拷贝符号信息和 relocation 信息.
# OBJCOPY_FLAGS += -O binary # 指定输入目标为二进制文件

# $(IMG_TARGET) : $(TARGET)
# 	$(OBJCOPY) $^ $(OBJCOPY_FLAGS) $@

# qemu : $(IMG_TARGET)
# 	$(QEMU) $(QEMU_FLAGS) \
# 		-device loader,file=$(IMG_TARGET),addr=0x80200000
# # https://stackoverflow.com/questions/58420670/qemu-bios-vs-kernel-vs-device-loader-file

# debug : $(IMG_TARGET)
# 	$(QEMU) $(QEMU_FLAGS) $(QEMU_DEBUG_FLAGS) \
# 		-device loader,file=$(IMG_TARGET),addr=0x80200000

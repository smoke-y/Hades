//ref: https://brennan.io/2020/03/22/sos-block-device/

#define VIRTIO_MAGIC       0x74726976
#define VIRTIO_RESET       0
#define VIRTIO_ACK         1
#define VIRTIO_DRIVER      2
#define VIRTIO_STATUS_OK   8
#define VIRTIO_RING_SIZE   128
#define VIRTIO_DESC_F_NEXT 1
#define VIRTIO_DESC_F_WRITE 2
#define READ32(reg) (*(volatile s32*)&(reg))
#define WRITE32(reg, value) (*(volatile s32*)&(reg))=(value)

volatile char *virtioStart = (char*)0x10001000;
volatile char *virtioEnd   = (char*)0x10009000;

typedef struct{
	u64 addr;
	u32 len;
	u16 flags;
	u16 next;
} Descriptor;
typedef struct{
	u16 flags;
	u16 idx;
	u16 ring[VIRTIO_RING_SIZE];
	u16 event;
}Available;
typedef struct{
	u32 id;
	u32 len;
}UsedElem;
typedef struct{
	u16 flags;
	u16 idx;
	UsedElem ring[VIRTIO_RING_SIZE];
	u16 event;
}Used;
struct Queue{
	Descriptor desc[VIRTIO_RING_SIZE];
	Available avail;
	u8 padding[PAGE_SIZE - (sizeof(Descriptor)*VIRTIO_RING_SIZE + sizeof(Available))];
	Used used;
};
struct virtioRegs{
	u32 MagicValue;
	u32 Version;
	u32 DeviceID;
	u32 VendorID;
	u32 DeviceFeatures;
	u32 DeviceFeaturesSel;
	u8 _reserved0[8];
	u32 DriverFeatures;
	u32 DriverFeaturesSel;
	u32 DriverPageSize;
	u8 _reserved1[4];
	u32 QueueSel;
	u32 QueueNumMax;
	u32 QueueNum;
	u32 QueueAlign;
	u64 QueuePfn;
	u8 _reserved2[8];
	u32 QueueNotify;
	u8 _reserved3[12];
	u32 InterruptStatus;
	u32 InterruptACK;
	u8 _reserved4[8];
	u32 Status;
	u32 _reserved5[3];
	u32 QueueDescLow;
	u32 QueueDescHigh;
	u32 _reserved6[2];
	u32 QueueAvailLow;
	u32 QueueAvailHigh;
	u32 _reserved7[2];
	u32 QueueUsedLow;
	u32 QueueUsedHigh;
	u32 _reserved8[21];
	u32 ConfigGeneration;
	u32 Config[0];
}__attribute__((packed));

void memBar(){asm volatile("sfence.vma");};

#include "virtioBlk.c"

u8 virtioInit(){
    while(virtioStart < virtioEnd){
        struct virtioRegs *vregs = (struct virtioRegs*)virtioStart;
        virtioStart += 0x1000;
        if(READ32(vregs->MagicValue) != VIRTIO_MAGIC) continue;
        switch(READ32(vregs->DeviceID)){
            case 2: return virtioSetUpBlk(vregs);
        };
    };
	return FALSE;
};
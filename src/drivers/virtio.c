//ref: https://brennan.io/2020/03/22/sos-block-device/

#define VIRTIO_MAGIC 0x74726976
volatile char *virtioStart = (char*)0x10001000;
volatile char *virtioEnd   = (char*)0x10008000;

typedef volatile struct __attribute__((packed)) {
	u32 MagicValue;
	u32 Version;
	u32 DeviceID;
	u32 VendorID;
	u32 DeviceFeatures;
	u32 DeviceFeaturesSel;
	u32 _reserved0[2];
	u32 DriverFeatures;
	u32 DriverFeaturesSel;
	u32 _reserved1[2];
	u32 QueueSel;
	u32 QueueNumMax;
	u32 QueueNum;
	u32 _reserved2[2];
	u32 QueueReady;
	u32 _reserved3[2];
	u32 QueueNotify;
	u32 _reserved4[3];
	u32 InterruptStatus;
	u32 InterruptACK;
	u32 _reserved5[2];
	u32 Status;
	u32 _reserved6[3];
	u32 QueueDescLow;
	u32 QueueDescHigh;
	u32 _reserved7[2];
	u32 QueueAvailLow;
	u32 QueueAvailHigh;
	u32 _reserved8[2];
	u32 QueueUsedLow;
	u32 QueueUsedHigh;
	u32 _reserved9[21];
	u32 ConfigGeneration;
	u32 Config[0];
} virtioRegs;

#define READ32(reg) (*(volatile s32*)&(reg))
#define WRITE32(reg, value) (*(volatile s32*)&(reg))=(value)

void virtioSetUpBlk(virtioRegs *vregs){
    
};
void virtioInit(){
    while(virtioStart < virtioEnd){
        virtioRegs *vregs = (virtioRegs*)virtioStart;
        virtioStart += 0x1000;
        if(READ32(vregs->MagicValue) != VIRTIO_MAGIC) continue;
        //TODO: reset and ack
        switch(READ32(vregs->DeviceID)){
            case 2:
                virtioSetUpBlk(vregs);
                break;
        };
    };
};
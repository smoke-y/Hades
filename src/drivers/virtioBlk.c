#define DEFAULT_STATUS 69
#define SECTOR_SIZE 512
#define VIRTIO_BLK_F_RO 5
#define VIRTIO_BLK_T_IN 0
#define VIRTIO_BLK_T_OUT 1
#define VIRTIO_BLK_T_FLUSH 4
#define VIRTIO_BLK_T_DISCARD 11
#define VIRTIO_BLK_T_WRITE_ZEROES 13

typedef struct{
	u32 blkType;
	u32 reserved;
	u32 sector;
} HeaderBlk;
typedef struct{
	HeaderBlk header;
	u8 *data;
	u8 status;
	u16 head;
} RequestBlk;

u8 virtioSetUpBlk(struct virtioRegs *vregs){
	WRITE32(vregs->Status, VIRTIO_RESET);
	memBar();
	WRITE32(vregs->Status, READ32(vregs->Status) | VIRTIO_ACK);
	memBar();
	WRITE32(vregs->Status, READ32(vregs->Status) | VIRTIO_DRIVER);
	memBar();
	u32 driverFeatures = READ32(vregs->DeviceFeatures);
	WRITE32(vregs->DriverFeatures, driverFeatures);
	memBar();
	WRITE32(vregs->Status, READ32(vregs->Status) | VIRTIO_STATUS_OK);
	memBar();
	if(!(READ32(vregs->Status) & VIRTIO_STATUS_OK)){
		PANIC("Virtio did not accept our features :(");
		return FALSE;
	};
	WRITE32(vregs->QueueNum, VIRTIO_RING_SIZE);
	WRITE32(vregs->QueueSel, 0); //fence until all registers have been written
	memBar();
	u32 numOfPages = ceil((f64)sizeof(struct Queue)/PAGE_SIZE);
	struct Queue *queue = (struct Queue*)allocContHardPage(numOfPages);
	WRITE32(vregs->DriverPageSize, PAGE_SIZE);
	WRITE32(vregs->QueuePfn, (u32)((u64)queue/PAGE_SIZE));
	WRITE32(vregs->Status, READ32(vregs->Status) | VIRTIO_STATUS_OK);
	memBar();
	if(!(READ32(vregs->Status) & VIRTIO_STATUS_OK)){
		PANIC("Virtio not ok :(");
		return FALSE;
	};
	kprint("[+] virtio_blk driver is live\n");
	hades.blkDriver.queue    = queue;
	hades.blkDriver.regs     = vregs;
	hades.blkDriver.idx      = 0;
	hades.blkDriver.ackIdx   = 0;
	hades.blkDriver.reqMem   = allocHardPage();
	hades.blkDriver.reqBit   = 0;
	hades.blkDriver.readOnly = driverFeatures & (1 << VIRTIO_BLK_F_RO) != 0;
	if(hades.blkDriver.readOnly) kprint("    read only\n");
	else kprint("    read and write\n");
	return TRUE;
};

u16 fillNextDescriptor(Descriptor *desc){
	hades.blkDriver.idx = (hades.blkDriver.idx + 1) % VIRTIO_RING_SIZE;
	hades.blkDriver.queue->desc[hades.blkDriver.idx] = *desc;
	if(desc->flags & VIRTIO_DESC_F_NEXT){
		hades.blkDriver.queue->desc[hades.blkDriver.idx].next = (hades.blkDriver.idx+1)%VIRTIO_RING_SIZE;
	};
	return hades.blkDriver.idx;
};

u8 blockCmd(u8 write, u8 *buffer, u32 len, u32 off){
	if(hades.blkDriver.readOnly && write == TRUE){
		PANIC("Writing to a read only blk");
		return FALSE;
	};
	if(len % SECTOR_SIZE != 0){
		PANIC("length of buffer is not a multiple of SECTOR_SIZE(%d)", SECTOR_SIZE);
		return FALSE;
	};

	u32 freeOff = 0;
	while(TRUE){
		if(!IS_BIT(hades.blkDriver.reqBit, freeOff)){
			SET_BIT(hades.blkDriver.reqBit, freeOff);
			break;
		};
		freeOff++;
	};

	RequestBlk *reqBlk = ((RequestBlk*)hades.blkDriver.reqMem) + freeOff;
	reqBlk->header.sector = off / SECTOR_SIZE;
	reqBlk->header.blkType = VIRTIO_BLK_T_IN;
	if(write) reqBlk->header.blkType = VIRTIO_BLK_T_OUT;
	reqBlk->data = buffer;
	reqBlk->header.reserved = 0;
	reqBlk->status = DEFAULT_STATUS;

	Descriptor desc;
	desc.addr  = (u64)reqBlk;
	desc.len   = sizeof(HeaderBlk);
	desc.flags = VIRTIO_DESC_F_NEXT;
	desc.next  = 0;
	u32 headIdx = fillNextDescriptor(&desc);

	desc.addr  = (u64)buffer;
	desc.len   = len;
	desc.flags = VIRTIO_DESC_F_NEXT | (write?VIRTIO_DESC_F_WRITE:0);
	desc.next  = 0;
	fillNextDescriptor(&desc);

	desc.addr  = (u64)&(reqBlk->status);
	desc.len   = sizeof(reqBlk->status);
	desc.flags = VIRTIO_DESC_F_WRITE;
	desc.next  = 0;
	fillNextDescriptor(&desc);

	hades.blkDriver.queue->avail.ring[hades.blkDriver.queue->avail.idx % VIRTIO_RING_SIZE] = headIdx;
	hades.blkDriver.queue->avail.idx = (hades.blkDriver.queue->avail.idx + 1) % VIRTIO_RING_SIZE;
	WRITE32(hades.blkDriver.regs->QueueNotify, 0);

	return TRUE;
};
u8 virtioSetUpBlk(struct virtioRegs *vregs){
	WRITE32(vregs->Status, VIRTIO_RESET);
	memBar();
	WRITE32(vregs->Status, READ32(vregs->Status) | VIRTIO_ACK);
	memBar();
	WRITE32(vregs->Status, READ32(vregs->Status) | VIRTIO_DRIVER);
	memBar();
	u32 driverFeature = READ32(vregs->DeviceFeatures);
	WRITE32(vregs->DriverFeatures, driverFeature);
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
	u32 numOfPages = (sizeof(struct Queue) + PAGE_SIZE-1)/PAGE_SIZE;
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
	hades.blkDriver.queue  = queue;
	hades.blkDriver.regs   = vregs;
	hades.blkDriver.idx    = 0;
	hades.blkDriver.ackIdx = 0;
	return TRUE;
};

#include "nemu.h"
#include "device/mmio.h"
#include "memory/mmu.h"

#define PTE_ADDR(pte)  ((uint32_t)(pte)&~0xfff)
//解析va
#define PDX(va) (((uint32_t)(va)>>22)&0x3ff)
#define PTX(va) (((uint32_t)(va)>>12)&0x3ff)
#define OFF(va) ((uint32_t)(va)&0xfff)

#define PMEM_SIZE (128 * 1024 * 1024)
#define PG_SIZE 4096
#define PRESENT 0x1
#define ACCESSED 0x20
#define DIRTY 0x40
#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
  int map_No=is_mmio(addr);
  if(map_No!=-1){
  	return mmio_read(addr,len,map_No);
  }
  return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  int map_No=is_mmio(addr);
  if(map_No!=-1){
  	mmio_write(addr,len,data,map_No);
	return;
  }
  memcpy(guest_to_host(addr), &data, len);
}
paddr_t page_translate(vaddr_t addr,bool iswrite){
  CR0 cr0=(CR0)cpu.cr0;
  if (cr0.paging&&cr0.protect_enable)
  {
    /* code */
    CR3 cr3=(CR3)cpu.cr3;

    //设置PDE
    PDE* pgdirs=(PDE*)PTE_ADDR(cr3.val);
    PDE pde=(PDE)paddr_read((uint32_t)(pgdirs+PDX(addr)),4);
    Assert(pde.present,"addr=0x%x",addr);

    //PTE
    PTE* ptab=(PTE*)PTE_ADDR(pde.val);
    PTE pte=(PTE)paddr_read((uint32_t)(ptab+PTX(addr)),4);
    Assert(pte.present,"addr=0x%x",addr);

    //accessed,dirty
    pde.accessed=1;
    pte.accessed=1;
    if (iswrite)
    {
      pte.dirty=1;
    }

    //根据偏移量转化为物理地址
    paddr_t paddr=PTE_ADDR(pte.val)| OFF(addr);
    return paddr;
  }
  return addr;
}

paddr_t page_translateqcj(vaddr_t addr, bool isRead){
/*
    present: 0 bit
    R/W: 1 bit
    U/D: 2 bit
    Access: 5 bit
    Dirty: 6 bit
    dir: 31 bit - 22 bit
    table: 21 bit - 12 bit
    each table item occupies 4 byte 
*/
  if(cpu.PG){//open the pte mode
    //find page table entry.
    paddr_t pde_base_addr = cpu.cr3;
    paddr_t pde_item_addr = pde_base_addr+((addr>>22)<<2);
    paddr_t pde_item = paddr_read(pde_item_addr,4);
    if(pde_item & PRESENT){
	    //find page frame entry
	    paddr_t pte_base_addr = pde_item & 0xFFFFF000;//take 20bit
	    paddr_t pte_item_addr = pte_base_addr + (((addr>>12)<<2) & 0x0FFF);
	    paddr_t pte_item = paddr_read(pte_item_addr,4);

	    if(pte_item & PRESENT){
        //get page address
        paddr_t page_base_addr = pte_item & 0xFFFFF000;
        paddr_t page_addr = page_base_addr + (addr & 0x0FFF);

        //set accessed
        pde_item |= ACCESSED;
        pte_item |= ACCESSED;

        //set dirty		
        if(!isRead){
            pde_item |= DIRTY;
            pte_item |= DIRTY;		
        }
        
        //update item
        paddr_write(pde_item_addr,4,pde_item);
        paddr_write(pte_item_addr,4,pte_item);
        
		    return page_addr;
	    }else{
	    	Log("Page Frame is not present.");
		    assert(0);
	    }

	  }else{
      Log("PTE is not present.");	
      assert(0);
    }
    
  }else{
    return addr;
  }
}




uint32_t vaddr_read(vaddr_t addr, int len) {
  if((addr&0x0FFF)+len>PG_SIZE){
    //data cross the page boundary
    
    int res=PG_SIZE-(addr&0x0FFF);

    paddr_t paddr=page_translate(addr,true);
    uint32_t low_data=paddr_read(paddr,res);

    paddr=page_translate(addr+res,true);
    uint32_t high_data=paddr_read(paddr,len-res);

    uint32_t data=low_data + (high_data<<(res<<3));
    return data;
  }else{
    paddr_t paddr=page_translate(addr,true);
    return paddr_read(paddr, len);
  }
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  if((addr&0x0FFF)+len>PG_SIZE){
    //data cross the page boundary
    int res=PG_SIZE-(addr&0x0FFF);

    paddr_t paddr= page_translate(addr,false);
    uint32_t low_data= data & ((1<<(res<<3))-1);

    paddr_write(paddr,res,low_data);
    paddr=page_translate(addr+res,false);
    uint32_t high_data=data>>(res<<3);

    paddr_write(paddr,len-res,high_data);

  }else{
    paddr_t paddr=page_translate(addr,false);
    paddr_write(paddr, len, data);
  }
}

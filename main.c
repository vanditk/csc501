/* user.c - main */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <paging.h>

int procA() {
	kprintf("procA lives\n");
	return 0;
}

int processVA() {
	kprintf("processVA lives\n");
	int *x;
	int temp;
	x = vgetmem(1000);  /* allocates some memory in the virtual heap which is in virtual memory */
	*x = 100;
	x++;
	*x = 200;
	temp = *x;  /* You are reading back from virtual heap to check if the previous write was successful */
	kprintf("data in virt mem of processVA = %d\n", temp);
   temp = vfreemem(--x, 1000); /* frees the allocation in the virtual heap */
   kprintf("vfree return %d\n", temp);
	return 0;
}

int procB() {
	char *addr = (char*) 0x60000000; //1G
	int i = ((unsigned long) addr) >> 12;	// the ith page
	int result;
	kprintf("procB lives\n");
	bsd_t bs = 1;
	result = get_bs(bs, 100);
	if(result == SYSERR){
		kprintf("procB get_bs call failed\n");
		return 0;
	}
	else
		kprintf("procB get_bs call result %d\n", result);

	if (xmmap(i, bs, 100) == SYSERR) {
		kprintf("procB xmmap call failed\n");
		return 0;
	}

	addr = (char*) 0x60000000; //1G
	for (i = 0; i < 10; i++) {
		kprintf("procB 0x%08x: %c\n", addr, *addr);
		addr += 4096;       //increment by one page each time
	}

	return 0;
}

/*------------------------------------------------------------------------
 *  main  --  user main program
 *------------------------------------------------------------------------
 */
int main() {

    int proc_A, proc_B, process_VA;
    int result;
	kprintf("\n\nHello World, Xinu lives\n\n");
	//srpolicy(FIFO);

	char *addr = (char*) 0x40000000; //1G
	bsd_t bs = 1;

	int i = ((unsigned long) addr) >> 12;	// the ith page

	result = get_bs(bs, 200);
	if(result == SYSERR){
		kprintf("get_bs call failed\n");
		return 0;
	}
	else
		kprintf("main get_bs call result %d\n", result);

	if (xmmap(i, bs, 150) == SYSERR) {
		kprintf("xmmap call failed\n");
		return 0;
	}

	for (i = 0; i < 10; i++) {
		*addr = 'A' + i;
		addr += NBPG;	//increment by one page each time
	}

	addr = (char*) 0x40000000; //1G
	for (i = 0; i < 10; i++) {
		kprintf("0x%08x: %c\n", addr, *addr);
		addr += 4096;       //increment by one page each time
	}

	proc_B = create(procB,INITSTK,INITPRIO,"procB",INITARGS);
	resume(proc_B);
	process_VA = vcreate(processVA,INITSTK,100,INITPRIO,"processVA",INITARGS);
	resume(process_VA);
	sleep(2);
	xmunmap(0x40000000 >> 12);
	release_bs(bs);
	kprintf("All izz well\n");
	return 0;
}

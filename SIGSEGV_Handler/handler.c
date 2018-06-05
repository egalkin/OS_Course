#define _GNU_SOURCE 1
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#define MEMORY_DUMP_SIZE 100

FILE* dump;

void dumpMemory(uintptr_t mem, int n) {
	int * p = (int *)(mem);
	int i = 0;
	fprintf(dump, "Memory dump:\n");
	for (i = 0; i < n; ++i) {
		fprintf(dump, "%d\n", p[i]);
	}
}

void dumpRegisters(void* vcontext) {
	ucontext_t* context = (ucontext_t*) vcontext;
	int n = sizeof(context->uc_mcontext.gregs)/sizeof(context->uc_mcontext.gregs[0]);
	fprintf(dump, "General purpose registers values: ");
	int i;
	for (i = 0; i < n; ++i) {
		fprintf(dump, "0x%08x " ,context->uc_mcontext.gregs[i]);
	}
	fprintf(dump, "\n");
}

void handler(int nSignum, siginfo_t* si, void* vcontext) {
	printf("Segmentation fault\n");
	printf("Signal %d received \n", nSignum);
	ucontext_t* context = (ucontext_t*)vcontext;
	int val;
	val = context->uc_mcontext.gregs[REG_RIP];
	printf("address = 0x%08x\n", val);
	dump = fopen("core_dump.out", "w");
	dumpRegisters(context);
	uintptr_t addr = val;
	dumpMemory(val,MEMORY_DUMP_SIZE);
	fclose(dump);
	abort();
}

void setHandler(void (*handler)(int, siginfo_t*, void*)) {
	struct sigaction action;
	action.sa_flags = SA_SIGINFO;
	action.sa_sigaction = handler;
	sigaction(SIGSEGV, &action, NULL);
}

void case1() {
	int* p = NULL;
	*p = 100;
}

void case2() {
	int *x = 0;
	int y = *x;
}

int main(int argc, char const* argv[]){
	setHandler(handler);
	case2();
}

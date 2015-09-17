/**
 * @file vloader.c
 * @author Sylvain Huet - 2006 - Initial version
 * @author RedoX <dev@redox.ws> - 2015 - GCC Port, cleanup
 * @date 2015/09/07
 * @brief VLISP Virtual Machine - Loader functions
 */
#include "common.h"
#include "vm/vmem.h"
#include "vm/vloader.h"

uint8_t *_bc_tabfun;
uint16_t _bc_nbfun;
int32_t _sys_start;
int32_t _global_start;

/**
 * @brief FIXME

 * @param [in/out]  *src  Address where to start reading
 *
 * @return Address when done reading
 */
uint8_t *loaderInitRec(uint8_t *src)
{
	int32_t l,i;

	l=loaderGetInt(src);
	src+=4;
	if (l==-1)
	{
		//		printf("nil\n",l>>1);
		VPUSH(NIL);
	}
	else if (l&1)
	{
		l>>=1;
		if (l&1)
		{
			l>>=1;
			//			printf("tuple %d\n",l);
			for(i=0;i<l;i++)
        src=loaderInitRec(src);
			VMKTAB(l);
		}
		else
		{
			l>>=1;
			//			printf("string taille %d\n",l);
			VPUSH(PNTTOVAL(VMALLOCSTR(src,l)));
			src+=l;
		}
	}
	else
	{
		//		printf("int int32_t %d\n",l>>1);
		VPUSH(l);
	}
	return src;
}

/**
 * @brief Get the Bytecode Size
 *
 * @param [in]  *src  Adress where the bytecode is stored
 *
 * @return  Size of the bytecode
 */
uint32_t loaderSizeBC(uint8_t *src)
{
	uint32_t n,b;
  /* Step 1: Global variables length*/
	n=loaderGetInt(src);
	src+=n;
  /* Step 2: Code length */
	b=loaderGetInt(src);
	src+=4+b;
	n+=4+b;
  /* Step 3: Functions table length */
	n+=2+(loaderGetShort(src)<<2);
  /* Done ! */
	return n;
}

/**
 * @brief Initialize the VM: load bytecode
 *
 * @param [in]  *src  Adress where the bytecode is stored
 */
void loaderInit(uint8_t *src)
{
	uint32_t n,nw,i;
	uint8_t* src0;
	uint8_t* dst;

	n=loaderSizeBC(src);               /* Read the Size of the Bytecode at *src */
  /* Copy the ROM Bytecode to RAM */
	dst=_bytecode;
	for(i=0;i<n;i++)
    dst[i]=src[i];
  /* FIXME: Find what this thing does */
	nw=(n+3)>>2;
	vmemInit(nw);

	/* FIXME: Find what this thing does */
	_sys_start=_vmem_stack-1;
	for(i=0;i<SYS_NB;i++)
    VPUSH(NIL);

  /* Load the global variables */
  src=_bytecode;
	src0=_bytecode;
    /* Get the global variables array size */
	n=loaderGetInt(src);
    /* Drop this offset */
	src+=4;
    /* Load the global variables in the Stack */
	_global_start=_vmem_stack-1;
	while(((uint32_t)(src-src0))<n)
    src=loaderInitRec(src);

  /* "Load" (actually, skip) the code */
	n=loaderGetInt(src);
    /* Skip code size */
	src+=4;

  /* Load the Functions table */
    /* */
	_bc_tabfun=&_bytecode[n+2];
    /* */
	_bc_nbfun=loaderGetShort(src+n);
	n+=2+(_bc_nbfun<<2);
	for(i=0;i<n;i++)
    _bytecode[i]=src[i];
	vmemSetstart((n+3)>>2);
}

/**
 * @brief Get the entrypoint for a function
 *
 * @param [in]  funnumber   ID/Number of the function
 *
 * @return  Start address of the function
 */
int32_t loaderFunstart(int32_t funnumber)
{
	return loaderGetInt(_bc_tabfun+(funnumber<<2));
}

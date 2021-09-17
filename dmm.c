#include <stdio.h>  // needed for size_t etc.
#include <unistd.h> // needed for sbrk etc.
#include <sys/mman.h> // needed for mmap
#include <assert.h> // needed for asserts
#include "dmm.h"

/* 
 * The lab handout and code guide you to a solution with a single free list containing all free
 * blocks (and only the blocks that are free) sorted by starting address.  Every block (allocated
 * or free) has a header (type metadata_t) with list pointers, but no footers are defined.
 * That solution is "simple" but inefficient.  You can improve it using the concepts from the
 * reading.
 */

/* 
 *size_t is the return type of the sizeof operator.   size_t type is large enough to represent
 * the size of the largest possible object (equivalently, the maximum virtual address).
 */

typedef struct metadata {
  size_t size;
  struct metadata* next;
  struct metadata* prev;
} metadata_t;

/*
 * Head of the freelist: pointer to the header of the first free block.
 */

static metadata_t* freelist = NULL;
static metadata_t* memSpaceStart = NULL;
static void* boundary = NULL;

void* dmalloc(size_t numbytes) {

  if(freelist == NULL) {
    if(!dmalloc_init()) {
      return NULL;
    }
  }

  assert(numbytes > 0);

  /* your code here */

  /*
  For a split, we first need to check whether the requested size is less than space available in the target block. 
  If so, the most memory-efficient approach is to allocate a block of the requested size by splitting it off of the target block, 
  leaving the rest of the target block free. The first block is returned to the caller and the second block remains in the freelist. 
  The metadata headers in both blocks must be updated accordingly to reflect their sizes.
  */

  // create some iterable metadata pointer
  metadata_t* iterable;
  iterable = freelist;
  boundary = (void*) (memSpaceStart + ALIGN(MAX_HEAP_SIZE));
  int iter = 0;
  // create some void* retPtr variable

  //printf("FL: %ld \n", (long) freelist);
  //printf("iterable: %ld \n", (long) iterable);
  //printf("memSpaceStart: %ld \n", (long) memSpaceStart);
  //printf("TC2:dmalloc(%d)\n", size)

  // the previous header still points to the header that was just allocated

  // while ( iterable is not null )
  while(iterable != NULL){
    if ( ALIGN(numbytes) < iterable->size ){
        size_t currSize = iterable->size;
        metadata_t* currPrev = iterable->prev;
        metadata_t* currNext = iterable->next;

        //iterable->size = currSize - ALIGN(numbytes);
        iterable->size = ALIGN(numbytes);
        //add aligned metadata size to current address to get start of ret address (set ret address as some void* pointer)
        //void* ptr = (void*) int_ptr + numbytes
        void* retPtr = (void*) (iterable + METADATA_T_ALIGNED);
        void* addr = (void*) (iterable);
        //void* retPtr = (void*)

        //need to make sure newHeaderPtr is not greater than original fl address + ALIGN(MAX_HEAP_SIZE)

        metadata_t* newHeaderPtr = (metadata_t*) (retPtr + ALIGN(numbytes));
        
        printf("iterable: %ld \n", (long) iterable);
        printf("return pointer from dmalloc: %ld \n", (long) retPtr);
        printf("num bytes aligned: %ld \n", (long) ALIGN(numbytes));
        printf("METADATA_T_ALIGNED: %ld \n", (long) METADATA_T_ALIGNED);
        printf("New header ptr: %ld \n", (long) newHeaderPtr);



        

        //printf("new Header: %ld \n", (long) newHeaderPtr);
        //printf("actual new header: %ld \n", (long) (iterable + METADATA_T_ALIGNED + ALIGN(numbytes)) );
        //printf("retPtr and numbytes: %ld \n", (long) (retPtr + ALIGN(numbytes)) );
        //printf("iterable: %ld \n", (long) iterable);

        //printf("newHeaderPtr: %ld \n", (long) newHeaderPtr);

        //printf("Memory space boundary: %ld \n", (long) (memSpaceStart + ALIGN(MAX_HEAP_SIZE)) );

        if ( newHeaderPtr < boundary ){
          newHeaderPtr->prev = currPrev;
          newHeaderPtr->next = currNext;
          if ( currNext != NULL ){
            currNext->prev = newHeaderPtr;
          }
          if ( currPrev != NULL ){
            currPrev->next = newHeaderPtr;
          }
          newHeaderPtr->size = (currSize - ALIGN(numbytes) - METADATA_T_ALIGNED);
          if ( iter == 0){
          // we have room for requested numbytes at the start of free mem space
          // we need to have freelist point to the new beginning of free mem space for the next time we look at mem
            freelist = newHeaderPtr;
          }
          //  currPrev.next should already be null if we have reached end of mem space

          //printf("new Header: %ld \n", (long) newHeaderPtr);
          //printf("iterable: %ld \n", (long) iterable);
          //printf("actual new header: %ld \n", (long) (iterable + METADATA_T_ALIGNED + ALIGN(numbytes)) );

        }
        else{
          if ( currPrev != NULL ){
            currPrev->next = currNext;
          }
        }
      
        return retPtr;
    }
    else{
      iterable = iterable->next;
    }
    iter ++;
  }

    // check if numbytes (aligned) is < freelist->size
      //if yes
        // we also need to keep track of currentOGSize
        // dereference pointer to metadata, then update size (w/ numBytes aligned)
        // we also need to keep track of currentprevious
        // 

        //update size of curr ptr
        //add aligned metadata size to current address to get start of ret address (set ret address as some void* pointer)

        //add aligned size of numBytes to get pointer to next block header

        // initialze new metadata pointer using above sum
            // set new metadataptr previous = currPrev
            // set new metadataptr.size = currentOGSize - (numBytes aligned) - metadataptrsizealigned
        //
      //iterable = iterable->next;

  return NULL;
}

void dfree(void* ptr) { 
  /* your code here */

  // we are passed ptr that points at first address in freed block

  // first, create meta_ptr var by subbing (ptr - META ALIGN)
  metadata_t*  meta_ptr = (metadata_t*) (ptr - METADATA_T_ALIGNED);

  // create iter pointer that points at start of freelist
  metadata_t* iterable = freelist;

  //printf("new Pointer: %ld \n", (long) (meta_ptr));
  //printf("iterable: %ld \n", (long) iterable);
  //printf("memSpaceStart: %ld \n", (long) memSpaceStart);
  //printf("TC2:dmalloc(%d)\n", size)

  // now we need to determmine --> is meta_ptr < iter OR is meta_ptr > iter

  if ( meta_ptr > iterable ){
    // if greater than:
    while ( iterable != NULL ){
      if ( iterable->next != NULL ){
        //if not null then  we need to see if iter < meta_ptr < iter->next (if this is true, insert mem block)
        if ( (iterable < meta_ptr) && (meta_ptr < iterable->next) ){
          // currNext = iter->next;
          metadata_t* iterNext = iterable->next;
          iterable->next = meta_ptr;
          iterNext->prev = meta_ptr;
          meta_ptr->prev = iterable;
          meta_ptr->next = iterNext;
          // set iter->next = to meta_ptr
          // currNext->prev = meta_ptr
          // meta_ptr->prev = iter;
          // meta_ptr->next = currNext;
          break;
        // break out of loop
        }
        else{
          iterable = iterable->next;
          continue;
        }   
      // if this is not true, then iter should become iter-> we should continue

      }
      else{
        // we are at the end of freelist --> the newly freed block must be placed at the end
        metadata_t* iterNext = iterable->next; // this is NULL
        iterable->next = meta_ptr;
        meta_ptr->prev = iterable;
        meta_ptr->next = iterNext; // now meta_ptr is at the end pointing at NULL
        break;
      }
    }
    //while ( iter != NULL )
    // check if iter->next is not null
      //if not null then  we need to see if iter < meta_ptr < iter->next (if this is true, insert mem block)
        // currNext = iter->next;
        // set iter->next = to meta_ptr
        //  currNext->prev = meta_ptr
        // meta_ptr->prev = iter;
        // meta_ptr->next = currNext;
        // 
        // break out of loop
      // if this is not true, then iter should become iter-> we should continue
    // if iter->next IS null
      // this is the last possible space in memblock where this freed block can fit
      // currNext = iter->next = NULL ;
      // set iter->next = to meta_ptr
      // meta_ptr->prev = iter;
      // meta_ptr->next = currNext;
      // break out of loop

  }
  else{
    //meta_ptr is less than iterable=freelist --> this means our freed block becomes the new first free block in freelist
    meta_ptr->next = iterable;
    meta_ptr->prev = iterable->prev; //this should be null
    iterable->prev = meta_ptr;
    freelist = meta_ptr;
    /*
    printf("meta: %ld \n", (long) meta_ptr);
    printf("iterable: %ld \n", (long) iterable);
    printf("if iterable is immediate, actual start of iterable: %ld \n", (long) (meta_ptr + METADATA_T_ALIGNED + meta_ptr->size) );
    */
  }
  
  // new coalesce()
    // I want to make sure that I always have enough space, so after each free
      // start at freelist
        // check if iter-> next is neighbor -> if yes, then coalesce, and continue
        // if no, then move to next block, and start loop again
  
  metadata_t* coalPtr = freelist;
  metadata_t* nextPtr = NULL;

  
  //printf("memSpaceStart: %ld \n", (long) memSpaceStart);
  //printf("TC2:dmalloc(%d)\n", size)

  while ( coalPtr != NULL ){

    nextPtr = (metadata_t*) (coalPtr + coalPtr->size + METADATA_T_ALIGNED);
    /*
    printf("coalesce block pointer: %ld \n", (long) coalPtr);
    printf("immediate Neighbor Start: %ld \n", (long) nextPtr);
    printf("actual next block: %ld \n", (long) (coalPtr->next) );
    */
    if ( coalPtr->next == NULL ){
      coalPtr = coalPtr->next;
      continue;
    }
    if ( coalPtr->next == nextPtr ){
      // next block must be immediate neighbor

      metadata_t* neighbor = coalPtr->next;

      coalPtr->size = (coalPtr->size + METADATA_T_ALIGNED + neighbor->size);

      metadata_t* neighborNext = neighbor->next;
      if ( neighborNext != NULL ){
        neighborNext->prev = coalPtr;
      }

      coalPtr->next = neighborNext;

      //coalesce, and then continue (don't increment)
        // need to set coalPtr->next->next = coalPtr->next;
        // need to increase size of coalptr-> size by nexts size + metadataAlignSize
        // need to check if coal->next->next != NULL
          // if not null, it need to point back at coalPtr
          // if null, then it is okay (coal can just point at NULL)
      continue;
    }
    else{
      // next block is not immediate neighbor --> go to next block, and then continue
      coalPtr = coalPtr->next;
      continue;
    }

  }


  /*
  // coalesce()
  // my freelist should be in sorted order based on how my freed nodes are inserted 
  // to coalesce
  metadata_t* prevNeighbor = meta_ptr->prev;
  metadata_t* nextNeighbor = meta_ptr->next;
  //once some block is freed, check neightboring blocks
  if ( prevNeighbor!= NULL ){
    // figure out if prev neighbor is immediate neighbor
    if ( (meta_ptr - prevNeighbor->size - METADATA_T_ALIGNED)==(prevNeighbor) ){
        // determine if nextNeighbor is not NULL
        if ( nextNeighbor != NULL ){
          if ( (meta_ptr + METADATA_T_ALIGNED + meta_ptr->size)==(nextNeighbor) ){
            metadata_t* nextNeighNext = nextNeighbor->next;
            prevNeighbor->size = (prevNeighbor->size + METADATA_T_ALIGNED + meta_ptr->size + METADATA_T_ALIGNED + nextNeighbor->size);
            prevNeighbor->next = nextNeighNext;
            if ( nextNeighNext != NULL ){
              nextNeighNext->prev = prevNeighbor;
            }
          }
          else{
            prevNeighbor->size = (prevNeighbor->size + METADATA_T_ALIGNED + meta_ptr->size);
            //metadata_t* metaNext = meta_ptr->next;
            prevNeighbor->next = nextNeighbor;
            nextNeighbor->prev = prevNeighbor;
          }
            // if not NULL --> check if nextNeigh is immediate neighbor && store nextNeighNext
            // if yes, join prevNeighbor, meta_ptr, and nextNeighbor sizes; point prevNeigh at nextneighnext 
            //then check if nextNeighNext not NULL
              // if not NULL, point nextNeighNext->prev at prevNeigh
        }
        else{
          prevNeighbor->size = (prevNeighbor->size + METADATA_T_ALIGNED + meta_ptr->size);
          //metadata_t* metaNext = meta_ptr->next;
          prevNeighbor->next = nextNeighbor; // prevNeighbor now points at NULL
          //nextNeighbor->prev = prevNeighbor;
        }    
          // if NULL OR not next is not immediate --> just join prevNeighbor and meta_ptr sizes, and then point prevNeigh at meta->next 
        
    }
    else{
      if ( nextNeighbor!=NULL){
        if ( (meta_ptr + METADATA_T_ALIGNED + meta_ptr->size)==(nextNeighbor) ){
          if ( nextNeighbor->next != NULL ){
            meta_ptr->size = (meta_ptr->size + METADATA_T_ALIGNED + nextNeighbor->size);
            metadata_t* nextNeighNext = nextNeighbor->next;
            meta_ptr->next = nextNeighNext;
            nextNeighNext->prev = meta_ptr;
            
          }
          else{
            meta_ptr->size = (meta_ptr->size + METADATA_T_ALIGNED + nextNeighbor->size);
            meta_ptr->next = nextNeighbor->next; //this should be null
          }
        }
      }
    }
      // if yes:
        // figure out if next neighbor is immediate neighbor
          // if yes, coalesce three blocks
          // if no, coalesce two blocks
        // check if nextNeighNext != NULL --> if not NULL, connect nextNeighNext->prev = prevNeighbor
      // if no:
        // I should check NexNeighbor independently (same code as if prevNeighbor == NULL  )
  }
  else{
    if ( nextNeighbor!=NULL){
      if ( (meta_ptr + METADATA_T_ALIGNED + meta_ptr->size)==(nextNeighbor) ){
        if ( nextNeighbor->next != NULL ){
          meta_ptr->size = (meta_ptr->size + METADATA_T_ALIGNED + nextNeighbor->size);
          metadata_t* nextNeighNext = nextNeighbor->next;
          meta_ptr->next = nextNeighNext;
          nextNeighNext->prev = meta_ptr;
          
        }
        else{
          meta_ptr->size = (meta_ptr->size + METADATA_T_ALIGNED + nextNeighbor->size);
          meta_ptr->next = nextNeighbor->next; //this should be null
        }
      }
    }
  }
  */
    // if meta->prev is not null --> check if meta - (meta->prev->size) - METAALIGNSIZE = meta->prev
      // if yes prev and meta a should be combined
      // now check if next should be combined --> if yes, combine the three blocks and update next and prev fields of prev block accordingly
      //      if no, combine prev and meta and update prev blocks accordingly
    // if meta-> prev is null --> check if meta->next is null
      // if not null --> check if meta + meta->size = meta->next
      // if true, meta and next should be coalesced
    // else we do not coalesce, we are done


  //If we keep the freelist in sorted order, coalescing two blocks is simple. 
  //You add the space of the second block and its metadata to the space in the first block. 
  //In addition, you need to unlink the second block from the freelist since it has been absorbed by the first block.
}


/*
 * Allocate heap_region slab with a suitable syscall.
 */
bool dmalloc_init() {

  size_t max_bytes = ALIGN(MAX_HEAP_SIZE);

  /*
   * Get a slab with mmap, and put it on the freelist as one large block, starting
   * with an empty header.
   */
  freelist = (metadata_t*)
     mmap(NULL, max_bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  if (freelist == (void *)-1) {
    perror("dmalloc_init: mmap failed");
    return false;
  }
  freelist->next = NULL;
  freelist->prev = NULL;
  freelist->size = max_bytes-METADATA_T_ALIGNED;
  memSpaceStart = freelist;


  return true;
}


/* for debugging; can be turned off through -NDEBUG flag*/
/*

This code is here for reference.  It may be useful.
Warning: the NDEBUG flag also turns off assert protection.


void print_freelist(); 

#ifdef NDEBUG
	#define DEBUG(M, ...)
	#define PRINT_FREELIST print_freelist
#else
	#define DEBUG(M, ...) fprintf(stderr, "[DEBUG] %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
	#define PRINT_FREELIST
#endif


void print_freelist() {
  metadata_t *freelist_head = freelist;
  while(freelist_head != NULL) {
    DEBUG("\tFreelist Size:%zd, Head:%p, Prev:%p, Next:%p\t",
	  freelist_head->size,
	  freelist_head,
	  freelist_head->prev,
	  freelist_head->next);
    freelist_head = freelist_head->next;
  }
  DEBUG("\n");
}
*/

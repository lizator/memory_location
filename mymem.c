#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "mymem.h"
#include <time.h>


/* The main structure for implementing memory allocation.
 * You may change this to fit your implementation.
 */

struct memoryList
{
  // doubly-linked list
  struct memoryList *last;
  struct memoryList *next;

  int size;            // How many bytes in this block?
  char alloc;          // 1 if this block is allocated,
                       // 0 if this block is free.
  void *ptr;           // location of block in memory pool.
};

strategies myStrategy = Best;    // Current strategy


size_t mySize;
void *myMemory = NULL;

static struct memoryList *head;
static struct memoryList *next;
static struct memoryList *curr;


/* initmem must be called prior to mymalloc and myfree.

   initmem may be called more than once in a given exeuction;
   when this occurs, all memory you previously malloc'ed  *must* be freed,
   including any existing bookkeeping data.

   strategy must be one of the following:
		- "best" (best-fit)
		- "worst" (worst-fit)
		- "first" (first-fit)
		- "next" (next-fit)
   sz specifies the number of bytes that will be available, in total, for all mymalloc requests.
*/

void initmem(strategies strategy, size_t sz)
{
	myStrategy = strategy;

	/* all implementations will need an actual block of memory to use */
	mySize = sz;

	if (myMemory != NULL) free(myMemory); /* in case this is not the first time initmem2 is called */

	if (head != NULL) { //Frees all
	    curr = head;
	    head = NULL;
	    next = NULL;
	    while (1){
	        if (curr->next != NULL) {
	            curr = curr->next;
	            free(curr->last);
	        } else {
	            free(curr);
	            curr = NULL;
                break;
	        }
	    }

	}

	/* TODO: release any other memory you were using for bookkeeping when doing a re-initialization! */


	myMemory = malloc(sz);
	
	head = (struct memoryList*) malloc(sizeof(struct memoryList)); //Init head with initial values
	head->next = NULL;
	head->last = NULL;
	head->size = mySize;
	head->alloc = 0;
	head->ptr = myMemory;

	if (myStrategy == Next) {
	    next = head;
	}


}

/* Allocate a block of memory with the requested size.
 *  If the requested block is not available, mymalloc returns NULL.
 *  Otherwise, it returns a pointer to the newly allocated block.
 *  Restriction: requested >= 1 
 */

// Søg funktion for Worst-Fit strategi.
// gotten from Volkan Isik s180103
struct memoryList* worstSearch(size_t size){
    struct memoryList *search = NULL;
    struct memoryList *biggestnode = NULL;
    search = head;
    int worstSize = 0;

    while (search!=NULL){
        if(search->size>=size && search->alloc==0){
            if(search->size > worstSize) {
                biggestnode = search;
                worstSize = search->size;
            }
        }
        search=search->next;
    }
    if(biggestnode != NULL)
        return biggestnode;
    else
        return NULL;
}

// Søg funktion for First-Fit strategi.
// gotten from Volkan Isik s180103
struct memoryList* firstSearch(size_t size){
    struct memoryList *search = NULL;
    search = head;

    while (search!=NULL){
        if(search->size >= size && search->alloc==0){
            return search;
        }
        search=search->next;
    }
    return NULL;
}

//Opretter en memoryblock og placerer den før eller overtager den block der blev givet.
// gotten from Volkan Isik s180103
struct memoryList* insert(struct memoryList* explode, size_t size){

    if(explode->size==size && explode->alloc==0){
        explode->alloc=1;
        if(explode->next==NULL){
            explode->ptr=explode->last->ptr+explode->last->size;
        }
        return explode;
    }

    //Opretter en ny node og placerer den i iforhold til given block
    struct memoryList *node = (struct memoryList*) malloc(sizeof(struct memoryList)*1);
    node->size=size;
    node->alloc=1;
    if(explode->last!=NULL)
        node->ptr=(explode->last->ptr+explode->last->size);
    else
        node->ptr=explode->ptr;
    explode->size-=size;

    if(explode->last != NULL){
        explode->last->next=node;
        node->last=explode->last;
        node->next=explode;
        explode->last=node;
    } else{
        node->last=NULL;
        node->next=explode;
        explode->last=node;
        head = node;
    }
    return node;
}

/* Denne metode indsætter en ny node efter den valgte node.  */
void insertNewNodeAfter(struct memoryList *trav, size_t requested, void *travPtr){

    if(trav == NULL){
        return;
    }

    /* Her bliver den nye node alloceret i vores hukommelse.  */
    struct memoryList *newNode = malloc(sizeof(struct memoryList));

    /* Hvis vores nodes next_node ikke er NULL kan pointerne blive opdeateret */
    if(trav -> next != NULL){
        newNode -> next = trav -> next;
        trav -> next -> last = newNode;

        newNode -> last = trav;
        trav -> next = newNode;
    }
        /* Hvis vores nodes next_node er lig med null er det slutningen af vores liste og pointerne bliver opdateret */
    else{
        trav -> next = newNode;
        newNode -> last = trav;
        newNode -> next = NULL;
        //tail = newNode;
    }

    /* Her sætter vi den nye nods parameter */
    newNode -> size = trav -> size - requested;
    newNode -> alloc = 0;
    newNode -> ptr = travPtr + requested;

    /* Her sætter vi den gamle nodes parametre.  */
    trav -> alloc = 1;
    trav -> size = requested;
}

void *mymalloc(size_t requested)
{
	assert((int)myStrategy > 0);

	if (requested > mem_largest_free()){
        return NULL;
	}
	
	switch (myStrategy)
	  {
	  case NotSet: 
	            return NULL;
	  case First:
      { //Gotten from Volkan Isik s180103
          struct memoryList *explode = firstSearch(requested);
          if(explode)
              return insert(explode,requested)->ptr;
          break;
      }
	  case Best:
	      curr = head; //Start at head
	      while (1) {
	          if (!curr->alloc && curr->size >= requested) { //If not allocated and have room to store requested do:
	              if (next != NULL) { //If another free block is found do:
	                  if (curr->size < next->size) { //If the new block is smaller than the first block do:
                          next = curr; //Save smallest available block possible
	                  }
	              } else { //If no block has been found yet
                      next = curr; //save first block found. continue to look for better options.
	              }
	          }
	          if (curr->next != NULL) { //If list not done
	              curr = curr->next; // go to next
	          } else
                  break; //else stop search
	      }
	      if (next->size == requested) { //If block found has exactly the right size
	          next->alloc = 1; //allocate it, without splitting it
	          void *temp = next->ptr;
	          next = NULL; //Release next for future user
              return temp;
	      } else { //else block has extra room and needs to be split
              struct memoryList *new = malloc(sizeof(struct memoryList)); //create new listitem with extra space left.
              new->size = next->size - requested;
              new->next = next->next;//put in list between foound block and the next one
              new->last = next;
              new->alloc = 0; //starts unallocated
              new->ptr = next->ptr + requested;
              next->size = requested; //update found block to be allocated with the right size
              next->next = new;
              next->alloc = 1;
              if (new->next != NULL) { //If new has a next, make next point back to new
                  new->next->last = new;
              }
              next = NULL; //Release next for future user
              return new->last->ptr;
	      }
	  case Worst:
      {
          //søger efter en fri memoryblok som er den største i hele memory'et
          struct memoryList *explode = worstSearch(requested);
          if(explode)
              //memoryblok placeres og pointeren returneres
              return insert(explode,requested)->ptr;
          break;
      }
	  case Next:
          while (curr -> size <= requested && curr -> alloc != 0){
              if(curr -> next == NULL){
                  curr = head;
              }
              else if(curr -> next == next){
                  return NULL;
              }
              else{
                  curr = curr -> next;
              }
          }

              /* Når vi så har fundet vores node kigger vi på om størrelsen passer. */
              if(curr -> size >= requested){

                  /* Hvis størrelsen passer præcist skal dens allocering bare sættes til 1. */
                  if(curr -> size == requested){
                      curr -> alloc = 1;
                      if(curr -> next == NULL){
                          next = head;
                      }
                      else{
                          next = curr;
                      }
                      return curr -> ptr;
                  }

                  /* Vi sætter den nye node ind. */
                  insertNewNodeAfter(curr, requested, curr -> ptr);

              }

              /* Her opdatere vi vores next pointer til det næste. Hvis den er ved slutningen bliver det sat til head ellers den næste. */
              if (curr -> next != NULL){
                  next = curr -> next;
              }
              else{
                  next = head;
              }
              return curr->ptr;
	  }
	return NULL;
}


/* Frees a block of memory previously allocated by mymalloc. */
void myfree(void* block)
{
    if (block == NULL) {
        return;
    }
    curr = head; //start at head
	while (1) { //loop though list to find block pointed at
	    if (curr->ptr == block && curr->alloc) { //If found block and allocated
	        curr->alloc = 0;                                    //unalocate (important if not merged into another

            if(curr->last != NULL && !curr->last->alloc) {      //combine with last if not allocated
                curr->last->size += curr->size;                 //add size to last
                curr->last->next = curr->next;                  //link last to next

                if(curr->next != NULL) {                        //If next is a link
                    curr->next->last = curr->last;              //link next to last

                    if (!curr->next->alloc) {                   //if it is not allocated, join with last
                        curr->last->size += curr->next->size;   //add size to last
                        curr->last->next = curr->next->next;    //link last to next next
                        if (curr->next->next != NULL) {         //if next next exists, link it to last
                            curr->next->next->last = curr->last;
                        }
                        free(curr->next);                       //Free next (cause removed from list)
                    }
                    free(curr);                                 //Free curr (cause removed from list)
                }
            } else if(curr->next != NULL && !curr->next->alloc) { //If last is null or allocated, check if next is null or allocated, and join if needed
                curr->size += curr->next->size;                 //put size in next
                if (curr->next->next != NULL) {
                    curr->next = curr->next->next;              //link current to next next
                    free(curr->next->last);                     //Free next
                    curr->next->last = curr;                    //link next to current.
                } else {
                    free(curr->next);                           //End of list, free last and make curr point to null
                    curr->next = NULL;
                }
            }
            return;
	    } else if (curr->next != NULL) { //go to next if not found yet
            curr = curr->next;
        } else {
            return;
	    }
	}
}

/****** Memory status/property functions ******
 * Implement these functions.
 * Note that when refered to "memory" here, it is meant that the 
 * memory pool this module manages via initmem/mymalloc/myfree. 
 */

/* Get the number of contiguous areas of free space in memory. */
int mem_holes()
{
    int res = 0;
    curr = head;
    while (1) {
        if (!curr->alloc) {
            res += 1;
        }
        if (curr->next != NULL) {
            curr = curr->next;
        } else {
            return res;
        }
    }

}

/* Get the number of bytes allocated */
int mem_allocated()
{
    int res = 0;
    curr = head;
    while (1) {
        if (curr->alloc) {
            res += curr->size;
        }
        if (curr->next != NULL) {
            curr = curr->next;
        } else {
            return res;
        }
    }

}

/* Number of non-allocated bytes */
int mem_free()
{
    int res = 0;
    curr = head;
    while (1) {
        if (!curr->alloc) {
            res += curr->size;
        }
        if (curr->next != NULL) {
            curr = curr->next;
        } else {
            return res;
        }
    }

}

/* Number of bytes in the largest contiguous area of unallocated memory */
int mem_largest_free()
{
    int res = 0;
    curr = head;
    while (1) {
        if (!curr->alloc && curr->size > res) {
            res = curr->size;
        }
        if (curr->next != NULL) {
            curr = curr->next;
        } else {
            return res;
        }
    }

}

/* Number of free blocks smaller than "size" bytes. */
int mem_small_free(int size)
{
    int res = 0;
    curr = head;
    while (1) {
        if (!curr->alloc && curr->size <= size) {
            res += 1;
        }
        if (curr->next != NULL) {
            curr = curr->next;
        } else {
            return res;
        }
    }

}       

char mem_is_alloc(void *ptr)
{
    curr = head;
    while (1) {
        if (curr->ptr == ptr) {
            return curr->alloc;
        } else {
            curr = curr->next;
        }
    }

}

/* 
 * Feel free to use these functions, but do not modify them.  
 * The test code uses them, but you may find them useful.
 */


//Returns a pointer to the memory pool.
void *mem_pool()
{
	return myMemory;
}

// Returns the total number of bytes in the memory pool. */
int mem_total()
{
	return mySize;
}


// Get string name for a strategy. 
char *strategy_name(strategies strategy)
{
	switch (strategy)
	{
		case Best:
			return "best";
		case Worst:
			return "worst";
		case First:
			return "first";
		case Next:
			return "next";
		default:
			return "unknown";
	}
}

// Get strategy from name.
strategies strategyFromString(char * strategy)
{
	if (!strcmp(strategy,"best"))
	{
		return Best;
	}
	else if (!strcmp(strategy,"worst"))
	{
		return Worst;
	}
	else if (!strcmp(strategy,"first"))
	{
		return First;
	}
	else if (!strcmp(strategy,"next"))
	{
		return Next;
	}
	else
	{
		return 0;
	}
}


/* 
 * These functions are for you to modify however you see fit.  These will not
 * be used in tests, but you may find them useful for debugging.
 */

/* Use this function to print out the current contents of memory. */
void print_memory()
{
    curr = head;
    int count = 0;
    while (1) {
        printf("listitem %d\n",count);
        count += 1;
        printf("size: %d, allocated: %s\n", curr->size, curr->alloc ? "true" : "false");
        printf("pointer %02x\n", curr->ptr);
        printf("--------------------------------\n");
        if (curr->next != NULL) {
            curr = curr->next;
        } else {
            return;
        }
    }
}

/* Use this function to track memory allocation performance.  
 * This function does not depend on your implementation, 
 * but on the functions you wrote above.
 */ 
void print_memory_status()
{
	printf("%d out of %d bytes allocated.\n",mem_allocated(),mem_total());
	printf("%d bytes are free in %d holes; maximum allocatable block is %d bytes.\n",mem_free(),mem_holes(),mem_largest_free());
	printf("Average hole size is %f.\n\n",((float)mem_free())/mem_holes());
}

/* Use this function to see what happens when your malloc and free
 * implementations are called.  Run "mem -try <args>" to call this function.
 * We have given you a simple example to start.
 */
void try_mymem(int argc, char **argv) {

    strategies strat;
	void *a, *b, *c, *d, *e;
    strat = Next;

	
	initmem(strat,500);
	
	a = mymalloc(100);
    print_memory();
    print_memory_status();
	b = mymalloc(100);
    print_memory();
    print_memory_status();
	c = mymalloc(100);
    print_memory();
    print_memory_status();
    d = mymalloc(100);
    print_memory();
    print_memory_status();
    e = mymalloc(100);
    print_memory();
    print_memory_status();
    myfree(b);
    print_memory();
    print_memory_status();
    myfree(d);
    print_memory();
    print_memory_status();
    myfree(c);
    print_memory();
    print_memory_status();


}



/*int main() { //main for debugging
    try_mymem(0, "best");
    return 0;
}*/
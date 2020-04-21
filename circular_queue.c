#include    <stdio.h>
#include    <string.h>
#include    <stdlib.h>

#define	    BUF_SIZE	0x0400 
#define	    RBUF_DFT_CAPACITY	64u

/* ERR STATUS CODE */
#define	    ERR_MALLOC_FAILED	-1

typedef struct rbuf {
  int *buf;
  size_t capacity;
  size_t size;
  size_t head;
  size_t tail;
} Rbuf;

void usage();

int rbuf_init(Rbuf *rbuf);

int rbuf_destruct(Rbuf *rbuf);

int rbuf_push(Rbuf *rbuf, int data);

int rbuf_pop(Rbuf *rbuf);

int rbuf_print(Rbuf *rbuf);

int rbuf_isfull(Rbuf *rbuf);

int rbuf_isempty(Rbuf *rbuf);

int main(){
    Rbuf rbuf;
    rbuf_init(&rbuf);

    char buf[BUF_SIZE]; 
    char cmd;
    while(fgets(buf, BUF_SIZE, stdin)){
	buf[strcspn(buf, "\r\n")] = '\0';

	cmd = *(buf + 1);

	switch(cmd){
	    case 'u':;
	      char *last_space_position = strrchr(buf, ' ');
	      int data = atoi(last_space_position + 1);
	      rbuf_push(&rbuf, data);
	      break;

	    case 'o':;
	      int ret = rbuf_pop(&rbuf);
	      if(ret < 0)
		fprintf(stderr, "The ring buffer is empty.\n");
	      break;

	    case 'r':
	      rbuf_print(&rbuf);
	      break;

	    default:
	      fprintf(stderr, "Unknown command. Please try again!\n");
	      usage();
	      break;
	}	
    }
    rbuf_destruct(&rbuf);
    return 0;
}

void usage(){
    printf("To push data: <push> <data>\n");
    printf("To pop data: <pop>\n");
    printf("To print all data: <print>\n");
    return;
}

int rbuf_init(Rbuf *rbuf){
   rbuf->buf = malloc(sizeof(int) * RBUF_DFT_CAPACITY);
   if(!rbuf->buf) return ERR_MALLOC_FAILED;
   rbuf->capacity = RBUF_DFT_CAPACITY;
   rbuf->size = 0;
   rbuf->head = 0;
   rbuf->tail = 0;
   return 0;
}

int rbuf_destruct(Rbuf *rbuf){
    free(rbuf->buf);
    return 0;
}

int rbuf_push(Rbuf *rbuf, int data){
    if(rbuf_isfull(rbuf)){
	printf("The ring buffer is full!\n");
	return EXIT_FAILURE;
    }
    rbuf->buf[rbuf->tail] = data;
    rbuf->tail = (rbuf->tail + 1) % rbuf->capacity;    
    rbuf->size++;

    return 0;
}

int rbuf_pop(Rbuf *rbuf){
    if(rbuf_isempty(rbuf)){
      printf("The ring buffer is empty!\n");
      return EXIT_FAILURE;
    }
    // invariant: the rbuf has entities
    int pop_val = rbuf->buf[rbuf->head];
    rbuf->head = (rbuf->head + 1) % rbuf->capacity;
    rbuf->size--;
    printf("Pop %d\n", pop_val);
    
    return 0;
}

int rbuf_print(Rbuf *rbuf){
    if(rbuf_isempty(rbuf)){
      printf("The ring buffer is empty!\n");
      goto end;
    }
    //invariant: the rbuf has entities
    printf("Ring buffer: ");
    for(size_t i = rbuf->head, counter = 0; counter < rbuf->size; (i = (i+1) % rbuf->capacity), counter++)
	printf("%d ", rbuf->buf[i]);
    printf("\n");

    end:;
    return 0;
}

int rbuf_isfull(Rbuf *rbuf){
    return ((rbuf->tail + 1) % rbuf->capacity == rbuf->head);
}

int rbuf_isempty(Rbuf *rbuf){
    return (rbuf->size == 0);
}

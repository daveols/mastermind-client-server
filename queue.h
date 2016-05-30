/*****************|
|** David Olsen **|
|**** dolsen *****|
|**** 641219 *****|
|*****************/
#ifndef QUEUE_H
#define QUEUE_H

typedef struct node {
  void *data; // hold some data
  struct node *next; // next node in the queue
} node;

node *node_create(); // create a new node
node *queue_add(node *head, void *data); // add a node to the back of the queue (return front)
void queue_insert_after(node *pos, void *data); // insert a node before the given position node (must supply pointer to head in case queue head changes)
void queue_insert_before(node **head, node *pos, void *data); // insert a node after the given position node
void *queue_peek(node *head); // peek at the data at the front of the queue
void *queue_pop(node **head); // delete node from the front of the queue and return its data
void *priority_queue_peek(node *h1, node *h2, node *h3); // peek at the highest priority item given queue heads (queues ordered by priority)
void *priority_queue_pop(node **h1, node **h2, node **h3); // pop the highest priority item given queue heads (queues ordered by priority)
int queue_len(node *head); // get length of given queue

#endif

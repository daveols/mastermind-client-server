/*****************|
|** David Olsen **|
|**** dolsen *****|
|**** 641219 *****|
|*****************/
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "queue.h"


node *
node_create(void *data) {
  node *np = (node *)malloc(sizeof(node));
  assert(np);
  np->data = data;
  np->next = NULL;
  return np;
}


node *
queue_add(node *head, void *data) {
  if(!head) {
    head = node_create(data);
  }
  else {
    // find tail
    node *last = head;
    while(last->next) {
      last = last->next;
    }
    last->next = node_create(data); // add to end
  }
  return head;
}


void
queue_insert_after(node *pos, void *data) {
  if (!pos) {
    return;
  }
  node *temp = pos->next;
  pos->next = node_create(data);
  pos->next->next = temp;
}


void queue_insert_before(node **head, node *pos, void *data) {
  if (!head || !*head || !pos) {
    return;
  }
  if (*head == pos) {
    // create new head
    node *new = node_create(data);
    new->next = *head;
    *head = new;
  }
  else {
    // otherwise loop from head to find prev node
    node *prev = *head;
    while(prev && prev->next != pos) {
      prev = prev->next;
    }
    if (!prev) {
      // couldn't find the node :(
      return;
    }
    queue_insert_after(prev, data);
  }
}


void *
queue_peek(node *head) {
  if (!head || !head->data) {
    return NULL;
  }
  return head->data;
}


void *
queue_pop(node **head) {
  if (!head || !*head) {
    return NULL;
  }
  node *prev = *head;
  void *data = (*head)->data;
  *head = (*head)->next;
  free(prev);
  return data;
}


void *
priority_queue_peek(node *h1, node *h2, node *h3) {
  if (queue_peek(h1)) {
    return queue_peek(h1);
  }
  if (queue_peek(h2)) {
    return queue_peek(h2);
  }
  return queue_peek(h3);
}


void *
priority_queue_pop(node **h1, node **h2, node **h3) {
  if (queue_peek(*h1)) {
    return queue_pop(h1);
  }
  if (queue_peek(*h2)) {
    return queue_pop(h2);
  }
  return queue_pop(h3);
}


int
queue_len(node *head) {
  if(!head) return 0;
  int i = 1;
  node *last = head;
  while(last->next) {
    last = last->next;
    i++;
  }
  return i;
}

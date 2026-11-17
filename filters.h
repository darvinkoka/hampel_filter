/*
 * filters.h
 *
 *  Created on: Apr 8, 2025
 *      Author: davidebattisti
 */

#ifndef INC_FILTERS_H_
#define INC_FILTERS_H_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>


typedef struct Node {
    double value;

    struct Node *nextFIFO;
    struct Node *prevFIFO;

    struct Node *nextSorted;
    struct Node *prevSorted;

    struct Node *nextDevSorted;
    struct Node *prevDevSorted;
} Node;

typedef struct {
    Node *fifoHead;
    Node *fifoTail;
    Node *sortedHead;
    Node *med;
    Node *center;
    Node *devSortedHead;
    Node *MAD;

    int count;
    double variance;
    double mean;
} Queue;

Node* createNode(double);
void insertSorted(Queue*, Node*);
void removeSorted(Queue*, Node*);
void insertDevSorted(Queue*, Node*);
void removeDevSorted(Queue*, Node*);
void initQueue(Queue*);
void printQueue(Queue*);
double findMedianFromHead(Queue*);
double findMADFromHead(Queue*);
void update_params(Queue*, double);
void update_med(Queue*);
void update_MAD(Queue*);
void enqueue(Queue*, double );
double hampel_filter(Queue*);


#endif /* INC_FILTERS_H_ */

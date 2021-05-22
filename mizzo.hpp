/*
    Name: Robert Choate
*/

#include <iostream>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <queue>


#ifndef PRODUCER_H
#define PRODUCER_H

#include "io.h"

const int MAXCANDY = 100;
const int MAXONBELT = 10;
const int MAXFROGS = 3;

const long NANOSEC = 1000000L;
const int MAX_MS = 999;
const int MS_TO_S = 1000;


// Structure for communicating data to increment/decrement producer threads ==> Might put info for both consumer and producer in here?
typedef struct {
    //const char* test;  // For debugging
    ProductType product; // Specify which candy type it is
    int* numOnBelt; // How many of each current candy are on the belt
    int N; // Number of milliseconds to wait for this candy type
    sem_t* mutexPtr; // Pointer to critical region semaphore
    sem_t* unconsumedPtr;
    sem_t* availableSlotPtr;
    sem_t* barrierPtr; // Used to exit the program
    std::queue<ProductType>* beltPtr; // Pointer to the conveyer belt
    int* candiesCreated; // Pointer to shared data with total # of candies created
} PRODUCER_DATA;

typedef struct {
    //const char* test; // For debugging
    ConsumerType consumer;
    int* numOnBelt; // How many of each current candy are on the belt
    int N; // Number of milliseconds to wait for this candy type
    sem_t* mutexPtr; // Pointer to critical region semaphore
    sem_t* unconsumedPtr;
    sem_t* availableSlotPtr;
    sem_t* barrierPtr; // Used to exit the program
    std::queue<ProductType>* beltPtr; // Pointer to the conveyer belt
    int* candiesConsumed; // To keep track of how many candies have been consumed
    int* hundred; // To keep track of 100 to 0
} CONSUMER_DATA;

#endif
/*
    Name: Robert Choate
    RedID: 823891010
    Account: cssc3308
    CS570, Spring 2021
    Assignment #4, mizzo
*/

#include "mizzo.hpp"

class Consumer {
public:
    int requiredTime = 0;
    ConsumerType consumer;

    // Artifacts from early implementation of project
    Consumer(int timeN, ConsumerType consume) {
        requiredTime = timeN;
        consumer = consume;
    }

    // Made for debugging
    void print() {
        std::cout << "req time = " << requiredTime << " consumer = " << consumer << std::endl;
    }

    // Main function of the file
    // Expects a pointer to CONSUMER_DATA which contains information on consumer thread
    static void * CandyConsumer(void* voidPtr) {

        // Typecast into a pointer of the expected type
        CONSUMER_DATA* consumerData = (CONSUMER_DATA*)voidPtr;
        
        // Setting up the struct for nanosleep
        struct timespec sleepTime;
        int millisec = consumerData->N;
        int seconds = 0;
        // Handling case that milliseconds entered exceed 1 second, 
        if(millisec > MAX_MS) {
            seconds = millisec / MS_TO_S;
            millisec = millisec % MS_TO_S;
        }
        sleepTime.tv_sec = seconds;
        long ms = millisec * NANOSEC;
        sleepTime.tv_nsec = ms; // NOTE: MUST BE IN RANGE OF 0 to 999999999, if statement above helps keep in this range

        while(true) {

            nanosleep(&sleepTime, NULL); // Always need to sleep before consuming a candy to simulate time required

            sem_wait(consumerData->unconsumedPtr); // Block until something to consume
            sem_wait(consumerData->mutexPtr); // Enter into critical region **********************************************

            // If this is 0, we have consumed 100 candies
            if(*(consumerData->hundred) == 0) {
                sem_post(consumerData->barrierPtr);
                pthread_exit(NULL);
            }

            ProductType candyConsumed = consumerData->beltPtr->front(); // Get candy type that is about to be popped off belt
            consumerData->beltPtr->pop(); // Pop candy off of belt queue

            *(consumerData->hundred) -= 1; // Subtract from 100 to 0
            consumerData->numOnBelt[candyConsumed] -= 1; // Take count of this candy coming off the belt
            consumerData->candiesConsumed[candyConsumed] += 1; // Take count of total candies consumed by this consumer

            io_remove_type(consumerData->consumer, candyConsumed, consumerData->numOnBelt, consumerData->candiesConsumed);

            sem_post(consumerData->mutexPtr); // Exit from critical region ***********************************************
            sem_post(consumerData->availableSlotPtr); // Increase available slots
        }
        pthread_exit(NULL);
    }
};
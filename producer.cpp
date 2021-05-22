/*
    Name: Robert Choate
    RedID: 823891010
    Account: cssc3308
    CS570, Spring 2021
    Assignment #4, mizzo
*/

#include "mizzo.hpp"

class Producer {
private:
    pthread_t producerThread;

public:
    int requiredTime = 0;
    ProductType product;

    // Artifact from early attempt at implementation
    Producer(int timeN, ProductType prod, pthread_t &prodThread) {
        requiredTime = timeN;
        product = prod;
        producerThread = prodThread;
    }

    // Used for debugging
    void print() {
        std::cout << "req time = " << requiredTime << " product = " << product << std::endl;
    }

    // Main function of this file
    // Expects a pointer to PRODUCER_DATA which contains information on producer thread
    static void * CandyGenerator(void* voidPtr) {

        // Typecast into a pointer of the expected type
        PRODUCER_DATA* producerData = (PRODUCER_DATA*)voidPtr;
        
        // Setting up the struct for nanosleep
        struct timespec sleepTime;
        int millisec = producerData->N;
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
            
            nanosleep(&sleepTime, NULL); // Always need to sleep before producing a candy to simulate time required
            
            // 100 candies have been made
            if(producerData->candiesCreated[FrogBite] + producerData->candiesCreated[Escargot] == MAXCANDY) {
                sem_post(producerData->unconsumedPtr); // Inform consumer item is ready to consume
                pthread_exit(NULL);
            }

            ProductType candyMade = producerData->product; // Get the candy type
            
            // Checking if there are 10 candies currently on the belt 
            if(producerData->numOnBelt[FrogBite] + producerData->numOnBelt[Escargot] == MAXONBELT) {
                sem_post(producerData->unconsumedPtr); // Inform consumer item is ready to consume
            }
            // Checking if there are 3 Frog candies on the belt
            else if(candyMade == FrogBite && producerData->numOnBelt[candyMade] == MAXFROGS) {
                sem_post(producerData->unconsumedPtr); // Inform consumer item is ready to consume
            }
            else { // Fall through main logic

                sem_wait(producerData->availableSlotPtr); // Make sure we have room in buffer
                sem_wait(producerData->mutexPtr); // Entering into critical region ******************************************
                
                producerData->beltPtr->push(candyMade); // Push candy into the belt queue
                producerData->candiesCreated[candyMade] += 1; // Take count of adding this candy to the belt
                producerData->numOnBelt[candyMade] += 1; // Take count of total candies created

                io_add_type(candyMade, producerData->numOnBelt, producerData->candiesCreated);

                sem_post(producerData->mutexPtr); // Exit from critical region **********************************************
                sem_post(producerData->unconsumedPtr); // Inform consumer item is ready to consume
            } 
        }
        pthread_exit(NULL);
    }
};
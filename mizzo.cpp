/*
    Name: Robert Choate
*/

#include "producer.cpp"
#include "consumer.cpp"
#include "io.c"

int main(int argc, char** argv) {
    
    int argFlags = 0;
    int ethelN = 0;
    int lucyN = 0;
    int frogN = 0;
    int snailN = 0;

    // To capture the arg flags,  NOTE: the ':' means that whatever is after the flag will be captured in optarg
    while((argFlags = getopt(argc, argv, "E:L:f:e:")) != -1) {
        switch(argFlags) {
            case 'E':
                // Check if the value passed after the flag is numeric, else exit
                if(std::isdigit(*optarg)) {
                    ethelN = std::atoi(optarg);
                }
                else {
                    std::cout << "Error: Must pass a numeric value after the flag -E.\n";
                    exit(EXIT_FAILURE);
                }
                break;
            case 'L':
                if(std::isdigit(*optarg)) {
                    lucyN = std::atoi(optarg);
                }
                else {
                    std::cout << "Error: Must pass a numeric value after the flag -L.\n";
                    exit(EXIT_FAILURE);
                }
                break;
            case 'f':
                if(std::isdigit(*optarg)) {
                    frogN = std::atoi(optarg);
                }
                else {
                    std::cout << "Error: Must pass a numeric value after the flag -f.\n";
                    exit(EXIT_FAILURE);
                }
                break;
            case 'e':
                if(std::isdigit(*optarg)) {
                    snailN = std::atoi(optarg);
                }
                else {
                    std::cout << "Error: Must pass a numeric value after the flag -e.\n";
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                std::cout << "Invalid flag or argument for flag.\n" <<
                    "Flags: -E N, -L N, -f N, -e N, where N is the number of milliseconds to wait." << std::endl;
                    exit(EXIT_FAILURE);
        }
    }

    // Ensuring correct values are captured per flag
    /*std::cout << "ethelN = " << ethelN << std::endl;
    std::cout << "lucyN = " << lucyN << std::endl;
    std::cout << "frogN = " << frogN << std::endl;
    std::cout << "snailN = " << snailN << std::endl;*/

    // Thread declarations & their data
    pthread_t frogThread;
    pthread_t snailThread;
    pthread_t lucyThread;
    pthread_t ethelThread;

    // Making producer and consumer objects for testing (Not required but was used in testing)
    /*ProductType frog = FrogBite;
    ProductType snail = Escargot;
    Producer* producer1 = new Producer(frogN, frog, frogThread);
    producer1->print();
    Producer* producer2 = new Producer(snailN, snail, snailThread);
    producer2->print();

    ConsumerType lucy = Lucy;
    ConsumerType ethel = Ethel;
    Consumer* consumer1 = new Consumer(lucyN, lucy);
    consumer1->print();
    Consumer* consumer2 = new Consumer(ethelN, ethel);
    consumer2->print();*/

    // Semaphore declarations
    sem_t barrier; // Used to help exit the program
    sem_init(&barrier, 0, 0); // Initialize barrier to 0

    sem_t mutex; // Critical region semaphore, to be used to give safe access to shared values
    sem_init(&mutex, 0, 1); // Initialize the mutex semaphore to 1
   
    sem_t unconsumed; // Semaphore for items in buffer  ==> AKA FULL IN TEXTBOOK
    sem_init(&unconsumed, 0, 0); // Initialize value to 0 

    sem_t avalaibleSlots; // Semaphore for space in buffer ==> AKA EMPTY IN TEXTBOOK
    sem_init(&avalaibleSlots, 0, MAXCANDY); // Initialize to 100

    std::queue<ProductType> belt; // Initialize the conveyerbelt queue (FIFO) to share between producer/consumers
    
    // Setting up the data that the threads will use
    // Array that keeps track of each type of candy created
    int createdArray[2];
    int* candyCount = createdArray;
    candyCount[0] = 0;
    candyCount[1] = 0;

    // To keep track of how many of each type of candy has been consumed by Lucy
    int L_consumedArray[2];
    int* L_candyConsumed = L_consumedArray;
    L_candyConsumed[0] = 0;
    L_candyConsumed[1] = 0;

    // To keep track of how many of each type of candy has been consumed by Ethel
    int E_consumedArray[2];
    int* E_candyConsumed = E_consumedArray;
    E_candyConsumed[0] = 0;
    E_candyConsumed[1] = 0;

    // To keep track of how many of each candy are currently on the belt
    int candies[2];
    int* candyOnBelt = candies;
    candyOnBelt[0] = 0;
    candyOnBelt[1] = 0;

    // Initializing the producer structures
    PRODUCER_DATA* frogData = new PRODUCER_DATA; 
    frogData->candiesCreated = candyCount; // Assign shared data pointer
    frogData->mutexPtr = &mutex; // Assign critical region semaphore
    frogData->unconsumedPtr = &unconsumed; // Assign the semaphore to signal consumers to consume
    frogData->availableSlotPtr = &avalaibleSlots; // Assign the semaphore with buffer size
    frogData->beltPtr = &belt; // Assign a shared pointer to the conveyer belt queue
    frogData->product = FrogBite; // Assign candy type
    frogData->N = frogN; // Assign amount of time to wait
    frogData->numOnBelt = candyOnBelt; // Assign the list which tracks current candy on belt
    //frogData->test = "FROG_TEST"; // Used for debugging

    PRODUCER_DATA* snailData = new PRODUCER_DATA;
    snailData->candiesCreated = candyCount;
    snailData->mutexPtr = &mutex;
    snailData->unconsumedPtr = &unconsumed;
    snailData->availableSlotPtr = &avalaibleSlots;
    snailData->beltPtr = &belt;
    snailData->product = Escargot;
    snailData->N = snailN;
    snailData->numOnBelt = candyOnBelt;
    //snailData->test = "SNAIL_TEST";

    int hundred = MAXCANDY;
    // Initializing the consumer structures
    CONSUMER_DATA* lucyData = new CONSUMER_DATA;
    lucyData->candiesConsumed = L_candyConsumed; // Assign the list of candies consumed by this consumer
    lucyData->mutexPtr = &mutex; // Assign critical region semaphore
    lucyData->unconsumedPtr = &unconsumed; // Assign the semaphore to signal consumers to consume
    lucyData->availableSlotPtr = &avalaibleSlots; // Assign the semaphore with buffer size
    lucyData->barrierPtr = &barrier; // Assign pointer to the barrier semaphore
    lucyData->beltPtr = &belt; // Assign a shared pointer to the conveyer belt queue
    lucyData->consumer = Lucy; // Assign the consumer type
    lucyData->N = lucyN; // Assign amount of time to wait
    lucyData->numOnBelt = candyOnBelt; // Assign the list which tracks current candy on belt
    lucyData->hundred = &hundred; // Assign a variable that starts at the value of 100
    //lucyData->test = "LUCY_TEST";

    CONSUMER_DATA* ethelData = new CONSUMER_DATA;
    ethelData->candiesConsumed = E_candyConsumed;
    ethelData->mutexPtr = &mutex;
    ethelData->unconsumedPtr = &unconsumed;
    ethelData->availableSlotPtr = &avalaibleSlots;
    ethelData->barrierPtr = &barrier;
    ethelData->beltPtr = &belt;
    ethelData->consumer = Ethel;
    ethelData->N = ethelN;
    ethelData->numOnBelt = candyOnBelt;
    ethelData->hundred = &hundred;
    //ethelData->test = "ETHEL_TEST";

    // Create the threads for each consumer and producer
    pthread_create(&lucyThread, NULL, Consumer::CandyConsumer, lucyData);
    pthread_create(&ethelThread, NULL, Consumer::CandyConsumer, ethelData);
    pthread_create(&frogThread, NULL, Producer::CandyGenerator, frogData);
    pthread_create(&snailThread, NULL, Producer::CandyGenerator, snailData);
    
    // Have the barrier make us wait here until the threads finish their work
    sem_wait(&barrier);

    // Set the arguments for the io_production_report function
    int* a_consumedArray[2];
    int** consumedArrays = a_consumedArray;
    consumedArrays[0] = L_consumedArray;
    consumedArrays[1] = E_consumedArray;

    io_production_report(createdArray, consumedArrays);

    return 0;
}
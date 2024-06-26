#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int all_primes_found = 0;

void generator(int channel1_id){

    int num = 2;
    while (channel_put(channel1_id,num) == 0)
    {
        num ++;
    }
    
}

int isPrime(int num){
    for (int i = 2; i * i <= num; i++)
    {
        if (num % i == 0)
            return 0;
    }
    return 1;
}

void checker(int channel1_id,int channel2_id){

    int num;
    while (channel_take(channel1_id,&num) == 0 && !all_primes_found)
    {
        if(isPrime(num))
        {
            if(channel_put(channel2_id,num) != 0)
            {
                all_primes_found = 1;
            }
        }
    }
    channel_destroy(channel1_id);
}

void printer(int channel2_id){
    int num = 0;
    int counter = 0;
    while (counter < 100)
    {
        channel_take(channel2_id,&num);
        counter ++;
        printf("Prime number %d : %d \n", counter, num);
    }
    channel_destroy(channel2_id);

}


int main(int argc,char * argv[])
{
    int n = 3; // Number of checkers
    if (argc > 1)
    {
        n = atoi(argv[1]);
    }

    int channel1 = channel_create();
    int channel2 = channel_create();

    for(int i = 0; i < n ; i++) // Create checkers proccesses
    {
        if(fork() == 0)
        {
            checker(channel1,channel2);
            exit(0);
        }
    }

    if(fork() == 0) // Create printer proccess
    {
        printer(channel2);
        exit(0);
    }

    generator(channel1);

    return 1;
}
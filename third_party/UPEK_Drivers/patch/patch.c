#include <unistd.h>

//Referred to in library
void Sleep(int ms)
{
    usleep(ms*1000);
}

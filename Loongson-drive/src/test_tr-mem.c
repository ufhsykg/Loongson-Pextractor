#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h> 
#include <sys/mman.h>

#define MAP_SIZE        0x10000
#define AUDIO_REG_BASE  0x1fe10000

#define GPIO_EN         0x500
#define GPIO_OUT        0x510
#define GPIO_IN         0x520

//控制GPIO39
#define GPIO_PIN        55

int main(int argc, char **argv)
{
    int i;
    int dev_fd, offset, gpio_move;
	dev_fd = open("/dev/mem", O_RDWR | O_SYNC);      
//    int GPIO_PIN =  aoti(argv[1]);
	if (dev_fd < 0)  
	{
		printf("open(/dev/mem) failed.");    
		return -1;
	}  

	unsigned char *map_base=(unsigned char * )mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd, AUDIO_REG_BASE );

	printf("%x \n", *(volatile unsigned int *)(map_base + GPIO_EN)); //打印该寄存器地址的value
	printf("%x \n", *(volatile unsigned int *)(map_base + GPIO_OUT)); //打印该寄存器地址的value
    
    if(GPIO_PIN > 31) {
        offset = 4;
        gpio_move = GPIO_PIN - 32;
    } else {
        offset = 0;
        gpio_move = GPIO_PIN;
    }

	*(volatile unsigned int *)(map_base + GPIO_EN + offset) &= ~(1<<gpio_move);         //GPIO输出使能

    for(i=0;i<5000;i++) {
	    *(volatile unsigned int *)(map_base + GPIO_OUT + offset) |= (1<<gpio_move);     //输出高
	    usleep(370);
	    *(volatile unsigned int *)(map_base + GPIO_OUT + offset) &= ~(1<<gpio_move);    //输出底
	    usleep(370);
    }

	munmap(map_base,MAP_SIZE);//解除映射关系

	if(dev_fd)
		close(dev_fd);

	return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>

#define DEVICE_FILE "/dev/my_cdev0"
#define LENGTH      256

int main()
{
    system("clear");
    int fd, done = 0;
    char write_buf[LENGTH], read_buf[LENGTH];
    memset(read_buf,'\0', LENGTH);

    assert(fd = open(DEVICE_FILE, O_RDWR));

    printf("Testing %s\n",DEVICE_FILE);
    printf("%s", "Enter data: ");
    scanf("%s", write_buf);
    __fpurge(stdin);
    write(fd, write_buf, strlen(write_buf));
    printf("Written %ld bytes\n", strlen(write_buf));

    printf("%s", "Enter data: ");
    scanf("%s", write_buf);
    __fpurge(stdin);
    write(fd, write_buf, strlen(write_buf));
    printf("Added %ld bytes\n", strlen(write_buf));

    lseek(fd,0,0);

    printf("FILE POINTER MOVED TO BEGIN(LSEEK)\n");
    printf("Enter count to read and test driver: ");
    int count = 0;
    scanf("%d", &count);
    __fpurge(stdin);
    read(fd, read_buf, count);
    read_buf[count] = '\0'; 
    printf("Read data: %s\n", read_buf);

    printf("Read from current offset, enter count : ");
    scanf("%d", &count);
    __fpurge(stdin);
    read(fd, read_buf, count);
    read_buf[count] = '\0'; 
    printf("Read data: %s\n", read_buf);

    lseek(fd,0,0);
    printf("FILE POINTER MOVED TO BEGIN(LSEEK)\n");
    read(fd, read_buf, LENGTH);
    printf("Read all: %s\n", read_buf);
    printf("Bye!...\n");
    close(fd);
    getchar();
}

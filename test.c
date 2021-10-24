#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio_ext.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#define DEVICE_FILE "/dev/mychardev-0"
#define LENGTH      256
#define READ        'r'
#define READ_ALL    'a'
#define WRITE       'w'
#define QUIT        'q'
#define FALSE        0
#define TRUE         !FALSE

char get_option() 
{
    char option;
    
    do {
        system("clear");
        printf("DEVICE OPTIONS:\n\n- Read(%c)\n- Read_all_file(%c)\n- Write(%c)\n- Quit(%c)\n", 
                READ, READ_ALL, WRITE, QUIT);
        option = tolower(getchar());
        __fpurge(stdin);    // очистка потока
    } while (option != READ && option != READ_ALL && option != WRITE && option != QUIT);

    return option;
}

int main()
{
    int fd, done = FALSE;
    char write_buf[LENGTH], read_buf[LENGTH];
    memset(read_buf,'\0', LENGTH);
    assert(fd = open(DEVICE_FILE, O_RDWR));
    int count = 0;
    while (!done) {
        switch(get_option()) {
        case READ:
            printf("Enter count to read from begin: ");
            scanf("%d", &count);
            __fpurge(stdin);
            read(fd, read_buf, count);
            printf("device: %s\n", read_buf);
            getchar();
            break;
        case READ_ALL:
            read(fd, read_buf, LENGTH);
            printf("device: %s\n", read_buf);
            getchar();
            break;
        case WRITE:
            printf("%s", "Enter data: ");
            scanf("%1023[^\n]s", write_buf);
            __fpurge(stdin);
            write(fd, write_buf, strlen(write_buf));
            break;
        case QUIT:
            done = TRUE;
            break;
        }
    }
    close(fd);
    return 0;
}

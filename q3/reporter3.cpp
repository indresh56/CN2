#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <iostream>
using namespace std;
int main()
{
    int fd = open("Rep3", O_WRONLY);
    char s[200];
    fgets(s, sizeof(s), stdin);
    // cout << s << endl;
    write(fd, s, strlen(s));
    close(fd);
}
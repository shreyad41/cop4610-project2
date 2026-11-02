#include <unistd.h>
#include <sys/syscall.h>

int main(void) 
{
    syscall(SYS_getpid);
    syscall(SYS_getppid);
    syscall(SYS_getuid);
    syscall(SYS_getgid);
    syscall(SYS_write, 1, "hello\n", 6);
    return 0;
}

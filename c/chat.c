/**
 * Author: hippowc
 * 
 * 基于udp的一对一聊天程序
 * 还需要对printf和scanf深入了解下
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

#define UDP_PORT 7080
#define UDP_ADDR "127.0.0.1"

void* recving_msg(int*);

int main(int argc, char const *argv[])
{
    char input_buff[1024];
    memset(input_buff, 0, sizeof(input_buff));
    printf("input target ip:(like 127.0.0.1) ");
    /* 输入最好使用scanf，fgets可以用于输入字符串，输入其他类型时需要注意转换 */
    /* 目前scanf的默认输入还有些问题，先不做吧 */
    scanf("%s", input_buff);

    in_addr_t addr_int;
    if (input_buff[0] == '\n') {
        addr_int = (in_addr_t)inet_addr(UDP_ADDR);
    } else
    {
        addr_int = (in_addr_t)inet_addr(input_buff);
    }
    
    int port = 0;
    printf("input target port:(like 7080) ");
    scanf("%d", &port);
    
    if (port == 0) {
        port = UDP_PORT;
    }

    struct sockaddr_in dest_sock_addr;
    dest_sock_addr.sin_family = AF_INET;
    dest_sock_addr.sin_port = htons(port);
    dest_sock_addr.sin_addr.s_addr = addr_int;

    int your_port = 0;
    printf("input your port:(like 7080) ");
    scanf("%d", &your_port);

    if (your_port == 0) {
        your_port = UDP_PORT;
    }
    
    struct sockaddr_in your_sock_addr;
    your_sock_addr.sin_family = AF_INET;
    your_sock_addr.sin_port = htons(0);
    your_sock_addr.sin_addr.s_addr = INADDR_ANY;

    /* 新线程监听消息 */
    pthread_t recv_t;
    pthread_create(&recv_t, NULL, &recving_msg, (int *)&your_port);

    /* 主线程发送消息 */
    int sock_fd = -1;
    sock_fd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        perror("init socket error");
        exit(-1);
    }

    int ret = bind(sock_fd, (struct sockaddr*)&your_sock_addr, sizeof(your_sock_addr));
    if (ret < 0) {
        perror("socket绑定本地地址失败");
        exit(-1);
    }

    int len_send_addr = sizeof(dest_sock_addr);
    char send_buff[1024];
    int flag = 1;
    while(flag){
        memset(send_buff, 0, sizeof(send_buff));
        fgets(send_buff, sizeof(send_buff), stdin);
        int result = strcmp(send_buff, "exit\n");
        if (result == 0) {
            flag = 0;
        }

        sendto(sock_fd, send_buff, sizeof(send_buff), 0, &dest_sock_addr, sizeof(dest_sock_addr));
    }
    
    return 0;
}

void* recving_msg(int *port_addr) {
    int port = *port_addr;
    struct sockaddr_in your_sock_addr;
    your_sock_addr.sin_family = AF_INET;
    your_sock_addr.sin_port = htons(port);
    your_sock_addr.sin_addr.s_addr = INADDR_ANY;

    int sock_fd = -1;
    sock_fd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        perror("init listening socket error");
        exit(-1);
    }

    int ret = bind(sock_fd, (struct sockaddr*)&your_sock_addr, sizeof(your_sock_addr));
    if (ret < 0) {
        perror("listening socket绑定本地地址失败");
        exit(-1);
    }

    char recv_buff[1024];
    struct sockaddr_in from_addr;
    from_addr.sin_family = AF_INET;
    from_addr.sin_port = htons(0);
    from_addr.sin_addr.s_addr = INADDR_ANY;
    int len_addr = sizeof(from_addr);

    ret = -1;
    while(1){
        ret = recvfrom(sock_fd, recv_buff, sizeof(recv_buff), 0, &from_addr, &len_addr);
        if (ret < 0) {
            perror("recv failed");
            exit(-1);
        }
        
        printf("-> %s", recv_buff);
    }
    
}

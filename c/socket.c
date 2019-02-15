/* 
 * author: hippowc
 * socket 实现函数
 * 
 * socket(协议方案，数据流/快形式，附加协议) 
 * 成功返回文件描述符：0,1,2分别表示标准输入、标准输出、标准错误。所以其他打开的文件描述符都会大于2, 错误时就返回 -1
 * 错误也被定义为：INVALID_SOCKET，其值为-1
*/

#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>

/* 以tcp方式创建socket */
int create_tcp_sock() {
    int sock_fd = -1;
    sock_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("init socket error");
        exit(-1); // 在 stdlib.h 中声明
    }
    return sock_fd;
    
}

/* 以udp方式创建socket */
int create_udp_sock() {
    int sock_fd = -1;
    sock_fd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        perror("init socket error");
        exit(-1);
    }
    return sock_fd;
}

/*
 * AF_前缀表示地址族（Address Family），而PF_前缀表示协议族（Protocol Family）。
 * 历史上曾有这样的想法：单个协议族可以支持多个地址族，PF_的值可以用来创建套接字，而AF_值用于套接字的地址结构。
 * 但实际上，支持多个地址族的协议族从来就没实现过，而头文件<sys/socket.h>中为一给定的协议定义的PF_值总是与此协议的AF_值相同。
*/

/**
 * htonl/htons就是把本机字节顺序转化为网络字节顺序，h---host 本地主机，to 就是to 了，n ---net 网络的意思，
 * l 是 unsigned long，"s"表示short
 * 所谓网络字节顺序（大尾顺序）就是指一个数在内存中存储的时候“高对低，低对高”（即一个数的高位字节存放于低地址单元，低位字节存放在高地址单元中）。
 * 但是计算机的内存存储数据时有可能是大尾顺序或者小尾顺序。
*/

/**
 * sock地址结构体
*/
#include <netinet/in.h>
typedef struct sockaddr_in sockaddr;

sockaddr create_sock_addr(int port_no) {
    sockaddr sock_addr;
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(port_no);
    sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    return sock_addr;
}

/*
 * bind(socket返回的socket,定义的SOCKADDR_IN 结构体，SOCKADDR_IN 结构体的大小)
 */
void bind_sock_addr(int sock_fd, int port_no) {
    sockaddr sock_addr = create_sock_addr(port_no);
    /*
     * bind(socket返回的socket,定义的SOCKADDR_IN 结构体，SOCKADDR_IN 结构体的大小)
    */
    int ret = -1;
    ret = bind(sock_fd, (struct sockaddr *)&sock_addr, sizeof(sock_addr));
    if (ret < 0) {
        perror("绑定socket失败");
        exit(-1);
    }
}

/**
 * 一个例子展示如何建立一个监听socket
*/
void handle_client(int);
int server_sock() {
    int sock_fd = create_tcp_sock();
    // 绑定地址
    bind_sock_addr(sock_fd, 8080);
    /*
     * listen(socket返回的socket,队列长度?)
    */
    int ret = -1;
    ret = listen(sock_fd, 1);
    if (ret < 0) {
        perror("监听失败");
        exit(-1);
    }

    /* 处理客户端socket */
    int client_socket_fd = -1;
    sockaddr name;
    socklen_t name_len = (socklen_t)sizeof(name);

    /*
     * accept （socket返回的socket，指向的定义的SOCKADDR_IN 结构体指针，指针的大小）
    */
    client_socket_fd = accept(sock_fd, (struct sockaddr *)&name, &name_len);
    if (client_socket_fd < 0) {
        perror("接受客户端失败");
        exit(-1);
    }

    handle_client(client_socket_fd);

    return sock_fd;
}

/**
 * 一个简单的处理函数
*/
#include <string.h>

void handle_client(int client_socket_fd) {
    char buffer[1024];
    
    memset(buffer, 0, sizeof(buffer));
    printf("begin to recieve \n");
    /*
     * recv/recvfrom (socket返回的socket,要放在的BUF，BUF长度，接收方式)
     * buf实际就是一小块内存区，用于临时存放发送的数据
     * 返回值 >0 返回收到的字节数目 =0断开连接 <0错误
    */
    int len = recv(client_socket_fd, buffer, sizeof(buffer), 0);
    printf("%d \n", len);
    fputs(buffer, stdout);

    /* 发送给客户端 */
    char *to_client = "ok \n";
    send(client_socket_fd, to_client, sizeof(to_client), 0);

    close(client_socket_fd);
}

int client_sock() {
    int sock_fd = create_tcp_sock();
    bind_sock_addr(sock_fd, 0);
    // 连接服务器的地址
    sockaddr sock_addr;
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(8080);
    sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    // sock_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int ret = -1;
    ret = connect(sock_fd, &sock_addr, sizeof(sockaddr));
    if (ret < 0) {
        perror("connect failed");
        exit(-1);
    }
    
    char buffer[1024] = {'h','e','l','l','o','\n'};
    send(sock_fd, buffer, sizeof(buffer), 0);

    char recv_buffer[1024];
    recv(sock_fd, recv_buffer, sizeof(recv_buffer), 0);

    fputs(recv_buffer, stdout);
    close(sock_fd);
}


/**
 * 以下使用udp方式进行收发信息，与tcp不同，udp不需要建立连接（不需要accept和listen）
 * 使用更简单，只需要将recv和send换成recv_from和send_to即可
 * 
*/

int udp_server() {
    int sock_fd = socket(PF_INET, SOCK_DGRAM, 0);
    bind_sock_addr(sock_fd, 8001);
    // 接受地址
    sockaddr c_addr = create_sock_addr(8002);
    int len = sizeof(c_addr);

    char recv_buffer[1024];
    int ret = -1;
    ret = recvfrom(sock_fd, recv_buffer, sizeof(recv_buffer), 0, &c_addr, &len);
    if (ret < 0) {
        perror("recv failed");
        exit(-1);
    }
    
    fputs(recv_buffer, stdout);

    char send_buffer[1024] = {'g', 'e', 't'};
    sendto(sock_fd, send_buffer, sizeof(send_buffer), 0, &c_addr, sizeof(c_addr));

    return 0;
}

int udp_client() {
    int sock_fd = socket(PF_INET, SOCK_DGRAM, 0);
    bind_sock_addr(sock_fd, 8002);
    // 发送地址
    sockaddr c_addr = create_sock_addr(8001);
    int len = sizeof(c_addr);

    char send_msg[1024] = {'t', 'e', 's', 't'};
    sendto(sock_fd, send_msg, sizeof(send_msg), 0, &c_addr, sizeof(c_addr));

    char recv_buffer[1024];
    recvfrom(sock_fd, recv_buffer, sizeof(recv_buffer), 0, &c_addr, &len);
    fputs(recv_buffer, stdout);

    return 0;
}

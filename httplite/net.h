#pragma once

int net_init();
void net_close();
void net_close_client(int client_fd);
int net_listen_port(int port);
int net_onconnect();
int net_read_package(int fd, char* buff, int len);
int net_send_package(int fd, char* buff, int len);

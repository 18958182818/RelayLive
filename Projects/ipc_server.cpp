// sever.cpp : �������̨Ӧ�ó������ڵ㡣
//
#include "uvIpc.h"
#include "utilc_api.h"
#include "stdio.h"

int main()
{
    uv_ipc_handle_t* h = NULL;
    int ret = uv_ipc_server(&h, "relay_live", NULL);
    if(ret < 0) {
        printf("ipc server err: %s\n", uv_ipc_strerr(ret));
    }
    sleep(INFINITE);
    return 0;
}
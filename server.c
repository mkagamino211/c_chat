#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>//追加：時刻
#define PORT 5000
#define SOCK_MAX 50
#define UNUSED (-1)

int main()
{
    int s[SOCK_MAX + 1];
    int n = 0;
    int len;
    fd_set readfds;
    int cllen;
    struct sockaddr_in saddr;
    struct sockaddr_in caddr;
    char str[1024], buf[1024];
    int i, j;
    int msglen;
    int optval;
    FILE *fp1;//ファイル用

    /* ソケットの生成 */
    if((s[0] = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("socket");
        exit(1);
    }

    printf("-- Server started!!\n", n);


    optval = 1;

    for(i=1;i<SOCK_MAX+1;i++){
        s[i]=UNUSED;
    }

    bzero((char *)&saddr, sizeof(saddr));

    saddr.sin_family=AF_INET;
    saddr.sin_addr.s_addr=INADDR_ANY;
    saddr.sin_port=htons(PORT);
    
    /* ソケットにアドレスとポート番号を結びつける */
    if((bind(s[0], (struct sockaddr *)&saddr, sizeof(saddr))) == -1){
        perror("bind");
        exit(1);
    }

    if((listen(s[0], SOCK_MAX)) == -1){
        perror("listen");
        exit(1);
    }

    /*
    追加：server_log.csv作成およびログを書き込み・保存
    server_log.csvが作成される。
    */
    fp1=fopen("server_log.csv","w");
    while(1){
        FD_ZERO(&readfds);

        for(i=0;i<SOCK_MAX;i++){
            if(s[i] != UNUSED){
                FD_SET(s[i], &readfds);
            }
        }
        
        if((n=select(FD_SETSIZE, &readfds, NULL, NULL, NULL)) == -1){
            perror("select");
            exit(1);
        }
        printf("--select returns: %d\n", n);

        for(i=1;i<SOCK_MAX;i++){
            if(s[i] != UNUSED){
                if(FD_ISSET(s[i], &readfds)){
                    printf("--s[%d] ready for reading\n", i);
                    bzero(str, sizeof(str));
                    if((msglen = read(s[i], str, sizeof(str))) == -1){
                        perror("read");
                        fclose( fp1 );//ファイルを閉じる
                        close(s[i]);
                        s[i]=UNUSED;
                        printf("Socket[%d] is changed to unwatch.\n", i);
                    }
                    else if(msglen != 0){
                        time_t t = time(NULL);//発言時刻を追加
                        printf("client[%d]: %s%s", i, str,ctime(&t));//発言時刻を追加
                        bzero(buf, sizeof(buf));
                        sprintf(buf, "client[%d]: %s%s", i, str,ctime(&t));//発言時刻を追加
                        fprintf( fp1 , buf );//server_log.csvへ
                        for(j=1;j<SOCK_MAX;j++){
                            if(s[j] != UNUSED){
                                write(s[j], buf, strlen(buf));
                            }
                        }
                    }
                    else{
                        printf("--client[%d]: connection closed.\n", i);
                        fclose( fp1 );//ファイルを閉じる
                        close(s[i]);
                        s[i] = UNUSED;
                    }
                }
            }
        }



        if(FD_ISSET(s[0], &readfds) != 0){
            printf("Accept New one.\n");
            len = sizeof(caddr);
            for(i=1;i<SOCK_MAX+1;i++){
                if(s[i] == UNUSED){
                    s[i]=accept(s[0], (struct sockaddr *)&caddr, &len);
                    printf("%d = accept()\n", s[i]);
                    if(s[i]==-1){
                        perror(NULL);
                        exit(1);
                    }
                    if(i == SOCK_MAX){
                        printf("refuse connection.\n");
                        strcpy(str, "Server is too busy.\n");
                        write(s[i], str, strlen(str));
                        s[i]=UNUSED;
                    }
                    i =SOCK_MAX+1;


                }
            }
        }
    }
}

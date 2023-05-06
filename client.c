#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/time.h>

#define PORT 5000

int main(int argc, char *argv[])
{
    struct sockaddr_in addr;
    struct hostent *hp;
    int fd, n;
    int len;
    char buf[1024];
    int ret;
    fd_set readfds;
    fd_set writefds;
    FILE *fp1;//(csv)ファイル用

    /* 相手のIPアドレス */
    if(argc != 2){
        printf("Usage: iclient SERVER NAME\n");
        exit(1);
    }

    /* ソケットの生成 */
    if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("socket");
        fprintf(stderr, "socket() error\n");
        exit(1);
    }

    bzero((char *)&addr, sizeof(addr));

    /* ホストデータベースからホスト名に対応するホスト情報を取得 */
    if((hp = gethostbyname(argv[1])) == NULL){
        perror("No such host");
        fprintf(stderr, "gethostbyname() error\n");
        exit(1);
    }
    bcopy(hp->h_addr, &addr.sin_addr, hp->h_length);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);

    /* ソケットの接続 */
    if(connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0){
        perror("connect");
        fprintf(stderr, "connect() error\n");
        exit(1);
    }

    /*
    追加：client_log.csv作成およびログを書き込み・保存
    client_log.csvが作成される。
    */
    fp1=fopen("client_log.csv","w");
    for(;;){
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        FD_SET(fd, &readfds);
        FD_SET(0, &readfds);

        /* 受信されない場合はタイムアウト */
        if((n = select(FD_SETSIZE, &readfds, NULL, NULL, NULL)) == -1){
            perror("select");
            fprintf(stderr, "select() error\n");
            exit(1);
        }

        printf("--Return value of select() = %d [%d]\n", n, __LINE__);

        if(FD_ISSET(fd, &readfds)){
            printf("--Data coming from SOCKET[%d]\n", __LINE__);
            bzero(buf, sizeof(buf));


            /* ソケットのデータ受信 */
            if((n = recv(fd, buf, 1024, 0)) < 0){
                fprintf(stderr, "Cannot receive message\n");
                close(fd);
                exit(1);
            }else if(n == 0){

                printf("\n\n--close received\n\n");
                printf("%s\n", buf);
                fflush(stdout);
                break;
            }else{
                printf("%s", buf);
                fprintf( fp1 , buf );//client_log.csvへ
                fflush(stdout);

            }
        }

        if(FD_ISSET(0, &readfds)){
            printf("--Data coming from KEYBOARD[%d]\n", __LINE__);
            fgets(buf, 1024, stdin);

            if(send(fd, buf, 1024, 0) < 0){
                fprintf(stderr, "Cannot send message\n");
                close(fd);
                exit(1);
            }
        }
    }
    fclose( fp1 );//ファイルを閉じる
    close(fd);
    exit(0);
}

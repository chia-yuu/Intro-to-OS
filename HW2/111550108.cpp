/*
Student No.: 111550108
Student Name: Chia-Yu Wu
Email: amberwu0411@gmail.com
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not 
supposed to be posted to a public server, such as a 
public GitHub repository or a public web page.
*/

#include<iostream>
#include<unistd.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/wait.h>
#include<sys/time.h>
using namespace std;

int main(){
    int dim;
    printf("Input the matrix dimension: ");
    scanf("%d", &dim);
    int sz = dim * dim * sizeof(int);

    // init
    int shm1 = shmget(1, sz, IPC_CREAT | 0666);
    int *A = (int*)shmat(shm1, NULL, 0);

    int shm2 = shmget(2, sz, IPC_CREAT | 0666);
    int *B = (int*)shmat(shm2, NULL, 0);

    int shm3 = shmget(3, sz, IPC_CREAT | 0666);
    int *C = (int*)shmat(shm3, NULL, 0);

    int val = 0;
    for(int i=0;i<dim;i++){
        for(int j=0;j<dim;j++){
            A[i*dim + j] = val;
            B[i*dim + j] = val;
            val++;
        }
    }
    // solve
    for(int n_process=1;n_process<=16;n_process++){
        struct timeval start, end;
        gettimeofday(&start, 0);
        unsigned checkSum = 0;

        // cal
        int all = dim / n_process;
        int more = dim % n_process;
        for(int id=0;id<n_process;id++){
            // printf("process %d\n", id);
            pid_t pid;
            pid = fork();
            if(pid < 0){
                printf("fork failed\n");
                exit(-1);
            }
            else if(pid == 0){
                // printf("in child, ");
                // c[i][j] += a[i][k] * b[k][j]
                int n_row = all;
                if(id < more){n_row++;}
                int start = id * all + min(id, more);
                // printf("i = %d, n_row = %d\n", i, n_row);
                for(int i = start;i<start+n_row;i++){
                    for(int j=0;j<dim;j++){
                        // C[i*dim + j] = 0;
                        int sum = 0;
                        for(int k=0;k<dim;k++){
                            sum += A[i*dim + k] * B[k*dim + j];
                        }
                        C[i*dim + j] = sum;
                        // printf("C[%d][%d] = %d, ", i, j, C[i*dim + j]);
                    }
                    // cout<<endl;
                }
                exit(0);
            }
            // else{
            //     // printf("parent\n");
            //     // waitpid(pid, NULL, 0);
            //     wait(NULL);
            // }

            // wait(NULL);
            // for(int i=0;i<dim;i++){
            //     for(int j=0;j<dim;j++){
            //         cout<<C[i*dim + j]<<", ";
            //     }
            //     cout<<endl;
            // }
        }

        for(int i=0;i<n_process;i++){
            wait(NULL);
        }

        for(int i=0;i<dim;i++){
            for(int j=0;j<dim;j++){
                checkSum += C[i*dim + j];
            }
        }
        
        // output
        gettimeofday(&end, 0);
        int sec = end.tv_sec - start.tv_sec;
        int usec = end.tv_usec - start.tv_usec;

        if(n_process == 1) cout<<"Multiplying matrices using "<<n_process<<" process"<<endl;
        else cout<<"Multiplying matrices using "<<n_process<<" processes"<<endl;
        cout <<"Elapsed time: "<< sec+(usec/1000000.0) << "sec, Checksum: "<< checkSum << endl;
    }

    // delete shm
    shmdt(A);
    shmdt(B);
    shmdt(C);
    shmctl(shm1, IPC_RMID, NULL);
    shmctl(shm2, IPC_RMID, NULL);
    shmctl(shm3, IPC_RMID, NULL);

    return 0;
}
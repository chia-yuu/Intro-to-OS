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
#include<vector>
#include<sstream>
#include<unistd.h>
#include<sys/wait.h>
#include<string.h>
#include<fcntl.h>
#include<signal.h>
using namespace std;

void execute(vector<string> &word){
    // convert word to c string
    int n = word.size();
    char **arg = new char *[n+1];
    for(int i=0;i<n;i++){
        arg[i] = new char[word[i].size()+1];
        strcpy(arg[i], word[i].c_str());
    }
    arg[n] = NULL;

    // execute
    execvp(arg[0], arg);
    exit(0);
}

void signalHandler(int signum) {
    // while (waitpid(-1, NULL, WNOHANG) > 0);
    wait(NULL);
}

int main(){
    while(true){
        // read input
        cout<<">";
        string cmd = "";
        getline(cin, cmd);
        if(cmd == ""){continue;}

        // get each word
        vector<vector<string>> word(2);
        stringstream ss(cmd);
        string w;
        char op = ' ';
        int idx = 0;
        while(getline(ss, w, ' ')){
            if(w == "&"){
                op = '&';
            }
            else if(w == ">" || w == "<" || w == "|"){
                op = w[0];
                idx++;
            }
            else{
                word[idx].push_back(w);
            }
        }

        // cout<<cmd<<endl<<op<<endl;
        if(word[0][0] == "exit"){break;}

        // exe
        pid_t pid;
        // run in the background
        if(op == '&'){
            // cout<<"in &"<<endl;
            signal(SIGCHLD, signalHandler);
            pid = fork();
            // error occurred
            if(pid < 0){
                printf("In &, fork failed\n");
                exit(-1);
            }
            // child process
            else if(pid == 0){
                execute(word[0]);
            }
        }
        // execute left, write output to right
        else if(op == '>'){
            // cout<<"in >"<<endl;
            // printf("not implemented\n");
            pid = fork();
            // error occurred
            if(pid < 0){
                printf("In >, fork failed\n");
                exit(-1);
            }
            // child process
            else if(pid == 0){
                int fd = creat(word[1][0].c_str(), 0644);
                if(fd == -1){
                    printf("In > fd, creat failed\n");
                    exit(-1);
                }
                // dup2(STDOUT_FILENO, fd);
                dup2(fd, STDOUT_FILENO);
                close(fd);
                execute(word[0]);
            }
            // parent process (wait child to finish)
            else{
                waitpid(pid, NULL, 0);
            }
        }
        // take right content as input of left
        else if(op == '<'){
            // cout<<"in <"<<endl;
            // printf("not implemented\n");
            pid = fork();
            // error occurred
            if(pid < 0){
                printf("In <, fork failed\n");
                exit(-1);
            }
            // child process
            else if(pid == 0){
                int fd = open(word[1][0].c_str(), 0644);
                if(fd == -1){
                    printf("In < fd, open failed\n");
                    exit(-1);
                }
                // dup2(STDOUT_FILENO, fd);
                dup2(fd, STDIN_FILENO);
                close(fd);
                execute(word[0]);
            }
            // parent process (wait child to finish)
            else{
                waitpid(pid, NULL, 0);
            }
        }
        // pipe
        else if(op == '|'){
            // cout<<"in |"<<endl;
            // printf("not implemented\n");
            int pipefd[2];  // 0: in, 1: out
            if(pipe(pipefd) == -1){
                printf("In | pipefd, pipe failed\n");
                exit(-1);
            }

            pid = fork();
            // error occurred
            if(pid < 0){
                printf("In pipe first process, fork failed\n");
                exit(-1);
            }
            // first process (left)
            else if(pid == 0){
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[0]);
                close(pipefd[1]);
                execute(word[0]);
            }
            // parent process (wait first process to finish)
            else{
                // wait for the first process
                waitpid(pid, NULL, 0);
                // second process (right)
                pid = fork();
                // error occurred
                if(pid < 0){
                    printf("In pipe second process, fork failed\n");
                    exit(-1);
                }
                // second process
                else if(pid == 0){
                    dup2(pipefd[0], STDIN_FILENO);
                    close(pipefd[0]);
                    close(pipefd[1]);
                    execute(word[1]);
                }
                // parent process (wait child to finish)
                else{
                    close(pipefd[0]);
                    close(pipefd[1]);
                    waitpid(pid, NULL, 0);
                }
            }
        }
        else{
            // cout<<"in default"<<endl;
            pid = fork();
            // error occurred
            if(pid < 0){
                printf("In default, fork failed\n");
                exit(-1);
            }
            // child process
            else if(pid == 0){
                execute(word[0]);
            }
            // parent process (wait child to finish)
            else{
                waitpid(pid, NULL, 0);
            }
        }
    }
    return 0;
}
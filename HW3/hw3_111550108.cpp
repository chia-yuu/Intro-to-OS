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
#include<fstream>
#include<pthread.h>
#include<semaphore.h>
#include<vector>
#include<sys/time.h>
#include<queue>
#include<algorithm>
using namespace std;

int arr_sz;
vector<int> arr, finish, part_sz, next_job;
vector<pair<int, int>> merge_part;
queue<int> job_q;
sem_t q_lock, cnt_lock, fns_event, arr_lock;

void bubble_sort(vector<int>& arr, int left, int right){
    // printf("in bubble sort\n");
    // cout<<left<<' '<<right<<"\n";
    for(int i=left;i<right;i++){
        for(int j=left;j<right-(i-left)-1;j++){
            if(arr[j] > arr[j+1]){
                swap(arr[j], arr[j+1]);
            }
        }
    }
}

void merge(vector<int>& arr, int left, int right){
    // cout<<left<<' '<<mid<<' '<<right<<'\n';
    int mid = left + (right-left)/2;
    // if(left == 0 && right == arr_sz){
    //     cout<<arr[50];
    //     return;
    //     cout<<"l, m, r"<<left<<' '<<mid<<' '<<right<<'\n';
    // }
    for(int i=left+1;i<right;i++){
        if(arr[i] < arr[i-1]){
            mid = i;
            break;
        }
    }
    // cout<<left<<' '<<mid<<' '<<right<<'\n';
    vector<int> l, r;
    for(int i=left;i<mid;i++){l.push_back(arr[i]);}
    for(int i=mid;i<right;i++){r.push_back(arr[i]);}
    int l_idx = 0, r_idx = 0, idx = left;
    int nl = l.size(), nr = r.size();
    // if(left == 0 && right == arr_sz){
    //     // cout<<r[0]<<' '<<arr[mid]<<"\n";
    //     cout<<mid<<"\n";
    // }
    while(l_idx < nl && r_idx < nr){
        if(l[l_idx] < r[r_idx]){
            arr[idx] = l[l_idx];
            idx++, l_idx++;
            // if(left == 0 && right == arr_sz){
            //     cout<<l[l_idx]<<' '<<r[r_idx]<<"\n";
            // }
        }
        else{
            arr[idx] = r[r_idx];
            idx++, r_idx++;
        }
    }
    while(l_idx < nl){arr[idx] = l[l_idx]; idx++, l_idx++;}
    while(r_idx < nr){arr[idx] = r[r_idx]; idx++, r_idx++;}
    // cout<<l_idx<<' '<<r_idx<<' '<<idx<<"\n";
}

void* worker(void* arg){
    while(true){
        sem_wait(&q_lock);
        if(job_q.empty()){
            sem_post(&q_lock);
            // sem_post(&fns_event);
            break;
        }
        int job = job_q.front();
        // int l = job.first, r = job.second;
        job_q.pop();
        // cout<<"current job = "<<job<<endl;
        // cout<<job;
        sem_post(&q_lock);

        // if(r - l == part_sz){
        if(job<8){
            int l = 0, r = 0, i;
            for(i=0;i<job;i++){l+=part_sz[i];}
            r = l + part_sz[i];
            // sem_wait(&arr_lock);
            // cout<<l<<' '<<r<<"\n";
            // cout<<1;
            bubble_sort(arr, l, r);
            // sem_post(&arr_lock);
        }
        else{
            int l = 0, i;
            for(i=0;i<merge_part[job].first;i++){l += part_sz[i];}
            int r = l;
            for(;i<=merge_part[job].second;i++){r += part_sz[i];}
            int m = l + (r-l)/2;
            // sem_wait(&arr_lock);
            // cout<<l<<' '<<m<<' '<<r<<"\n";
            // cout<<2;
            merge(arr, l, r);
            // sem_post(&arr_lock);
        }

        if(job<14){
            sem_wait(&q_lock);
            // sem_wait(&fns_lock);
            finish[job] = true;
            // cout<<job;
            if(job&1 && finish[job] && finish[job-1]){
                // cout<<5;
                job_q.push(next_job[job]);
            }
            else if(!(job&1) && finish[job] && finish[job+1]){
                // cout<<6;
                job_q.push(next_job[job]);
            }
            // sem_post(&fns_lock);
            sem_post(&q_lock);
        }
        // else{
        //     cout<<"uh: "<<job<<endl;
        // }
    }
    return NULL;
}


int main(){
    // read input
    fstream file;
    file.open("input.txt", ifstream::in);
    file>>arr_sz;

    vector<int> v(arr_sz);
    for(int i=0;i<arr_sz;i++){file>>v[i];}
    file.close();

    // init
    int all = arr_sz/8, more = arr_sz%8;
    part_sz = vector<int>(8);
    for(int i=0;i<8;i++){
        part_sz[i] = all;
        if(more){
            part_sz[i]++;
            more--;
        }
    }

    // 8 sort job + 7 merge job
    next_job = vector<int>(15, 0);
    merge_part = vector<pair<int, int>>(15, {0, 0});
    int tmp = 8;
    for(int i=0;i<14;i+=2){
        next_job[i] = tmp;
        next_job[i+1] = tmp;
        tmp++;
    }
    merge_part[8] = {0, 1};
    merge_part[9] = {2, 3};
    merge_part[10] = {4, 5};
    merge_part[11] = {6, 7};
    merge_part[12] = {0, 3};
    merge_part[13] = {4, 7};
    merge_part[14] = {0, 7};

    // sort with n threads
    for(int n_thread=1;n_thread<=8;n_thread++){
        // cout<<"there are "<<n_thread<<" thread\n";
        // init
        arr = v;
        finish = vector<int>(16, 0);

        // get started time
        struct timeval start, end;
        gettimeofday(&start, 0);

        // init
        pthread_t thread[n_thread];
        sem_init(&q_lock, 0, 1);
        // sem_init(&fns_event, 0, 0);

        // push sort job to job_q
        for(int i=0;i<8;i++){
            // int l = i*part_sz, r = (i+1)*part_sz;
            // if(i == 7){r = arr_sz;}
            // job_q.push({l, r});
            job_q.push(i);
        }

        // sort
        for(int i=0;i<n_thread;i++){
            pthread_create(&thread[i], NULL, worker, NULL);
        }

        for(int i=0;i<n_thread;i++){
            pthread_join(thread[i], NULL);
        }
        // sem_wait(&fns_event);

        // print time to terminal
        gettimeofday(&end, 0);
        int sec = end.tv_sec - start.tv_sec;
        int usec = end.tv_usec - start.tv_usec;
        cout <<"worker thread #"<<n_thread<<", elapsed "<< (sec*1000.0) + (usec/1000.0) << " ms"<<endl;

        // write sorted array to output_i.txt
        string file_name = "output_" + to_string(n_thread) + ".txt";
        file.open(file_name, ofstream::out);
        for(int a:arr){file<<a<<' ';}
        file<<endl;
        file.close();

        sem_destroy(&q_lock);

        // debug
        // for(auto a:finish){cout<<a<<' ';}
        // cout<<endl;
    }

    // debug
    // sort(v.begin(), v.end());
    // for(int a:v){cout<<a<<" ";}
    // cout<<endl;

    return 0;
}

/*
0 13
13 26
26 39
39 52
52 64
64 76
76 88
88 100
0 13 26
26 39 52
52 64 76
76 88 100
0 26 52
52 76 100
0 50 100
*/
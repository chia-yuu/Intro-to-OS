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
#include<list>
#include<string.h>
#include<vector>
#include<sys/time.h>
using namespace std;

#define MP_SIZE 131071
#define PAGE_SIZE 4096
#define hash_func(val) (val % MP_SIZE)

// ofstream outf("test.txt");
vector<bool> operation;
vector<int> pagenum;
int n_inst;

class LRU{
public:
    typedef struct Node{
        int page;
        int dirty;
        Node *pre, *next;
        Node(int page, int dirty, Node *pre, Node *next): page(page), dirty(dirty), pre(pre), next(next){}
    }Node;

    int n_frame;
    int n_page = 0;
    int hit_cnt = 0;
    int miss_cnt = 0;
    int write_cnt = 0;

    // float cnt = 0;
    // int mx = 0;

    Node *head, *tail;  // MRU, LRU
    list<Node*> mp[MP_SIZE];

    LRU(int frame): n_frame(frame){
        head = new Node(0, 0, NULL, NULL);
        tail = new Node(0, 0, head, NULL);
        head->next = tail;
    }

    void check_hit(bool op, int page){
        int idx = hash_func(page);
        // outf<<"page "<<page<<" = idx "<<idx<<endl;
        // mx = mx>mp[idx].size()? mx : mp[idx].size();
        for(auto &nd : mp[idx]){
            if(nd->page == page){
                hit_cnt++;
                if(op) nd->dirty = 1;

                // update list
                nd->pre->next = nd->next;
                nd->next->pre = nd->pre;

                // add to head
                nd->pre = head;
                nd->next = head->next;
                head->next->pre = nd;
                head->next = nd;
                return;
            }
        }
        // miss
        add_miss(op, page);
    }

    void add_miss(bool op, int page){
        // outf<<", miss";
        miss_cnt++;

        // replace
        if(n_page == n_frame){
            // outf<<", replace";
            // update list
            Node *remove_nd = tail->pre;
            tail->pre = remove_nd->pre;
            remove_nd->pre->next = tail;

            write_cnt += remove_nd->dirty;

            // remove from mp
            int remove_idx = hash_func(remove_nd->page);
            // if(mp[remove_idx].back()->page == remove_nd->page){
            //     outf<<"1";
            // }
            // else{outf<<"0";}
            // for(auto it = mp[remove_idx].rbegin(); it != mp[remove_idx].rend(); it++){
            //     Node *list_nd = *it;
            //     if(list_nd->page == remove_nd->page){
            //         mp[remove_idx].erase(next(it).base());
            //         break;
            //     }
            // }
            mp[remove_idx].remove(remove_nd);
            delete remove_nd;
            // outf<<"page";
        }
        else{
            n_page++;
        }
        
        // add miss node to list and mp
        Node *nd = new Node(page, op, head, head->next);
        head->next->pre = nd;
        head->next = nd;
        int idx = hash_func(page);
        mp[idx].push_back(nd);
    }

    void simulate(){
        // ifstream f;
        // string op, offset;
        // f.open(file);
        // if(!f.is_open()){
        //     cout<<"cannot open the file\n";
        // }
        // FILE* f = fopen(file, "r");
        // if(!f){
        //     printf("cannot open the file\n");
        //     return;
        // }

        for(int i=0;i<n_inst;i++){
            check_hit(operation[i], pagenum[i]);
        }

        // char line[50];
        // // while(f>>op>>offset){
        // while(fgets(line, sizeof(line), f)){
        //     // long long tmp = stoll(offset, 0, 16);
        //     // int page = tmp / PAGE_SIZE;
        //     // int is_write = (op=="w");
        //     // check_hit(is_write, page);
        //     char op;
        //     long long offset;
        //     sscanf(line, "%c %llx", &op, &offset);
        //     int page = offset / PAGE_SIZE;
        //     check_hit(op, page);
        // }
        // f.close();
        // fclose(f);
        printf("%d\t%d\t%d\t\t%.10f\t\t%d\n", n_frame, hit_cnt, miss_cnt, double(miss_cnt) / double(hit_cnt+miss_cnt), write_cnt);
        // outf<<"frame: "<<n_frame<<", "<<cnt / miss_cnt<<endl;
        // outf<<mx<<endl;
    }
};

// LRU correct
/*
class CFLRU{
public:
    typedef struct Node{
        int page;
        int dirty;
        Node *pre, *next;
        Node(int page, int dirty, Node *pre, Node *next): page(page), dirty(dirty), pre(pre), next(next){}
    }Node;

    int n_frame;
    int n_page1 = 0;    // page num in mp1
    int n_page2 = 0;    // page num in mp2
    int hit_cnt = 0;
    int miss_cnt = 0;
    int write_cnt = 0;
    int cf_region_sz;
    int wk_region_sz;

    int tot = 0;

    Node *head1, *tail1;  // working region MRU, LRU
    Node *head2, *tail2;  // clean first region MRU, LRU
    Node *head3, *tail3;  // clean in CF region head, tail
    // mp1: working region, mp2: clean first region, mp3: clean node in clean first region
    list<Node*> mp1[MP_SIZE], mp2[MP_SIZE], mp3;

    CFLRU(int frame): n_frame(frame){
        cf_region_sz = frame / 4;
        wk_region_sz = frame - cf_region_sz;
        
        head1 = new Node(0, 0, NULL, NULL);
        tail1 = new Node(0, 0, head1, NULL);
        head1->next = tail1;

        head2 = new Node(0, 0, NULL, NULL);
        tail2 = new Node(0, 0, head2, NULL);
        head2->next = tail2;
    }

    void check_hit(char op, int page){
        int idx = hash_func(page);
        // hit in mp1, move to mp1 head
        for(auto &nd : mp1[idx]){
            if(nd->page == page){
                hit_cnt++;
                if(op == 'W') nd->dirty = 1;

                // update list
                nd->pre->next = nd->next;
                nd->next->pre = nd->pre;

                nd->pre = head1;
                nd->next = head1->next;
                head1->next->pre = nd;
                head1->next = nd;
                return;
            }
        }
        // hit in mp2, nd to mp1 head, mp1 tail to mp2 head
        for(auto &nd : mp2[idx]){
            if(nd->page == page){
                hit_cnt++;
                if(op == 'W') nd->dirty = 1;

                // nd to mp1 head
                nd->pre->next = nd->next;
                nd->next->pre = nd->pre;

                nd->pre = head1;
                nd->next = head1->next;
                head1->next->pre = nd;
                head1->next = nd;

                mp2[idx].remove(nd);
                mp1[idx].push_back(nd);

                // mp1 tail to mp2 head
                Node *move_nd = tail1->pre;
                tail1->pre = move_nd->pre;
                move_nd->pre->next = tail1;

                move_nd->pre = head2;
                move_nd->next = head2->next;
                head2->next->pre = move_nd;
                head2->next = move_nd;

                int move_idx = hash_func(move_nd->page);
                mp1[move_idx].remove(move_nd);
                mp2[move_idx].push_back(move_nd);
                return;
            }
        }
        // doesn't hit in mp1 or mp2 -> miss
        add_miss(op, page);
    }

    void add_miss(char op, int page){
        miss_cnt++;
        // mp1 full, move mp1 tail to mp2 head
        if(n_page1 >= wk_region_sz){
            // remove from mp1 tail
            Node *move_nd = tail1->pre;
            tail1->pre = move_nd->pre;
            move_nd->pre->next = tail1;

            // add to mp2 head
            move_nd->pre = head2;
            move_nd->next = head2->next;
            head2->next->pre = move_nd;
            head2->next = move_nd;

            int move_idx = hash_func(move_nd->page);
            mp1[move_idx].remove(move_nd);
            mp2[move_idx].push_back(move_nd);

            n_page1--, n_page2++;
        }
        // mp2 full, remove mp2 tail (remove mp2 clean)
        if(n_page2 > cf_region_sz){
            Node *remove_nd = tail2->pre;
            tail2->pre = remove_nd->pre;
            remove_nd->pre->next = tail2;

            write_cnt += remove_nd->dirty;

            int remove_idx = hash_func(remove_nd->page);
            mp2[remove_idx].remove(remove_nd);
            n_page2--;
            delete remove_nd;
        }
        // add miss node to mp1 head
        Node *nd = new Node(page, (op == 'W'), head1, head1->next);
        head1->next->pre = nd;
        head1->next = nd;
        int idx = hash_func(page);
        mp1[idx].push_back(nd);
        n_page1++;
    }

    void simulate(char *file){
        FILE* f = fopen(file, "r");
        if(!f){
            printf("cannot open the file\n");
        }

        char line[50];
        while(fgets(line, sizeof(line), f)){
            char op;
            long long offset;
            sscanf(line, "%c %llx", &op, &offset);
            int page = offset / PAGE_SIZE;
            int is_write = (op=='W');
            check_hit(op, page);
        }
        fclose(f);
    }

    void output(){
        printf("%d\t%d\t%d\t\t%.10f\t\t%d\n", n_frame, hit_cnt, miss_cnt, double(miss_cnt) / double(hit_cnt+miss_cnt), write_cnt);
    }
};
*/
class CFLRU{
public:
    typedef struct Node{
        int page;
        int dirty;
        Node *pre, *next;
        Node(int page, int dirty, Node *pre, Node *next): page(page), dirty(dirty), pre(pre), next(next){}
    }Node;

    int n_frame;
    int n_page1 = 0;    // page num in mp1
    int n_page2 = 0;    // page num in mp2
    int hit_cnt = 0;
    int miss_cnt = 0;
    int write_cnt = 0;
    int cf_region_sz;
    int wk_region_sz;

    // int tot = 0;
    // int mx = 0;

    Node *head1, *tail1;  // working region MRU, LRU
    Node *head2, *tail2;  // clean first region MRU, LRU
    Node *head3, *tail3;  // clean in CF region, LRU in head, MRU tail
    // mp1: working region, mp2: clean first region, mp3: clean node in clean first region
    list<Node*> mp1[MP_SIZE], mp2[MP_SIZE], mp3;

    int cnt = 0;

    CFLRU(int frame): n_frame(frame){
        cf_region_sz = frame / 4;
        wk_region_sz = frame - cf_region_sz;
        
        head1 = new Node(0, 0, NULL, NULL);
        tail1 = new Node(0, 0, head1, NULL);
        head1->next = tail1;

        head2 = new Node(0, 0, NULL, NULL);
        tail2 = new Node(0, 0, head2, NULL);
        head2->next = tail2;
    }

    void check_hit(bool op, int page){
        int idx = hash_func(page);
        // hit in mp1, move to mp1 head
        // mx = mx>mp1[idx].size()? mx : mp1[idx].size();
        for(auto &nd : mp1[idx]){
            if(nd->page == page){
                hit_cnt++;
                if(op) nd->dirty = 1;

                // update list
                nd->pre->next = nd->next;
                nd->next->pre = nd->pre;

                nd->pre = head1;
                nd->next = head1->next;
                head1->next->pre = nd;
                head1->next = nd;

                // outf<<", hit mp1";
                return;
            }
        }
        // hit in mp2, nd to mp1 head, mp1 tail to mp2 head
        // mp3 remove nd, add mp1 tail
        // mx = mx>mp2[idx].size()? mx : mp2[idx].size();
        for(auto &nd : mp2[idx]){
            if(nd->page == page){
                hit_cnt++;
                // maybe originally clean, so in mp3, but now dirty
                // if mp3.remove after nd->dirty = 1, originally clean node can't be removed from mp3
                if(!(nd->dirty)) mp3.remove(nd);
                if(op) nd->dirty = 1;

                // nd to mp1 head
                nd->pre->next = nd->next;
                nd->next->pre = nd->pre;

                nd->pre = head1;
                nd->next = head1->next;
                head1->next->pre = nd;
                head1->next = nd;

                mp1[idx].push_back(nd);
                mp2[idx].remove(nd);

                // mp1 tail to mp2 head
                Node *move_nd = tail1->pre;
                tail1->pre = move_nd->pre;
                move_nd->pre->next = tail1;

                move_nd->pre = head2;
                move_nd->next = head2->next;
                head2->next->pre = move_nd;
                head2->next = move_nd;

                int move_idx = hash_func(move_nd->page);
                // int i = mp1[move_idx].size()-1;
                // for(auto it = mp1[move_idx].rbegin(); it != mp1[move_idx].rend(); it++){
                //     // i--;
                //     Node *list_nd = *it;
                //     if(list_nd->page == move_nd->page){
                //         // cnt += (i / mp1[move_idx].size()) >= 3/4;
                //         mp1[move_idx].erase(next(it).base());
                //         break;
                //     }
                // }
                mp1[move_idx].remove(move_nd);
                mp2[move_idx].push_back(move_nd);
                if(!(move_nd->dirty)) mp3.push_back(move_nd);

                // outf<<", hit mp2";
                return;
            }
        }
        // doesn't hit in mp1 or mp2 -> miss
        add_miss(op, page);
    }

    void add_miss(bool op, int page){
        // outf<<", miss";
        miss_cnt++;
        // mp2 full, remove one node from mp2 (clean first)
        if(n_page2 >= cf_region_sz){
            // outf<<", mp2 full";
            // has clean in mp2, clean first
            if(!mp3.empty()){
                // tot++;
                // outf<<"(cf)";
                Node *remove_nd = mp3.front();
                remove_nd->next->pre = remove_nd->pre;
                remove_nd->pre->next = remove_nd->next;

                // outf<<"remove from mp3, node dirty = "<<remove_nd->dirty<<endl;

                int remove_idx = hash_func(remove_nd->page);
                mp2[remove_idx].remove(remove_nd);
                // int i = mp2[remove_idx].size()-1;
                // for(auto it = mp2[remove_idx].rbegin(); it != mp2[remove_idx].rend(); it++){
                //     Node *list_nd = *it;
                //     if(list_nd->page == remove_nd->page){
                //         // cnt += (i/mp2[remove_idx].size()) >= 3/4;
                //         mp2[remove_idx].erase(next(it).base());
                //         break;
                //     }
                // }
                mp3.remove(remove_nd);
                n_page2--;
                delete remove_nd;
            }
            else{
                // outf<<"(no clean)";
                Node *remove_nd = tail2->pre;
                tail2->pre = remove_nd->pre;
                remove_nd->pre->next = tail2;

                write_cnt += remove_nd->dirty;

                int remove_idx = hash_func(remove_nd->page);
                mp2[remove_idx].remove(remove_nd);
                n_page2--;
                delete remove_nd;
            }
        }
        // mp1 full, move mp1 tail to mp2 head, add to mp3
        if(n_page1 >= wk_region_sz){
            // outf<<", mp1 full";
            // remove from mp1 tail
            Node *move_nd = tail1->pre;
            tail1->pre = move_nd->pre;
            move_nd->pre->next = tail1;

            // add to mp2 head
            move_nd->pre = head2;
            move_nd->next = head2->next;
            head2->next->pre = move_nd;
            head2->next = move_nd;

            int move_idx = hash_func(move_nd->page);
            mp1[move_idx].remove(move_nd);
            // for(auto it = mp1[move_idx].rbegin(); it != mp1[move_idx].rend(); it++){
            //     Node *list_nd = *it;
            //     if(list_nd->page == move_nd->page){
            //         mp1[move_idx].erase(next(it).base());
            //         break;
            //     }
            // }
            mp2[move_idx].push_back(move_nd);
            if(!(move_nd->dirty)) mp3.push_back(move_nd);

            n_page1--, n_page2++;
        }
        // add miss node to mp1 head
        Node *nd = new Node(page, op, head1, head1->next);
        head1->next->pre = nd;
        head1->next = nd;
        int idx = hash_func(page);
        mp1[idx].push_back(nd);
        n_page1++;
    }

    void simulate(){
        // FILE* f = fopen(file, "r");
        // if(!f){
        //     printf("cannot open the file\n");
        //     return;
        // }

        for(int i=0;i<n_inst;i++){
            check_hit(operation[i], pagenum[i]);
        }
        // char line[50];
        // int idx = 0;
        // while(fgets(line, sizeof(line), f)){
        //     // if(idx % 100000 == 0){cout<<'.';}
        //     // idx++;
        //     char op;
        //     long long offset;
        //     sscanf(line, "%c %llx", &op, &offset);
        //     int page = offset / PAGE_SIZE;
        //     // outf<<"\npage = "<<page<<", pg1 = "<<n_page1<<", pg2 = "<<n_page2;
        //     check_hit(op, page);
        //     // outf<<endl;
        //     // for(Node *nd : mp3){
        //     //     outf<<nd->page<<", ";
        //     // }
        // }
        // fclose(f);
        printf("%d\t%d\t%d\t\t%.10f\t\t%d\n", n_frame, hit_cnt, miss_cnt, double(miss_cnt) / double(hit_cnt+miss_cnt), write_cnt);
        // outf<<"## "<<n_frame<<": "<< cnt / tot<<endl;
        // outf<<mx<<endl;
    }
};

int main(int argc, char* argv[]){
    clock_t start, end;

    // outf<<"hiiii";
    ifstream f;
    f.open(argv[1]);
    char op;
    string offset;
    while(f>>op>>offset){
        operation.push_back(op=='W');
        int page = stoll(offset, 0, 16) / PAGE_SIZE;
        pagenum.push_back(page);
    }
    n_inst = pagenum.size();
    f.close();

    // LRU
    printf("LRU policy:\n");
    printf("Frame\tHit\t\tMiss\t\tPage fault ratio\tWrite back count\n");
    start = clock();
    for(int frame = 4096;frame <= 65536; frame *= 2){
        // output_file<<"\n### "<<frame<<" ###"<<endl;
        LRU lru(frame);
        // lru.simulate(argv[1]);
        lru.simulate();
    }
    end = clock();
    double t = (double)(end - start) / CLOCKS_PER_SEC;
	printf("Elapsed time: %.6f sec\n\n", t);

    // CFLRU
    // output_file.open("tmp.txt");
    printf("CFLRU policy:\n");
    printf("Frame\tHit\t\tMiss\t\tPage fault ratio\tWrite back count\n");
    start = clock();
    for(int frame = 4096;frame <= 65536; frame *= 2){
    // for(int frame = 12;frame <= 12; frame *= 2){
        // output_file<<"\n### "<<frame<<" ###"<<endl;
        CFLRU cflru(frame);
        // cflru.simulate(argv[1]);
        cflru.simulate();
    }
    end = clock();
    t = (double)(end - start) / CLOCKS_PER_SEC;
	printf("Elapsed time: %.6f sec\n\n", t);
}

/*
./main inceptionV3_tf_oshw5.txt

s max: 65535, s min: 264
l max: 33550336, l min: 65536
line num = 49228943
           7086122

2:06

list remove = O(n)
*/